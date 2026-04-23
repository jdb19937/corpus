/*
 * descensor.c — Descensus Gradientis Stochasticus et Angulus Rotundationis
 *
 * SGD classicus [Robbins-Monro 1951] cum momento Polyak (1964) et
 * Nesterov (1983), exempli gratia applicatus ad regressionem
 * linearem in R^d. Programma concentrat in angulis fluitantis
 * binary32 qui in doctrina neurali saepe subveniunt:
 *
 *   I.   Diagnostica NaN/Inf gradientum (isfinite, isnan).
 *   II.  Divergentia cum rata discendi nimis magna: explosio
 *        ponderum, underflow, overflow.
 *   III. Modi rotundationis IEEE 754 via <fenv.h>:
 *          FE_TONEAREST (defalta, ties-to-even)
 *          FE_TOWARDZERO
 *          FE_UPWARD
 *          FE_DOWNWARD
 *        Effectus in accumulatione magnarum summarum.
 *   IV.  Exceptiones fluentes (feraiseexcept, fetestexcept):
 *          FE_OVERFLOW, FE_UNDERFLOW, FE_INEXACT, FE_DIVBYZERO.
 *   V.   Adaptive rate: AdaGrad [Duchi 2011], Adam [Kingma 2014]
 *        — momentum plus variantia gradientis.
 *
 * ═══════════════════════════════════════════════════════════════════
 * REFERENTIAE:
 *   [RM51]   Robbins H., Monro S. "A stochastic approximation method."
 *            Annals of Math. Stat. 22(3): 400-407 (1951).
 *   [Pol64]  Polyak B. T. "Some methods of speeding up the convergence
 *            of iteration methods." USSR Comp. Math. 4(5): 1-17 (1964).
 *   [Nes83]  Nesterov Y. "A method for unconstrained convex minimization
 *            with the rate of convergence O(1/k^2)." Dokl. Akad. Nauk
 *            SSSR 269: 543-547 (1983).
 *   [SMDH13] Sutskever I., Martens J., Dahl G., Hinton G. "On the
 *            importance of initialization and momentum in deep
 *            learning." ICML (2013).
 *   [DHS11]  Duchi J., Hazan E., Singer Y. "Adaptive subgradient
 *            methods for online learning..." JMLR 12: 2121-2159 (2011).
 *   [KB14]   Kingma D. P., Ba J. "Adam: A Method for Stochastic
 *            Optimization." arXiv:1412.6980 (2014).
 *   [C99 §7.6] ISO/IEC 9899:1999, Floating-point environment.
 *   [IEEE754] IEEE Std 754-2019 §4, rounding-direction attributes.
 *
 * Nullae optiones CLI, nullum stdin. Egressus deterministicus.
 * ═══════════════════════════════════════════════════════════════════
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <float.h>
#include <math.h>
#include <fenv.h>

/* Pragma STDC FENV_ACCESS requirit strictam semanticam exceptionum;
 * multi compilatores hanc ignorant (clang/GCC emitunt monitionem
 * "unknown pragma"), sed additio tutum est. */
#pragma STDC FENV_ACCESS ON

#define DICERE(...)    fprintf(stdout, __VA_ARGS__)
#define SUSSURRO(...)  fprintf(stderr, __VA_ARGS__)

#define D_PARAM    4     /* dimensio parametri */
#define N_DATA    32     /* numerus exemplorum */
#define MAX_ITER 500     /* iterationes maxima */

/* ========================================================================
 * I. GENERATOR DETERMINISTICUS
 * ==================================================================== */

typedef struct { uint32_t status; } Fors;

static inline uint32_t
xorshift32(Fors *f)
{
    uint32_t x = f->status;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    f->status = x ? x : 0xC001D00Du;
    return f->status;
}
static float forte_unus(Fors *f)   { return (float)(xorshift32(f) >> 8) * ldexpf(1.0f, -24); }
static float forte_sign(Fors *f)   { return 2.0f * forte_unus(f) - 1.0f; }
static float forte_norm(Fors *f)
{
    float s = 0.0f;
    for (int i = 0; i < 12; i++) s += forte_unus(f);
    return s - 6.0f;
}

/* ========================================================================
 * II. DATA DOCENDI — REGRESSIO LINEARIS
 * ====================================================================
 * Modellum verum: y = w*.x + b* + epsilon_i, cum
 *   w* = (0.5, -1.25, 2.0, -0.75), b* = 0.1
 * Noise: eps ~ N(0, 0.1). */

static float DATA_X[N_DATA][D_PARAM];
static float DATA_Y[N_DATA];

static const float W_VERUS[D_PARAM] = { 0.5f, -1.25f, 2.0f, -0.75f };
static const float B_VERUS         = 0.1f;

static void
genera_data(void)
{
    Fors f = { .status = 0x1337BEEFu };
    for (int i = 0; i < N_DATA; i++) {
        float z = B_VERUS;
        for (int j = 0; j < D_PARAM; j++) {
            DATA_X[i][j] = forte_sign(&f) * 2.0f;
            z += W_VERUS[j] * DATA_X[i][j];
        }
        DATA_Y[i] = z + 0.1f * forte_norm(&f);
    }
}

/* ========================================================================
 * III. MODELLUM: y_pred = w . x + b
 * ==================================================================== */

static float
predice(const float *w, float b, const float x[D_PARAM])
{
    float z = b;
    for (int j = 0; j < D_PARAM; j++) z = fmaf(w[j], x[j], z);
    return z;
}

/* Loss = 1/(2n) * sum (y_i - y_pred_i)^2 */
static float
loss_MSE(const float *w, float b)
{
    float s = 0.0f;
    for (int i = 0; i < N_DATA; i++) {
        float d = predice(w, b, DATA_X[i]) - DATA_Y[i];
        s = fmaf(d, d, s);
    }
    return 0.5f * s / (float)N_DATA;
}

/* Gradiens MSE: dL/dw_j = (1/n) sum (y_pred_i - y_i) * x_ij
 *                dL/db   = (1/n) sum (y_pred_i - y_i) */
static void
gradiens_MSE(const float *w, float b, float *gw, float *gb)
{
    for (int j = 0; j < D_PARAM; j++) gw[j] = 0.0f;
    *gb = 0.0f;
    for (int i = 0; i < N_DATA; i++) {
        float d = predice(w, b, DATA_X[i]) - DATA_Y[i];
        for (int j = 0; j < D_PARAM; j++)
            gw[j] = fmaf(d, DATA_X[i][j], gw[j]);
        *gb += d;
    }
    float inv_n = 1.0f / (float)N_DATA;
    for (int j = 0; j < D_PARAM; j++) gw[j] *= inv_n;
    *gb *= inv_n;
}

/* ========================================================================
 * IV. SGD STANDARDIS
 * ==================================================================== */

static int
sgd_vanilla(float *w, float *b, float eta, int max_iter,
            float *loss_final)
{
    float gw[D_PARAM], gb;
    for (int it = 0; it < max_iter; it++) {
        gradiens_MSE(w, *b, gw, &gb);
        /* Probatio ne gradiens divergeat */
        int finitus = isfinite(gb);
        for (int j = 0; j < D_PARAM; j++) finitus = finitus && isfinite(gw[j]);
        if (!finitus) {
            *loss_final = INFINITY;
            return -it;   /* negative iter index -> divergentia */
        }
        for (int j = 0; j < D_PARAM; j++) w[j] -= eta * gw[j];
        *b -= eta * gb;
    }
    *loss_final = loss_MSE(w, *b);
    return max_iter;
}

/* SGD + MOMENTUM POLYAK [Pol64]:
 *   v_{t+1} = beta * v_t + grad
 *   w_{t+1} = w_t - eta * v_{t+1} */
static int
sgd_momentum(float *w, float *b, float eta, float beta, int max_iter,
             float *loss_final)
{
    float vw[D_PARAM] = { 0.0f }, vb = 0.0f;
    float gw[D_PARAM], gb;
    for (int it = 0; it < max_iter; it++) {
        gradiens_MSE(w, *b, gw, &gb);
        int finitus = isfinite(gb);
        for (int j = 0; j < D_PARAM; j++) finitus = finitus && isfinite(gw[j]);
        if (!finitus) { *loss_final = INFINITY; return -it; }
        for (int j = 0; j < D_PARAM; j++) {
            vw[j] = beta * vw[j] + gw[j];
            w[j] -= eta * vw[j];
        }
        vb = beta * vb + gb;
        *b -= eta * vb;
    }
    *loss_final = loss_MSE(w, *b);
    return max_iter;
}

/* ADAM [KB14]:
 *   m_t = beta1 * m_{t-1} + (1-beta1) * g
 *   v_t = beta2 * v_{t-1} + (1-beta2) * g^2
 *   m_hat = m_t / (1 - beta1^t)
 *   v_hat = v_t / (1 - beta2^t)
 *   w -= eta * m_hat / (sqrt(v_hat) + eps) */
static int
adam(float *w, float *b, float eta, int max_iter, float *loss_final)
{
    const float beta1 = 0.9f, beta2 = 0.999f, eps = 1.0e-8f;
    float m_w[D_PARAM] = { 0 }, v_w[D_PARAM] = { 0 };
    float m_b = 0.0f, v_b = 0.0f;
    float gw[D_PARAM], gb;
    for (int it = 1; it <= max_iter; it++) {
        gradiens_MSE(w, *b, gw, &gb);
        int finitus = isfinite(gb);
        for (int j = 0; j < D_PARAM; j++) finitus = finitus && isfinite(gw[j]);
        if (!finitus) { *loss_final = INFINITY; return -it; }
        float corr1 = 1.0f - powf(beta1, (float)it);
        float corr2 = 1.0f - powf(beta2, (float)it);
        for (int j = 0; j < D_PARAM; j++) {
            m_w[j] = beta1 * m_w[j] + (1.0f - beta1) * gw[j];
            v_w[j] = beta2 * v_w[j] + (1.0f - beta2) * gw[j] * gw[j];
            float m_hat = m_w[j] / corr1;
            float v_hat = v_w[j] / corr2;
            w[j] -= eta * m_hat / (sqrtf(v_hat) + eps);
        }
        m_b = beta1 * m_b + (1.0f - beta1) * gb;
        v_b = beta2 * v_b + (1.0f - beta2) * gb * gb;
        *b -= eta * (m_b / corr1) / (sqrtf(v_b / corr2) + eps);
    }
    *loss_final = loss_MSE(w, *b);
    return max_iter;
}

/* ========================================================================
 * V. CAPUT ET INIT
 * ==================================================================== */

static void
caput_imprime(void)
{
    DICERE("# ═══════════════════════════════════════════════════════════\n");
    DICERE("# DESCENSOR v1.0.0 — SGD/Adam cum rotundatione IEEE 754\n");
    DICERE("# ═══════════════════════════════════════════════════════════\n");
    DICERE("# Modellum verum: y = w* . x + b* + N(0, 0.1)\n");
    DICERE("#   w* = (0.5, -1.25, 2.0, -0.75)\n");
    DICERE("#   b* = 0.1\n");
    DICERE("# n = %d data, d = %d parametri.\n", N_DATA, D_PARAM);
    DICERE("# FLT_EPSILON = %.4e, FLT_MIN = %.4e, FLT_MAX = %.4e\n",
           (double)FLT_EPSILON, (double)FLT_MIN, (double)FLT_MAX);
    DICERE("#\n");
}

/* ========================================================================
 * VI. SECTIO I — CONVERGENTIA SGD STANDARDIS
 * ==================================================================== */

static void
sectio_convergentia(void)
{
    DICERE("# ─── I. CONVERGENTIA METHODORUM [RM51, Pol64, KB14] ────────\n");
    static const struct {
        const char *nomen;
        float eta;
        int   max_iter;
        int   (*solutio)(float*, float*, float, int, float*);
        float beta;
        int   sumitBeta;
    } METHODI[] = {
        { "SGD vanilla", 0.05f,  300, NULL, 0.0f, 0 },
        { "SGD vanilla", 0.01f,  500, NULL, 0.0f, 0 },
        { "SGD+mom 0.9", 0.05f,  300, NULL, 0.9f, 1 },
        { "Adam",         0.1f,  300, NULL, 0.0f, 0 }
    };
    const int N = (int)(sizeof METHODI / sizeof METHODI[0]);

    DICERE("#  methodus        eta      iter    loss_finalis   w_err   b_err\n");
    for (int k = 0; k < N; k++) {
        float w[D_PARAM] = { 0 };
        float b = 0.0f;
        float L = 0.0f;
        int result;
        if (k == 0 || k == 1)
            result = sgd_vanilla(w, &b, METHODI[k].eta, METHODI[k].max_iter, &L);
        else if (k == 2)
            result = sgd_momentum(w, &b, METHODI[k].eta, METHODI[k].beta,
                                  METHODI[k].max_iter, &L);
        else
            result = adam(w, &b, METHODI[k].eta, METHODI[k].max_iter, &L);

        /* w_err = ||w - w*||_2, b_err = |b - b*| */
        float w_err = 0.0f;
        for (int j = 0; j < D_PARAM; j++) {
            float d = w[j] - W_VERUS[j];
            w_err += d * d;
        }
        w_err = sqrtf(w_err);
        float b_err = fabsf(b - B_VERUS);

        DICERE("#  %-13s   %.3f    %4d    %.4e   %.4f  %.4f\n",
               METHODI[k].nomen, (double)METHODI[k].eta, result,
               (double)L, (double)w_err, (double)b_err);
        SUSSURRO("# [I] %s w=[%.4e,%.4e,%.4e,%.4e] b=%.4e\n",
                 METHODI[k].nomen,
                 (double)w[0], (double)w[1], (double)w[2], (double)w[3],
                 (double)b);
    }
    DICERE("#\n");
}

/* ========================================================================
 * VII. SECTIO II — DIVERGENTIA ET NaN/Inf
 * ==================================================================== */

static void
sectio_divergentia(void)
{
    DICERE("# ─── II. DIVERGENTIA: NaN/Inf DETECTIO ─────────────────────\n");
    static const float ETAS[] = {
        0.01f, 0.1f, 0.5f, 1.0f, 2.0f, 5.0f, 10.0f
    };
    const int N = (int)(sizeof ETAS / sizeof ETAS[0]);
    DICERE("#   eta     iter    status             loss_fin    ||w||\n");
    for (int k = 0; k < N; k++) {
        float w[D_PARAM] = { 0 };
        float b = 0.0f;
        float L;
        int result = sgd_vanilla(w, &b, ETAS[k], 200, &L);
        float norma = 0.0f;
        for (int j = 0; j < D_PARAM; j++) norma = fmaf(w[j], w[j], norma);
        norma = sqrtf(norma);
        const char *status;
        if (result > 0 && isfinite(L)) status = "convergit        ";
        else if (result > 0)            status = "stagnat          ";
        else                             status = "divergit (NaN/Inf)";
        DICERE("#   %5.2f   %4d    %s  ",
               (double)ETAS[k], result < 0 ? -result : result, status);
        if (isfinite(L)) DICERE("%.4e   %.4e\n",
                                 (double)L, (double)norma);
        else              DICERE("%-10s   %.4e\n",
                                  isinf(L) ? "INF" : "NaN",
                                  (double)norma);
    }
    DICERE("# Cum eta > ~0.1 in hoc exemplo, gradiens amplificat pondera\n");
    DICERE("# exponenter -> overflow -> NaN per (INF - INF) aut Inf*0.\n");
    DICERE("# Defensio: gradient clipping (non hic) aut eta conservativa.\n");
    DICERE("#\n");
}

/* ========================================================================
 * VIII. SECTIO III — MODI ROTUNDATIONIS [C99 §7.6, IEEE754 §4]
 * ====================================================================
 * Summa magna cum rotundatione variata demonstrat effectum modi.
 * Exspectatur valor valde similis pro FE_TONEAREST et aliae modi. */

static void
sectio_rotundatio(void)
{
    DICERE("# ─── III. MODI ROTUNDATIONIS ───────────────────────────────\n");
    /* Probatio: summa 1/i pro i = 1..10^6 sub quoque modo. */
    const int n = 1000000;
    static const struct { int mode; const char *nomen; } MODI[] = {
        { FE_TONEAREST,  "TONEAREST" },
        { FE_TOWARDZERO, "TOWARDZERO" },
        { FE_UPWARD,     "UPWARD" },
        { FE_DOWNWARD,   "DOWNWARD" }
    };
    const int NM = (int)(sizeof MODI / sizeof MODI[0]);

    int original = fegetround();
    DICERE("# Summa 1/i, i = 1..%d, per quoque modo rotundationis:\n", n);
    DICERE("#   modus         summa        FLT_ROUNDS\n");

    double reference = 0.0;
    for (int i = 1; i <= n; i++) reference += 1.0 / (double)i;

    for (int m = 0; m < NM; m++) {
        if (fesetround(MODI[m].mode) != 0) {
            DICERE("#   %-12s  [fallit]\n", MODI[m].nomen);
            continue;
        }
        float s = 0.0f;
        for (int i = 1; i <= n; i++) s += 1.0f / (float)i;
        DICERE("#   %-12s  %.6f       FLT_ROUNDS=%d\n",
               MODI[m].nomen, (double)s, FLT_ROUNDS);
        SUSSURRO("# [III] %s s=%.9e ref=%.9e\n",
                 MODI[m].nomen, (double)s, reference);
    }
    fesetround(original);
    DICERE("# Referens (duplex): %.6f\n", reference);
    DICERE("# Nota: FLT_ROUNDS macro dynamice reflet modum currentem.\n");
    DICERE("#\n");
}

/* ========================================================================
 * IX. SECTIO IV — EXCEPTIONES FLUENTES [C99 §7.6.2]
 * ==================================================================== */

static void
sectio_exceptiones(void)
{
    DICERE("# ─── IV. EXCEPTIONES FLUENTES ──────────────────────────────\n");
    DICERE("# Probatio operationum quae exceptiones eliciunt:\n");

    /* Operationes probatorae */
    static const struct {
        const char *descriptio;
        int         expectat;
        const char *flag_nomen;
    } TESTS[] = {
        { "FLT_MAX * 2.0",     FE_OVERFLOW,  "FE_OVERFLOW" },
        { "1.0 / FLT_MAX",     FE_INEXACT,   "FE_INEXACT (potest)" },
        { "sqrtf(-1.0)",       FE_INVALID,   "FE_INVALID" },
        { "1.0 / 0.0",         FE_DIVBYZERO, "FE_DIVBYZERO" },
        { "FLT_MIN * 0.5",     FE_UNDERFLOW, "FE_UNDERFLOW (potest)" },
        { "1.0 + 0.1 ulp",     FE_INEXACT,   "FE_INEXACT" }
    };
    const int NT = (int)(sizeof TESTS / sizeof TESTS[0]);
    DICERE("#  operatio             flag elicitum         valor\n");
    /* C99 §6.6: operandi volatilis operatio non constans est, ergo
     * compilator operationem non-foldet in compile-time, et vexilla
     * fenv runtime-emissa fiunt uniformia per omnes compilatores
     * (pragma FENV_ACCESS ON semper respectata). */
    volatile float v_flt_max = FLT_MAX;
    volatile float v_flt_min = FLT_MIN;
    volatile float v_one     = 1.0f;
    volatile float v_neg_one = -1.0f;
    volatile float v_eps     = FLT_EPSILON;
    volatile float v_zero    = 0.0f;
    for (int i = 0; i < NT; i++) {
        feclearexcept(FE_ALL_EXCEPT);
        volatile float r = 0.0f;
        switch (i) {
            case 0: r = v_flt_max * 2.0f; break;
            case 1: r = v_one / v_flt_max; break;
            case 2: r = sqrtf(v_neg_one); break;
            case 3: r = v_one / v_zero; break;
            case 4: r = v_flt_min * 0.5f; break;
            case 5: r = v_one + v_eps * 0.7f; break;
        }
        int flags = fetestexcept(FE_ALL_EXCEPT);
        char status[64] = "";
        if (flags & FE_OVERFLOW)  strcat(status, "OVER ");
        if (flags & FE_UNDERFLOW) strcat(status, "UNDER ");
        if (flags & FE_INEXACT)   strcat(status, "INEX ");
        if (flags & FE_INVALID)   strcat(status, "INVAL ");
        if (flags & FE_DIVBYZERO) strcat(status, "DIV0 ");
        if (status[0] == '\0')    strcpy(status, "(nullum)");
        DICERE("#  %-20s %-20s  ", TESTS[i].descriptio, status);
        if (isfinite(r)) DICERE("%.4e\n", (double)r);
        else if (isnan(r)) DICERE("NaN\n");
        else DICERE(r > 0 ? "+INF\n" : "-INF\n");
        SUSSURRO("# [IV] test=%d r=%.9e flags=0x%x\n",
                 i, (double)r, flags);
        (void)TESTS[i].expectat;
        (void)TESTS[i].flag_nomen;
    }
    feclearexcept(FE_ALL_EXCEPT);
    DICERE("#\n");
}

/* ========================================================================
 * X. SECTIO V — TRAIECTUS DESCENSUS
 * ====================================================================
 * Ostensio loss per iterationem — "learning curve". */

static void
sectio_traiectus(void)
{
    DICERE("# ─── V. TRAIECTUS DESCENSUS (SGD+mom vs Adam) ──────────────\n");
    float w_sgd[D_PARAM] = { 0 }, b_sgd = 0.0f;
    float w_adm[D_PARAM] = { 0 }, b_adm = 0.0f;
    float vw[D_PARAM] = { 0 }, vb = 0.0f;
    const float eta_sgd = 0.05f, beta_sgd = 0.9f;
    const float beta1 = 0.9f, beta2 = 0.999f, eps = 1.0e-8f, eta_a = 0.1f;
    float m_w[D_PARAM] = { 0 }, v_w[D_PARAM] = { 0 };
    float m_b = 0.0f, v_b = 0.0f;
    const int iter = 200;
    const int quisque = 20;

    DICERE("#  iter   L(SGD+mom)    L(Adam)     bar_SGD              bar_Adam\n");
    for (int it = 0; it <= iter; it++) {
        if (it % quisque == 0) {
            float ls = loss_MSE(w_sgd, b_sgd);
            float la = loss_MSE(w_adm, b_adm);
            int bs = (int)(fminf(40.0f, logf(ls + 1.0f) * 8.0f));
            int ba = (int)(fminf(40.0f, logf(la + 1.0f) * 8.0f));
            if (bs < 0) bs = 0;
            if (ba < 0) ba = 0;
            char bar_s[41] = {0}, bar_a[41] = {0};
            for (int k = 0; k < bs; k++) bar_s[k] = '=';
            for (int k = 0; k < ba; k++) bar_a[k] = '=';
            /* praecisione reducta ad stdout (.3e): accumulatio fluitantis
             * inter compilatores potest differe 1 ULP; plena praecisio
             * per SUSSURRO ad stderr. */
            DICERE("#  %4d   %.3e    %.3e   %-20s %-20s\n",
                   it, (double)ls, (double)la, bar_s, bar_a);
            SUSSURRO("# [V] iter=%d L_sgd=%.9e L_adam=%.9e\n",
                     it, (double)ls, (double)la);
        }
        if (it == iter) break;

        /* SGD step */
        float gw[D_PARAM], gb;
        gradiens_MSE(w_sgd, b_sgd, gw, &gb);
        for (int j = 0; j < D_PARAM; j++) {
            vw[j] = beta_sgd * vw[j] + gw[j];
            w_sgd[j] -= eta_sgd * vw[j];
        }
        vb = beta_sgd * vb + gb;
        b_sgd -= eta_sgd * vb;

        /* Adam step */
        gradiens_MSE(w_adm, b_adm, gw, &gb);
        int t = it + 1;
        float corr1 = 1.0f - powf(beta1, (float)t);
        float corr2 = 1.0f - powf(beta2, (float)t);
        for (int j = 0; j < D_PARAM; j++) {
            m_w[j] = beta1 * m_w[j] + (1.0f - beta1) * gw[j];
            v_w[j] = beta2 * v_w[j] + (1.0f - beta2) * gw[j] * gw[j];
            w_adm[j] -= eta_a * (m_w[j] / corr1) / (sqrtf(v_w[j] / corr2) + eps);
        }
        m_b = beta1 * m_b + (1.0f - beta1) * gb;
        v_b = beta2 * v_b + (1.0f - beta2) * gb * gb;
        b_adm -= eta_a * (m_b / corr1) / (sqrtf(v_b / corr2) + eps);
    }
    DICERE("#\n");
}

/* ========================================================================
 * XI. EPILOGUS
 * ==================================================================== */

static void
epilogus(void)
{
    DICERE("# ─── EPILOGUS ──────────────────────────────────────────────\n");
    DICERE("# Sex lectiones descensus gradientis binary32:\n");
    DICERE("#   1. SGD vanilla convergit linearii rate ~ (1 - eta L)^t.\n");
    DICERE("#   2. Momentum Polyak convergentiam accelerat; cost: 2x memoria.\n");
    DICERE("#   3. Adam adaptat rate per parametrum; robustior ad noise.\n");
    DICERE("#   4. NaN/Inf detectio per isfinite praevenit propagationem\n");
    DICERE("#      errorum in gradientes subsequentes.\n");
    DICERE("#   5. Modi rotundationis non-defalti errorem systematicum\n");
    DICERE("#      addunt summae magnae; FE_TONEAREST praeferendum.\n");
    DICERE("#   6. fenv.h exceptiones in hardware numerico diagnosticas\n");
    DICERE("#      facit — utile pro CI/pipe testing.\n");
    DICERE("# ═══════════════════════════════════════════════════════════\n");
}

/* ========================================================================
 * XII. FUNCTIO PRINCIPALIS
 * ==================================================================== */

int
main(void)
{
    caput_imprime();
    genera_data();
    sectio_convergentia();
    sectio_divergentia();
    sectio_rotundatio();
    sectio_exceptiones();
    sectio_traiectus();
    epilogus();
    return 0;
}

/* ========================================================================
 * APPENDIX A — CONVERGENTIA SGD
 * ====================================================================
 *
 * Pro loss quadraticus convexus L(w) = 1/2 (Aw - b)^T(Aw - b) / n,
 * gradiens G(w) = A^T(Aw - b) / n. SGD update:
 *   w_{t+1} = w_t - eta G(w_t)
 *           = (I - eta A^T A / n) w_t + eta A^T b / n
 *
 * Operator I - eta H (cum H = A^T A / n Hessianum) habet eigenvalues
 * 1 - eta lambda_i. Stabilitas requirit |1 - eta lambda_i| < 1
 * pro omnibus i, i.e.:
 *   0 < eta < 2 / lambda_max.
 *
 * Si eta > 2/lambda_max, pondera exponere crescunt -> overflow.
 * Praxis: schedule rate (cosine, warmup), gradient clipping.
 *
 * ─────────────────────────────────────────────────────────────────
 * APPENDIX B — ADAM INTERNUS
 * ─────────────────────────────────────────────────────────────────
 *
 * Adam [KB14] combinat momentum (primum ordinem) cum estimatione
 * variantiae (secundum ordinem):
 *
 *   m_t = beta1 * m_{t-1} + (1 - beta1) * g_t    (EMA gradiens)
 *   v_t = beta2 * v_{t-1} + (1 - beta2) * g_t^2  (EMA gradiens^2)
 *
 * Bias correction necessarius quia m_0 = v_0 = 0:
 *   m_hat = m_t / (1 - beta1^t)
 *   v_hat = v_t / (1 - beta2^t)
 *
 * Update:
 *   w_{t+1} = w_t - eta * m_hat / (sqrt(v_hat) + eps)
 *
 * Numeros epsilon 1e-8 defensio contra div/0 cum v_hat ~ 0.
 * In binary16 (fp16), eps = 1e-4 aut 1e-3 debet esse — sqrtf(v_hat)
 * subnormalis fieri potest pro v_hat < 2^-30.
 *
 * ─────────────────────────────────────────────────────────────────
 * APPENDIX C — fenv.h COMPATIBILITAS
 * ─────────────────────────────────────────────────────────────────
 *
 * C99 §7.6 definit environmentum fluentem. Requirit pragma:
 *   #pragma STDC FENV_ACCESS ON
 * pro strictam semanticam. Multi compilatores hanc pragma ignorant
 * (clang, GCC) et presumunt DEFAULT_OFF. Sine pragma, compilator
 * potest operationes re-ordinare in modo qui flags incorrectas
 * eliciat.
 *
 * Pro code portabili:
 *   - Voca feclearexcept(FE_ALL_EXCEPT) ante operationem interesse.
 *   - Usa volatile pro resultu ut compilator non-eliminat.
 *   - Voca fetestexcept IMMEDIATE post operationem.
 *
 * x86: usa MXCSR register (SSE) aut x87 status word (legacy).
 * ARM (aarch64): FPCR register, status bits similes.
 *
 * ═══════════════════════════════════════════════════════════════════ */
