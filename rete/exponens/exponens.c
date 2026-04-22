/*
 * exponens.c — Exponentiatio et Log-Sum-Exp Stabilis
 *
 * Tractatio numerica softmax et log-sum-exp in binary32, cum
 * focus in limitibus exponentiationis:
 *
 *   expf(x):
 *     - x >= 88.7228   -> supereffluxio ad +INFINITY
 *     - x <= -87.3365  -> subeffluxio ad 0 (ante subnormales)
 *     - x <= -103.972  -> subeffluxio ad 0 exacte (post subnormales)
 *     - x == -INF       -> 0 exacte (defined [C99 §7.12.6.1])
 *     - x == +INF       -> +INF
 *     - x == NaN        -> NaN
 *
 *   Softmax naivus: y_i = exp(x_i) / sum_j exp(x_j).
 *     Cum max(x) >= 89, omnes exp ruunt ad INF; NaN emerget per
 *     divisionem INF/INF. Solutio: translatio per maximum.
 *
 *   Log-sum-exp: logsumexp(x) = log(sum exp(x_i))
 *                             = max(x) + log(sum exp(x_i - max(x))).
 *     Identitas ista praecisionem conservat quia omnes exp post
 *     translationem in [exp(-K), 1] iacent.
 *
 *   Masca attentionis: nexus lost usantur -INFINITY ut
 *     exp(-INF) = 0, quod scores irrilevantes effective
 *     annihilet sine branching.
 *
 * ═══════════════════════════════════════════════════════════════════
 * SECTIONES:
 *   I.   Limites expf: mapping input -> fpclassify(expf(input)).
 *   II.  Softmax naivus vs stabilis cum logitis magnis.
 *   III. Log-sum-exp: identitas translativa demonstrata.
 *   IV.  Entropia crucialis stabilis: log(softmax) sine log(0).
 *   V.   Masca -INFINITY (attentio causalis).
 *   VI.  Gumbel-softmax: temperatura et quasi-argmax.
 *   VII. Distributio distributionis: entropia H(p) pro variis temp.
 *
 * REFERENTIAE:
 *   [Bri89]  Bridle J. S. "Probabilistic Interpretation of Feedforward
 *            Classification Network Outputs." in Soulie &
 *            Herault (eds.), NATO ASI Series (1989). [Softmax nomen]
 *   [GEC14]  Goodfellow I. J., Bengio Y., Courville A. "Deep Learning"
 *            Draft (2014), Cap. 4.1 (Overflow & Underflow).
 *   [JGP17]  Jang E., Gu S., Poole B. "Categorical Reparameterization
 *            with Gumbel-Softmax." ICLR (2017).
 *   [MB16]   Martins A. F. T., Astudillo R. F. "From Softmax to
 *            Sparsemax." ICML (2016).
 *   [Mul97]  Muller J.-M. "Elementary Functions: Algorithms and
 *            Implementation." Birkhauser (1997).
 *   [C99 §7.12.6] ISO/IEC 9899:1999, functiones expf/logf.
 *   [IEEE754] IEEE Std 754-2019.
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

#define DICERE(...)    fprintf(stdout, __VA_ARGS__)
#define SUSSURRO(...)  fprintf(stderr, __VA_ARGS__)

static uint32_t
bitti(float x)
{
    uint32_t u;
    memcpy(&u, &x, sizeof u);
    return u;
}

static const char *
classis_nomen(int c)
{
    switch (c) {
        case FP_NAN:       return "NAN";
        case FP_INFINITE:  return "INF";
        case FP_ZERO:      return "NUL";
        case FP_SUBNORMAL: return "SUB";
        case FP_NORMAL:    return "NOR";
        default:           return "???";
    }
}

/* ========================================================================
 * I. CONSTANTES DE LIMITIBUS expf binary32
 * ====================================================================
 * log(FLT_MAX)        =  88.7228317...   -> supreme input ut exp normalis
 * log(FLT_MIN)        = -87.3365479...   -> limes ad regionem subnormalem
 * log(FLT_TRUE_MIN)   = -103.9720770...  -> limes exactus ad zero
 *
 * Pro binary32 eps_ULP(1) = 2^-23, logarithmus bit additus = 2^-23,
 * quod significat input x cum eps_x ad exp committit errorem
 * relativum ~ |eps_x|. Haec stabilitas expf relativa est. */

#define EXP_LIMES_SUPER    88.7228317f    /* log(FLT_MAX) */
#define EXP_LIMES_SUB     -87.3365479f    /* log(FLT_MIN) */
#define EXP_LIMES_NUL    -103.9720770f    /* log(FLT_TRUE_MIN) */

/* ========================================================================
 * II. SECTIO I — LIMITES expf
 * ==================================================================== */

static void
sectio_limites(void)
{
    DICERE("# ─── I. LIMITES EXPF BINARY32 ──────────────────────────────\n");
    static const float INPUTS[] = {
        -200.0f, -110.0f, -103.97f, -100.0f,
         -89.0f,  -87.5f,  -87.3f,  -50.0f,
           0.0f,    1.0f,   10.0f,   80.0f,
          87.0f,   88.7f,   88.73f,  89.0f,
         100.0f,  200.0f,  INFINITY, -INFINITY
    };
    const int N = (int)(sizeof INPUTS / sizeof INPUTS[0]);
    DICERE("#       x               expf(x)          class    bitti\n");
    for (int i = 0; i < N; i++) {
        float x = INPUTS[i];
        float y = expf(x);
        int c = fpclassify(y);
        /* Format: x in %.4f if finite, else scripta verbalis */
        if (isfinite(x)) {
            DICERE("#   %+9.4f     %.4e       %s      0x%08" PRIx32 "\n",
                   (double)x, (double)y, classis_nomen(c), bitti(y));
        } else {
            DICERE("#   %9s     %.4e       %s      0x%08" PRIx32 "\n",
                   (x > 0) ? "+INF" : "-INF",
                   (double)y, classis_nomen(c), bitti(y));
        }
        SUSSURRO("# [I] x=%.6e y=%.9e\n", (double)x, (double)y);
    }
    DICERE("#\n");
    DICERE("# Clave observationes:\n");
    DICERE("#   expf(-INF) = +0      (definitum [C99 §7.12.6.1])\n");
    DICERE("#   expf(+INF) = +INF    (definitum)\n");
    DICERE("#   expf(  0)  = 1       (exactum)\n");
    DICERE("#   expf(-103.97) transit NOR -> SUB -> NUL.\n");
    DICERE("#   expf(+88.73) superflu ad INF.\n");
    DICERE("#\n");
}

/* ========================================================================
 * III. SOFTMAX: NAIVUS ET STABILIS
 * ==================================================================== */

static void
softmax_naivus(const float *x, int n, float *y)
{
    float Z = 0.0f;
    for (int i = 0; i < n; i++) Z += expf(x[i]);
    for (int i = 0; i < n; i++) y[i] = expf(x[i]) / Z;
}

static void
softmax_stabilis(const float *x, int n, float *y)
{
    float maximus = x[0];
    for (int i = 1; i < n; i++) if (x[i] > maximus) maximus = x[i];
    float Z = 0.0f;
    for (int i = 0; i < n; i++) {
        y[i] = expf(x[i] - maximus);
        Z += y[i];
    }
    if (Z > 0.0f) {
        float invZ = 1.0f / Z;
        for (int i = 0; i < n; i++) y[i] *= invZ;
    }
}

/* ========================================================================
 * IV. SECTIO II — SOFTMAX NAIVUS RUINATUS
 * ==================================================================== */

static void
sectio_softmax(void)
{
    DICERE("# ─── II. SOFTMAX: NAIVUS vs STABILIS ───────────────────────\n");
    typedef struct { const char *nomen; float x[5]; int n; } Casus;
    Casus casus[] = {
        { "parvus",      { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f }, 5 },
        { "translatus",  { 91.0f, 92.0f, 93.0f, 94.0f, 95.0f }, 5 },
        { "extremus",    { 0.0f, 50.0f, 100.0f, 200.0f, 300.0f }, 5 },
        { "negativus",   { -150.0f, -149.0f, -148.0f, -147.0f, -146.0f }, 5 }
    };
    const int N = 4;

    for (int k = 0; k < N; k++) {
        float yn[5], ys[5];
        softmax_naivus(casus[k].x, casus[k].n, yn);
        softmax_stabilis(casus[k].x, casus[k].n, ys);
        DICERE("# Casus '%s': x = [", casus[k].nomen);
        for (int i = 0; i < casus[k].n; i++) {
            DICERE("%s%.1f", i ? ", " : "", (double)casus[k].x[i]);
        }
        DICERE("]\n");
        DICERE("#   naivus:    ");
        for (int i = 0; i < casus[k].n; i++) {
            if (isnan(yn[i])) DICERE("   NaN  ");
            else              DICERE(" %.4f", (double)yn[i]);
        }
        DICERE("\n");
        DICERE("#   stabilis:  ");
        for (int i = 0; i < casus[k].n; i++) {
            DICERE(" %.4f", (double)ys[i]);
        }
        DICERE("\n");
        /* Summa pro probatio */
        float Sn = 0.0f, Ss = 0.0f;
        for (int i = 0; i < casus[k].n; i++) {
            if (!isnan(yn[i])) Sn += yn[i];
            Ss += ys[i];
        }
        DICERE("#   sum(naivus)=%.4f, sum(stabilis)=%.4f\n",
               (double)Sn, (double)Ss);
        SUSSURRO("# [II] casus=%s Sn=%.9e Ss=%.9e\n",
                 casus[k].nomen, (double)Sn, (double)Ss);
    }
    DICERE("# Observatio: naivus ruinatur cum max(x) > 88.72 aut\n");
    DICERE("# cum omnes x < -103.97 (Z -> 0, divisio per zero -> NaN/INF).\n");
    DICERE("#\n");
}

/* ========================================================================
 * V. LOG-SUM-EXP STABILIS
 * ====================================================================
 * logsumexp(x) = M + log(sum exp(x_i - M))   ubi M = max(x).
 * Probatio: exp(logsumexp) = exp(M) * sum exp(x - M) = sum exp(x). */

static float
logsumexp(const float *x, int n)
{
    float M = x[0];
    for (int i = 1; i < n; i++) if (x[i] > M) M = x[i];
    float S = 0.0f;
    for (int i = 0; i < n; i++) S += expf(x[i] - M);
    return M + logf(S);
}

static float
logsumexp_naivus(const float *x, int n)
{
    float S = 0.0f;
    for (int i = 0; i < n; i++) S += expf(x[i]);
    return logf(S);
}

/* ========================================================================
 * VI. SECTIO III — LOG-SUM-EXP
 * ==================================================================== */

static void
sectio_logsumexp(void)
{
    DICERE("# ─── III. LOG-SUM-EXP ─────────────────────────────────────\n");
    typedef struct { const char *n; float x[6]; int len; } Casus;
    Casus casus[] = {
        { "parvus",    { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f }, 6 },
        { "medius",    { 50.0f, 51.0f, 52.0f, 53.0f, 54.0f, 55.0f }, 6 },
        { "magnus",    { 200.0f, 201.0f, 202.0f, 203.0f, 204.0f, 205.0f }, 6 },
        { "parvissi",  { -200.0f, -199.0f, -198.0f, -197.0f, -196.0f, -195.0f }, 6 }
    };
    DICERE("# casus       LSE_naivus    LSE_stabilis   max(x)\n");
    for (int k = 0; k < 4; k++) {
        float ln = logsumexp_naivus(casus[k].x, casus[k].len);
        float ls = logsumexp(casus[k].x, casus[k].len);
        float M = casus[k].x[0];
        for (int i = 1; i < casus[k].len; i++)
            if (casus[k].x[i] > M) M = casus[k].x[i];
        DICERE("# %-10s  ", casus[k].n);
        if (isfinite(ln)) DICERE(" %+8.4f    ", (double)ln);
        else              DICERE("  %s/%s    ",
                                 isnan(ln) ? "NaN" : (ln > 0 ? "+INF" : "-INF"),
                                 "     ");
        DICERE("%+8.4f       %+.1f\n",
               (double)ls, (double)M);
        SUSSURRO("# [III] %s ln=%.6e ls=%.6e\n",
                 casus[k].n, (double)ln, (double)ls);
    }
    DICERE("# Stabilis numquam supereffluxit; naivus fallit cum max > 88.\n");
    DICERE("#\n");
}

/* ========================================================================
 * VII. ENTROPIA CRUCIALIS STABILIS
 * ====================================================================
 * L = -sum t_i * log(softmax(x)_i)
 *   = -sum t_i * (x_i - logsumexp(x))
 *   = logsumexp(x) - sum t_i * x_i     (si t est distributio valida)
 *   = logsumexp(x) - x_target          (si t est one-hot).
 * Hoc evitat log(0) omnino. */

static float
cross_entropy_stabilis(const float *logit, int n, int target)
{
    float lse = logsumexp(logit, n);
    return lse - logit[target];
}

static float
cross_entropy_naiva(const float *logit, int n, int target)
{
    float sm[16];
    softmax_naivus(logit, n, sm);
    if (sm[target] <= 0.0f) return INFINITY;
    return -logf(sm[target]);
}

static void
sectio_crossentropy(void)
{
    DICERE("# ─── IV. CROSS-ENTROPY STABILIS ────────────────────────────\n");
    typedef struct { const char *n; float x[5]; int target; } Casus;
    Casus c[] = {
        { "soft",       { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f }, 4 },
        { "confusus",   { -100.0f, -99.0f, -98.0f, -97.0f, -96.0f }, 4 },
        { "magnus",     { 100.0f, 101.0f, 102.0f, 103.0f, 104.0f }, 4 },
        { "target_mal", { 0.0f, 0.0f, 0.0f, 0.0f, 200.0f }, 0 }
    };
    DICERE("# casus         naivus_L    stabilis_L\n");
    for (int k = 0; k < 4; k++) {
        float Ln = cross_entropy_naiva(c[k].x, 5, c[k].target);
        float Ls = cross_entropy_stabilis(c[k].x, 5, c[k].target);
        DICERE("# %-12s ", c[k].n);
        if (isfinite(Ln)) DICERE("  %+9.4f", (double)Ln);
        else              DICERE("  %-9s", isinf(Ln) ? "+INF" : "NaN");
        DICERE("   %+9.4f\n", (double)Ls);
        SUSSURRO("# [IV] %s Ln=%.6e Ls=%.6e\n",
                 c[k].n, (double)Ln, (double)Ls);
    }
    DICERE("# Cross-entropy stabilis loss(x, t) = lse(x) - x_t.\n");
    DICERE("# Nullus logf(0), nulla supereffluxio.\n");
    DICERE("#\n");
}

/* ========================================================================
 * VIII. MASCA -INFINITY (ATTENTIO CAUSALIS)
 * ==================================================================== */

static void
sectio_masca(void)
{
    DICERE("# ─── V. MASCA -INFINITY (ATTENTIO CAUSALIS) ────────────────\n");
    /* Scores "lost" in positionibus 1, 2: masca -INF */
    float scores[5] = { 2.0f, -INFINITY, -INFINITY, 1.0f, 3.0f };
    float y[5];
    softmax_stabilis(scores, 5, y);
    DICERE("# scores: [ 2.0, -INF, -INF,  1.0,  3.0]\n");
    DICERE("# softmax: [");
    for (int i = 0; i < 5; i++) {
        DICERE("%s %.4f", i ? "," : "", (double)y[i]);
    }
    DICERE("]\n");
    DICERE("# Positionibus masatis (y_1, y_2) = 0 exacte (expf(-INF) = 0).\n");
    DICERE("# Summa = %.4f (confirma distributionem validam).\n",
           (double)(y[0] + y[1] + y[2] + y[3] + y[4]));

    /* Illustratio attentionis causalis: 5-sequentia, T_ij = -INF
     * ubi j > i (futura occlusa). */
    DICERE("#\n# Matrix attentionis causalis post softmax (T = 5):\n");
    DICERE("#    j=0     1     2     3     4\n");
    for (int i = 0; i < 5; i++) {
        float sc[5];
        for (int j = 0; j < 5; j++) {
            sc[j] = (j <= i) ? (float)(0.3f * (float)(i + j)) : -INFINITY;
        }
        float a[5];
        softmax_stabilis(sc, 5, a);
        DICERE("# i=%d  ", i);
        for (int j = 0; j < 5; j++) DICERE("%.3f ", (double)a[j]);
        DICERE("\n");
    }
    DICERE("# Superior triangularis exacte zero; nulla intensitas perdita.\n");
    DICERE("#\n");
}

/* ========================================================================
 * IX. GUMBEL-SOFTMAX ET TEMPERATURA
 * ====================================================================
 * Gumbel-softmax [JGP17]: g_i = -log(-log(u_i)) cum u_i uniformes.
 * y = softmax((x + g) / tau). tau -> 0 : quasi-argmax.
 * tau -> inf : quasi-uniformis. */

static void
sectio_gumbel(void)
{
    DICERE("# ─── VI. GUMBEL-SOFTMAX ET TEMPERATURA [JGP17] ─────────────\n");
    /* Logiti fixi, perturbationes Gumbel deterministicae */
    float logit[5] = { 0.5f, 1.2f, 0.8f, 2.0f, 0.1f };
    float gumbel[5];
    {
        /* u ~ Uniform(0,1) fixi deterministici */
        float u[5] = { 0.17f, 0.42f, 0.63f, 0.88f, 0.31f };
        for (int i = 0; i < 5; i++) {
            gumbel[i] = -logf(-logf(u[i]));
        }
    }
    static const float TAU[] = { 5.0f, 1.0f, 0.5f, 0.1f, 0.01f };
    const int NT = 5;
    DICERE("# Logiti = [0.5, 1.2, 0.8, 2.0, 0.1], argmax = index 3\n");
    DICERE("#   tau       y_0    y_1    y_2    y_3    y_4    H(p)\n");
    for (int t = 0; t < NT; t++) {
        float scaled[5];
        for (int i = 0; i < 5; i++)
            scaled[i] = (logit[i] + gumbel[i]) / TAU[t];
        float y[5];
        softmax_stabilis(scaled, 5, y);
        float H = 0.0f;
        for (int i = 0; i < 5; i++) {
            if (y[i] > 0.0f) H -= y[i] * logf(y[i]);
        }
        DICERE("#  %6.2f   ", (double)TAU[t]);
        for (int i = 0; i < 5; i++) DICERE("%.3f  ", (double)y[i]);
        DICERE("%.4f\n", (double)H);
        SUSSURRO("# [VI] tau=%.4f H=%.6e\n", (double)TAU[t], (double)H);
    }
    DICERE("# tau -> 0 : distributio ad delta-Dirac (H -> 0).\n");
    DICERE("# tau -> inf : distributio uniformis (H -> log(5) = 1.6094).\n");
    DICERE("#\n");
}

/* ========================================================================
 * X. SECTIO VII — ENTROPIA VS TEMPERATURA (DIAGRAMMA)
 * ==================================================================== */

static void
sectio_entropia_diagram(void)
{
    DICERE("# ─── VII. H(softmax(x/T)) VS T — DIAGRAMMA ─────────────────\n");
    float logit[5] = { 0.5f, 1.2f, 0.8f, 2.0f, 0.1f };
    DICERE("#  T        entropia   bar\n");
    static const float TAUS[] = {
        0.01f, 0.02f, 0.05f, 0.1f, 0.2f, 0.5f, 1.0f,
        2.0f, 5.0f, 10.0f, 100.0f
    };
    const int NT = (int)(sizeof TAUS / sizeof TAUS[0]);
    const float H_MAX = logf(5.0f);   /* 1.6094 */
    for (int t = 0; t < NT; t++) {
        float scaled[5];
        for (int i = 0; i < 5; i++) scaled[i] = logit[i] / TAUS[t];
        float y[5];
        softmax_stabilis(scaled, 5, y);
        float H = 0.0f;
        for (int i = 0; i < 5; i++)
            if (y[i] > 0.0f) H -= y[i] * logf(y[i]);
        int bar_len = (int)((double)(H / H_MAX) * 40.0);
        if (bar_len < 0) bar_len = 0;
        if (bar_len > 40) bar_len = 40;
        DICERE("#  %6.2f   %.4f     ", (double)TAUS[t], (double)H);
        for (int b = 0; b < bar_len; b++) DICERE("#");
        DICERE("\n");
    }
    DICERE("# H_max = log(5) = %.4f\n", (double)H_MAX);
    DICERE("#\n");
}

/* ========================================================================
 * XI. CAPUT ET EPILOGUS
 * ==================================================================== */

static void
caput_imprime(void)
{
    DICERE("# ═══════════════════════════════════════════════════════════\n");
    DICERE("# EXPONENS v1.0.0 — Exponentiatio Stabilis binary32\n");
    DICERE("# ═══════════════════════════════════════════════════════════\n");
    DICERE("# Limites expf:\n");
    DICERE("#   log(FLT_MAX)       = %+.4f\n", (double)EXP_LIMES_SUPER);
    DICERE("#   log(FLT_MIN)       = %+.4f\n", (double)EXP_LIMES_SUB);
    DICERE("#   log(FLT_TRUE_MIN)  = %+.4f\n", (double)EXP_LIMES_NUL);
    DICERE("#\n");
}

static void
epilogus(void)
{
    DICERE("# ─── EPILOGUS ──────────────────────────────────────────────\n");
    DICERE("# Sex principia exponentiationis stabilis:\n");
    DICERE("#   1. expf(+88.73) supereffluxit; expf(-103.97) subefflux.\n");
    DICERE("#   2. Softmax stabilis: y_i = exp(x_i - max(x)) / Z.\n");
    DICERE("#   3. Log-sum-exp: LSE(x) = M + log(sum exp(x_i - M)).\n");
    DICERE("#   4. Cross-entropy ex logit: L = LSE(x) - x_t.\n");
    DICERE("#   5. Masca -INFINITY: exp(-INF) = 0 exacte.\n");
    DICERE("#   6. Temperatura in softmax: controllat entropiam output.\n");
    DICERE("# ═══════════════════════════════════════════════════════════\n");
}

/* ========================================================================
 * XII. FUNCTIO PRINCIPALIS
 * ==================================================================== */

int
main(void)
{
    caput_imprime();
    sectio_limites();
    sectio_softmax();
    sectio_logsumexp();
    sectio_crossentropy();
    sectio_masca();
    sectio_gumbel();
    sectio_entropia_diagram();
    epilogus();
    return 0;
}

/* ========================================================================
 * APPENDIX A — PROBATIO IDENTITATIS LOG-SUM-EXP
 * ====================================================================
 *
 * Sit M = max(x_i).
 *
 *   exp(M + log(sum exp(x_i - M)))
 *     = exp(M) * exp(log(sum exp(x_i - M)))
 *     = exp(M) * sum exp(x_i - M)
 *     = sum exp(M) * exp(x_i - M)
 *     = sum exp(x_i)
 *
 * Ergo LSE(x) = log(sum exp(x_i)). QED.
 *
 * Stabilitas numerica: post translationem, omnes exponentiales
 * valores in (0, 1] iacent (unus exactus 1.0 pro argmax).
 * Summa numquam supereffluxit, et ne subeffluxit nisi OMNES
 * x_i - M < -103.97, quod per definitionem (M = max) fieri
 * non potest — saltem unus terminus est 1.0 exacte.
 *
 * ─────────────────────────────────────────────────────────────────
 * APPENDIX B — CROSS-ENTROPY IDENTITAS
 * ─────────────────────────────────────────────────────────────────
 *
 * Pro target t one-hot cum t_j = 1 in loco j*:
 *   L = -sum_i t_i log(softmax(x)_i)
 *     = -log(softmax(x)_{j*})
 *     = -log(exp(x_{j*}) / Z)
 *     = -x_{j*} + log(Z)
 *     = LSE(x) - x_{j*}
 *
 * Haec forma stabilior est quia:
 *   a. Nullus log(0) fieri potest.
 *   b. LSE(x) stabilis est (vide Appendix A).
 *
 * ─────────────────────────────────────────────────────────────────
 * APPENDIX C — PROPRIETATES GUMBEL-SOFTMAX
 * ─────────────────────────────────────────────────────────────────
 *
 * Gumbel-Max trick: si g_i ~ Gumbel(0,1) iid, tunc
 *   argmax_i (log pi + g_i)  ~  Categorical(pi)
 *
 * Ubi pi = softmax(log pi). Substituendo argmax per softmax cum
 * temperature tau, obtenemus Gumbel-softmax [JGP17]:
 *   y = softmax((log pi + g) / tau)
 *
 * Cum tau -> 0, y -> one-hot argmax (sed gradiens exstat, ideo
 * trick "reparameterization" utilis est in VAE categoricis).
 * Cum tau -> inf, y -> uniformis.
 *
 * ═══════════════════════════════════════════════════════════════════ */
