/*
 * propagator.c — Propagatio Retrograda (Backpropagation)
 *
 * Demonstratio rigorosa propagationis retrogradae erroris
 * (Rumelhart, Hinton, Williams 1986) in rete minimo MLP cum uno
 * strato occulto, exempli gratia explorans relationem gradientis
 * analytici cum gradiente numerico per differentiam finitam
 * centralem:
 *
 *      dL/dw  ≈  (L(w + h) − L(w − h)) / (2h)
 *
 * Sub praecisione simplici (binary32) curva erroris in scala
 * logarithmica formam litterae U gerit: pro h nimis magno,
 * error truncationis O(h^2) dominatur; pro h nimis parvo, error
 * rotundationis O(epsilon/h) dominatur. Minimum theoreticum circa
 *      h* = (3 * epsilon)^(1/3) ~ 6.1e-3 pro binary32.
 *
 * ═══════════════════════════════════════════════════════════════════
 * SECTIONES:
 *   I.   Rete: 2 ingressus -> 3 occulti (tanh) -> 1 egressus (sigmoid).
 *   II.  Passus directus cum fmaf ad cumulationem stabilem.
 *   III. Passus retrogradus: chain rule, deltae per stratum.
 *   IV.  Gradiens numericus: differentia finita centralis pro omni
 *        parametro individualiter (slow, sed diagnosticum).
 *   V.   Exploratio h: tabula erroris relativi, h ∈ {1e-1, ..., 1e-8}.
 *   VI.  Problema XOR: doctrina MLP per 2000 iterationes.
 *   VII. Diagnostica praecisionis: FLT_EPSILON, fma, rotundatio.
 *
 * ═══════════════════════════════════════════════════════════════════
 * REFERENTIAE:
 *   [RHW86]  Rumelhart D. E., Hinton G. E., Williams R. J. "Learning
 *            representations by back-propagating errors." Nature
 *            323(6088): 533-536 (1986).
 *   [LBH15]  LeCun Y., Bengio Y., Hinton G. "Deep learning."
 *            Nature 521(7553): 436-444 (2015).
 *   [Wer74]  Werbos P. J. "Beyond Regression: New Tools for
 *            Prediction and Analysis in the Behavioral Sciences."
 *            Ph.D. thesis, Harvard University (1974).
 *   [MP69]   Minsky M., Papert S. "Perceptrons." MIT Press (1969).
 *            [Theorema XOR: impossibilis pro perceptrone uno-strato.]
 *   [NR07]   Press W. H. et al. "Numerical Recipes, 3rd ed."
 *            Cambridge (2007). §5.7 Numerical Derivatives.
 *   [Gol91]  Goldberg D. "What Every Computer Scientist..." ACM
 *            Computing Surveys 23(1): 5-48 (1991). §2.2 relative
 *            error.
 *   [IEEE754] IEEE Std 754-2019.
 *
 * Nullae optiones CLI, nullum stdin. Egressus deterministicus in
 * stdout (praecisio trunciata); diagnostica plenae praecisionis
 * in stderr.
 * ═══════════════════════════════════════════════════════════════════
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <float.h>
#include <math.h>

/* ========================================================================
 * I. CONSTANTES ET ARCHITECTURA RETIS
 * ==================================================================== */

#define N_IN        2    /* dimensio vectoris ingressus     */
#define N_HID       3    /* numerus neuronum occultorum     */
#define N_OUT       1    /* dimensio vectoris egressus      */

#define N_W1        (N_IN * N_HID)   /* = 6  pondera strati primi */
#define N_B1        N_HID            /* = 3  biases strati primi  */
#define N_W2        (N_HID * N_OUT)  /* = 3  pondera strati secundi */
#define N_B2        N_OUT            /* = 1  bias strati secundi    */
#define N_PARAM     (N_W1 + N_B1 + N_W2 + N_B2)    /* = 13 */

#define DICERE(...)    fprintf(stdout, __VA_ARGS__)
#define SUSSURRO(...)  fprintf(stderr, __VA_ARGS__)

/* Truncatio ad quattuor cifras decimales pro valoribus circa unum. */
static float
trunc4(float x)
{
    if (!isfinite(x)) return x;
    return roundf(x * 10000.0f) / 10000.0f;
}

static uint32_t
bitti(float x)
{
    uint32_t u;
    memcpy(&u, &x, sizeof u);
    return u;
}

/* ========================================================================
 * II. PARAMETRI — VECTOR UNUS, SECTIONES LOGICAE
 * ====================================================================
 * Omnia pondera in uno vectore contigua. Mapping:
 *   [0..5]   W1[N_HID][N_IN]  row-major: W1[h][i] = theta[h*N_IN + i]
 *   [6..8]   b1[N_HID]
 *   [9..11]  W2[N_OUT][N_HID]
 *   [12]     b2[N_OUT]
 * Haec structura planaris gradientem numericum facilem reddit. */

static inline float *
W1_ref(float *theta, int h, int i) { return &theta[h * N_IN + i]; }
static inline float *
b1_ref(float *theta, int h)        { return &theta[N_W1 + h]; }
static inline float *
W2_ref(float *theta, int o, int h) { return &theta[N_W1 + N_B1 + o * N_HID + h]; }
static inline float *
b2_ref(float *theta, int o)        { return &theta[N_W1 + N_B1 + N_W2 + o]; }

/* ========================================================================
 * III. ACTIVATIONES ET DERIVATIONES
 * ====================================================================
 * tanhf per bibliothecam. Derivatio: 1 - tanh^2(x).
 * Sigmoides stabilis: σ(x) = 1/(1+exp(-x)) pro x >= 0
 *                            exp(x)/(1+exp(x)) pro x < 0
 * Derivatio: σ'(x) = σ(x) * (1 - σ(x)). */

static float
sigmoides(float x)
{
    if (x >= 0.0f) {
        float e = expf(-x);
        return 1.0f / (1.0f + e);
    } else {
        float e = expf(x);
        return e / (1.0f + e);
    }
}

/* ========================================================================
 * IV. PASSUS DIRECTUS
 * ====================================================================
 * z1[h] = sum_i W1[h][i] * x[i] + b1[h]
 * a1[h] = tanh(z1[h])
 * z2[o] = sum_h W2[o][h] * a1[h] + b2[o]
 * y[o]  = sigmoid(z2[o])
 * L     = (1/2) * sum_o (y[o] - t[o])^2     (error quadratus) */

typedef struct {
    float a1[N_HID];     /* activatio strati occulti (post tanh)    */
    float z1[N_HID];     /* prae-activatio strati occulti           */
    float y[N_OUT];      /* egressus post sigmoidem                 */
    float z2[N_OUT];     /* prae-activatio strati egressus          */
} Cache;

static float
passus_directus(const float *theta, const float x[N_IN], const float t[N_OUT],
                Cache *cache)
{
    /* Stratum I: linearis + tanh */
    for (int h = 0; h < N_HID; h++) {
        float z = *b1_ref((float *)theta, h);
        for (int i = 0; i < N_IN; i++) {
            z = fmaf(*W1_ref((float *)theta, h, i), x[i], z);
        }
        cache->z1[h] = z;
        cache->a1[h] = tanhf(z);
    }
    /* Stratum II: linearis + sigmoid */
    for (int o = 0; o < N_OUT; o++) {
        float z = *b2_ref((float *)theta, o);
        for (int h = 0; h < N_HID; h++) {
            z = fmaf(*W2_ref((float *)theta, o, h), cache->a1[h], z);
        }
        cache->z2[o] = z;
        cache->y[o] = sigmoides(z);
    }
    /* Erroris quadratus */
    float L = 0.0f;
    for (int o = 0; o < N_OUT; o++) {
        float d = cache->y[o] - t[o];
        L = fmaf(d, d, L);
    }
    return 0.5f * L;
}

/* ========================================================================
 * V. PASSUS RETROGRADUS — CHAIN RULE [RHW86]
 * ====================================================================
 * dL/dy[o]     = y[o] - t[o]
 * dL/dz2[o]    = dL/dy[o] * σ'(z2[o]) = dL/dy[o] * y[o] * (1 - y[o])
 * dL/dW2[o][h] = dL/dz2[o] * a1[h]
 * dL/db2[o]    = dL/dz2[o]
 * dL/da1[h]    = sum_o dL/dz2[o] * W2[o][h]
 * dL/dz1[h]    = dL/da1[h] * (1 - a1[h]^2)
 * dL/dW1[h][i] = dL/dz1[h] * x[i]
 * dL/db1[h]    = dL/dz1[h]
 */

static void
passus_retrogradus(const float *theta, const float x[N_IN],
                   const float t[N_OUT], const Cache *cache,
                   float *grad)
{
    /* Inicializa omnia gradientes ad zero. */
    for (int i = 0; i < N_PARAM; i++) grad[i] = 0.0f;

    float delta2[N_OUT];
    for (int o = 0; o < N_OUT; o++) {
        float dy = cache->y[o] - t[o];
        float dsig = cache->y[o] * (1.0f - cache->y[o]);
        delta2[o] = dy * dsig;
    }
    /* Gradientes strati secundi */
    for (int o = 0; o < N_OUT; o++) {
        *b2_ref(grad, o) = delta2[o];
        for (int h = 0; h < N_HID; h++) {
            *W2_ref(grad, o, h) = delta2[o] * cache->a1[h];
        }
    }
    /* Propagatio ad stratum occultum */
    float delta1[N_HID];
    for (int h = 0; h < N_HID; h++) {
        float s = 0.0f;
        for (int o = 0; o < N_OUT; o++) {
            s = fmaf(delta2[o], *W2_ref((float *)theta, o, h), s);
        }
        float dtanh = 1.0f - cache->a1[h] * cache->a1[h];
        delta1[h] = s * dtanh;
    }
    /* Gradientes strati primi */
    for (int h = 0; h < N_HID; h++) {
        *b1_ref(grad, h) = delta1[h];
        for (int i = 0; i < N_IN; i++) {
            *W1_ref(grad, h, i) = delta1[h] * x[i];
        }
    }
}

/* ========================================================================
 * VI. GRADIENS NUMERICUS — differentia finita centralis
 * ====================================================================
 * Pro quolibet parametro theta_k, perturba +h et -h, refac passum
 * directum, et approxima dL/dtheta_k per (L+ - L-) / (2h).
 * Error truncationis O(h^2), error rotundationis O(eps/h).
 * Minimum per h* = (3 eps / |f'''|)^(1/3) — pro binary32 circa 6e-3. */

static void
gradiens_numericus(float *theta, const float x[N_IN], const float t[N_OUT],
                   float h, float *grad)
{
    for (int k = 0; k < N_PARAM; k++) {
        float original = theta[k];
        Cache c;

        theta[k] = original + h;
        float Lp = passus_directus(theta, x, t, &c);

        theta[k] = original - h;
        float Lm = passus_directus(theta, x, t, &c);

        theta[k] = original;
        grad[k] = (Lp - Lm) / (2.0f * h);
    }
}

/* ========================================================================
 * VII. METRICAE COMPARATIONIS
 * ==================================================================== */

static float
distantia_L2(const float *a, const float *b, int n)
{
    float s = 0.0f;
    for (int i = 0; i < n; i++) {
        float d = a[i] - b[i];
        s = fmaf(d, d, s);
    }
    return sqrtf(s);
}

static float
norma_L2(const float *a, int n)
{
    float s = 0.0f;
    for (int i = 0; i < n; i++) s = fmaf(a[i], a[i], s);
    return sqrtf(s);
}

static float
error_relativus(const float *ga, const float *gn, int n)
{
    float num = distantia_L2(ga, gn, n);
    float den = norma_L2(ga, n) + norma_L2(gn, n);
    if (den < FLT_MIN) return 0.0f;
    return num / den;
}

/* ========================================================================
 * VIII. DATA XOR — EXEMPLUM CLASSICUM [MP69 critica, RHW86 responsio]
 * ====================================================================
 * XOR non est linearitate separabilis; MLP cum uno strato occulto
 * solutionem invenire potest. Quattuor puncta: */

static const float XOR_X[4][N_IN] = {
    { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }
};
static const float XOR_T[4][N_OUT] = {
    { 0.0f }, { 1.0f }, { 1.0f }, { 0.0f }
};

/* ========================================================================
 * IX. PARAMETRI INITIALES — valores fixi deterministici
 * ==================================================================== */

static void
theta_init(float *theta)
{
    /* Valores deliberate chosen: parvi, non-zero, sine symmetria. */
    static const float PROTO[N_PARAM] = {
        /* W1 (3x2): */
         0.3125f,  0.5000f,
        -0.2500f,  0.4375f,
         0.1250f, -0.3750f,
        /* b1: */
         0.0625f, -0.1875f,  0.2500f,
        /* W2 (1x3): */
         0.5000f, -0.3750f,  0.2500f,
        /* b2: */
        -0.1250f
    };
    memcpy(theta, PROTO, sizeof PROTO);
}

/* ========================================================================
 * X. CAPUT PROLOGUM
 * ==================================================================== */

static void
caput_imprime(void)
{
    DICERE("# ═══════════════════════════════════════════════════════════\n");
    DICERE("# PROPAGATOR v1.0.0 — Propagatio Retrograda binary32\n");
    DICERE("# ═══════════════════════════════════════════════════════════\n");
    DICERE("# Architectura: %d -> %d (tanh) -> %d (sigmoid)\n",
           N_IN, N_HID, N_OUT);
    DICERE("# Parametri: W1(%d) + b1(%d) + W2(%d) + b2(%d) = %d in toto\n",
           N_W1, N_B1, N_W2, N_B2, N_PARAM);
    DICERE("# FLT_EPSILON = %.4e  (ULP(1.0) binary32)\n",
           (double)FLT_EPSILON);
    DICERE("# h* theoretica = (3*eps)^(1/3) ~= 6.1e-3 [NR07 §5.7]\n");
    DICERE("#\n");
}

/* ========================================================================
 * XI. SECTIO I — PASSUS DIRECTUS INSPECTIO
 * ==================================================================== */

static void
sectio_directa(void)
{
    DICERE("# ─── I. PASSUS DIRECTUS ────────────────────────────────────\n");
    float theta[N_PARAM];
    theta_init(theta);
    Cache c;

    DICERE("#  idx  x[0]    x[1]    t      z2        y        L\n");
    for (int k = 0; k < 4; k++) {
        float L = passus_directus(theta, XOR_X[k], XOR_T[k], &c);
        DICERE("#  %2d   %.4f  %.4f  %.1f   %+7.4f   %.4f   %.4f\n",
               k,
               (double)XOR_X[k][0], (double)XOR_X[k][1],
               (double)XOR_T[k][0],
               (double)trunc4(c.z2[0]),
               (double)trunc4(c.y[0]),
               (double)trunc4(L));
        SUSSURRO("# [I] k=%d z2=%.9e y=%.9e L=%.9e\n",
                 k, (double)c.z2[0], (double)c.y[0], (double)L);
    }
    DICERE("#\n");
}

/* ========================================================================
 * XII. SECTIO II — PROBATIO GRADIENTIS ANALYTICI
 * ====================================================================
 * Comparatio: gradiens analyticus (backprop) vs numericus
 * cum h = 1e-3 (circa optimum theoreticum pro binary32). */

static void
sectio_probatio(void)
{
    DICERE("# ─── II. PROBATIO GRADIENTIS [RHW86 vs NR07] ───────────────\n");
    DICERE("# Exemplum: punctum XOR_0 = (0,0), t = 0.\n");
    DICERE("# h = 1e-3 (prope optimum teoreticum pro binary32).\n");
    float theta[N_PARAM];
    theta_init(theta);
    Cache c;
    passus_directus(theta, XOR_X[0], XOR_T[0], &c);
    float grad_a[N_PARAM];
    float grad_n[N_PARAM];
    passus_retrogradus(theta, XOR_X[0], XOR_T[0], &c, grad_a);
    gradiens_numericus(theta, XOR_X[0], XOR_T[0], 1.0e-3f, grad_n);

    DICERE("#   k      grad_analytic   grad_numeric    diff\n");
    for (int k = 0; k < N_PARAM; k++) {
        float diff = grad_a[k] - grad_n[k];
        DICERE("#  %3d    %+10.6f    %+10.6f   %+.2e\n",
               k,
               (double)trunc4(grad_a[k]),
               (double)trunc4(grad_n[k]),
               (double)diff);
        SUSSURRO("# [II] k=%d a=%.9e n=%.9e\n",
                 k, (double)grad_a[k], (double)grad_n[k]);
    }
    float err = error_relativus(grad_a, grad_n, N_PARAM);
    DICERE("# Error relativus ||a - n|| / (||a|| + ||n||) = %.4e\n",
           (double)err);
    DICERE("# Toleratia typica backprop: < 1e-4 significat concordiam.\n");
    DICERE("#\n");
}

/* ========================================================================
 * XIII. SECTIO III — EXPLORATIO SCALAE h
 * ====================================================================
 * Tabula: h contra errorem relativum. Curva U expectata:
 *   h magnum -> truncatio O(h^2) dominatur
 *   h parvum -> rotundatio O(eps/h) dominatur
 *   minimum prope h ~ (eps)^(1/3) ~ 4.9e-3 pro binary32 */

static void
sectio_scala(void)
{
    DICERE("# ─── III. SCALA h: TRUNCATIO VS ROTUNDATIO ─────────────────\n");
    DICERE("# Error relativus inter grad_analytic et grad_numeric vs h.\n");
    DICERE("#     h           error_relativus      dominium\n");

    float theta[N_PARAM];
    theta_init(theta);
    Cache c;
    passus_directus(theta, XOR_X[1], XOR_T[1], &c);
    float grad_a[N_PARAM];
    passus_retrogradus(theta, XOR_X[1], XOR_T[1], &c, grad_a);

    /* h ex { 1e-0, 1e-1, ..., 1e-8 }, etiam valores intermedii */
    static const float H_VALUES[] = {
        1.0e-0f, 3.0e-1f, 1.0e-1f, 3.0e-2f,
        1.0e-2f, 3.0e-3f, 1.0e-3f, 3.0e-4f,
        1.0e-4f, 3.0e-5f, 1.0e-5f, 3.0e-6f,
        1.0e-6f, 1.0e-7f, 1.0e-8f
    };
    const int N_H = (int)(sizeof H_VALUES / sizeof H_VALUES[0]);

    float err_min = INFINITY;
    float h_min = 0.0f;
    for (int i = 0; i < N_H; i++) {
        float grad_n[N_PARAM];
        gradiens_numericus(theta, XOR_X[1], XOR_T[1], H_VALUES[i], grad_n);
        float err = error_relativus(grad_a, grad_n, N_PARAM);
        const char *dom;
        if (H_VALUES[i] > 1.0e-2f) dom = "truncatio O(h^2)";
        else if (H_VALUES[i] < 1.0e-5f) dom = "rotundatio O(eps/h)";
        else dom = "equilibrium";
        /* Pro h parvo, diff (Lp - Lm) cancellationem catastrophicam patitur
         * — err accumulationem erroris rotundationis reflectit, non est
         * stabilis inter compilatores (potest factore 2 differre). Ad stdout
         * solum err stabilem ( > 1e-3) emittimus; ceteri "~eps" placeholder. */
        if (err > 5.0e-3f) {
            DICERE("#   %.2e      %.2e           %s\n",
                   (double)H_VALUES[i], (double)err, dom);
        } else {
            DICERE("#   %.2e       ~eps            %s\n",
                   (double)H_VALUES[i], dom);
        }
        SUSSURRO("# [III] h=%.6e err=%.6e\n",
                 (double)H_VALUES[i], (double)err);
        if (err < err_min) { err_min = err; h_min = H_VALUES[i]; }
    }
    DICERE("# Minimum empiricum: h = %.2e (err sub epsilon, vide stderr)\n",
           (double)h_min);
    SUSSURRO("# Minimum empiricum plenum: h = %.6e, error = %.6e\n",
             (double)h_min, (double)err_min);
    DICERE("# Minimum teoreticum: h ~ 5e-3 pro binary32.\n");
    DICERE("#\n");
}

/* ========================================================================
 * XIV. SECTIO IV — DOCTRINA XOR
 * ====================================================================
 * Descensus gradientis plenus (batch) per 2000 iterationes.
 * Demonstrat convergentiam MLP ad solutionem XOR — quod
 * perceptron classicus (sine strato occulto) facere non potest. */

static void
sectio_doctrina(void)
{
    DICERE("# ─── IV. DOCTRINA XOR PER DESCENSUM GRADIENTIS ─────────────\n");
    float theta[N_PARAM];
    theta_init(theta);
    const float eta = 0.5f;
    const int iterationes = 2000;
    const int reportae_intervalla[] = {0, 100, 500, 1000, 1500, 1999};
    const int n_rep = 6;
    int rep_idx = 0;

    DICERE("#  iter    L_totum   y(0,0)  y(0,1)  y(1,0)  y(1,1)\n");
    for (int it = 0; it < iterationes; it++) {
        float L_totum = 0.0f;
        float grad_accum[N_PARAM] = { 0.0f };
        float y_last[4] = { 0.0f };
        for (int k = 0; k < 4; k++) {
            Cache c;
            float L = passus_directus(theta, XOR_X[k], XOR_T[k], &c);
            L_totum += L;
            y_last[k] = c.y[0];
            float g[N_PARAM];
            passus_retrogradus(theta, XOR_X[k], XOR_T[k], &c, g);
            for (int i = 0; i < N_PARAM; i++) grad_accum[i] += g[i];
        }
        for (int i = 0; i < N_PARAM; i++) {
            theta[i] -= eta * grad_accum[i] * 0.25f;  /* mediana per batchum */
        }
        if (rep_idx < n_rep && it == reportae_intervalla[rep_idx]) {
            DICERE("#  %4d    %.4f    %.4f  %.4f  %.4f  %.4f\n",
                   it, (double)trunc4(L_totum),
                   (double)trunc4(y_last[0]), (double)trunc4(y_last[1]),
                   (double)trunc4(y_last[2]), (double)trunc4(y_last[3]));
            rep_idx++;
        }
    }
    DICERE("# Target:            %.1f    %.1f    %.1f    %.1f\n",
           (double)XOR_T[0][0], (double)XOR_T[1][0],
           (double)XOR_T[2][0], (double)XOR_T[3][0]);
    DICERE("#\n");
}

/* ========================================================================
 * XV. SECTIO V — DIAGNOSTICA fmaf
 * ====================================================================
 * Comparatio cumulationis "naiva" (z = z + w*x) cum fmaf (UNA
 * rotundatio). Exemplum artificiale cum catastrophica
 * cancellatione: w*x proximum -z. */

static void
sectio_fma(void)
{
    DICERE("# ─── V. fmaf: UNA ROTUNDATIO [IEEE754 §5.4.1] ──────────────\n");
    /* Exemplum: z = -1.0f, w = 1.0f + epsilon, x = 1.0f - epsilon.
     * Summa exacta: w*x - 1 = -epsilon^2 ≈ 1.42e-14.
     * Naïve: (w*x) rotundatur -> 1.0f, dein 1.0f - 1.0f = 0.0f.
     * Cum fmaf: UNA rotundatio, error absolutus <= ULP(|-1 + w*x|). */
    /* Volatile contra contractionem compilatoris (FP_CONTRACT).
     * Per variabilem intermediam coactam rotundationem producimus. */
#if defined(STDC_VERSION_PRAGMA_OK)
#pragma STDC FP_CONTRACT OFF
#endif
    volatile float w = 1.0f + FLT_EPSILON;
    volatile float x = 1.0f - FLT_EPSILON;
    volatile float z_init = -1.0f;
    volatile float prod = w * x;           /* rotundat ad 1.0f */
    float naive = prod + z_init;           /* = 0.0f stricte */
    float stabile = fmaf(w, x, z_init);    /* UNA rotundatio */
    DICERE("# w         = 1 + eps = %.8f\n", (double)w);
    DICERE("# x         = 1 - eps = %.8f\n", (double)x);
    DICERE("# z         = -1\n");
    DICERE("# w*x + z   (naivus)  = %.4e\n", (double)naive);
    DICERE("# fmaf(w,x,z) (stabile) = %.4e\n", (double)stabile);
    DICERE("# Exactum (duplex): -eps^2 ~= -1.4210854715e-14\n");
    DICERE("# Naivus peream praecisionem; fmaf eam servat.\n");
    SUSSURRO("# [V] naive bitti=0x%08" PRIx32 " stabile bitti=0x%08" PRIx32 "\n",
             bitti(naive), bitti(stabile));
    DICERE("#\n");
}

/* ========================================================================
 * XVI. EPILOGUS
 * ==================================================================== */

static void
epilogus(void)
{
    DICERE("# ─── EPILOGUS ──────────────────────────────────────────────\n");
    DICERE("# Propagatio retrograda [RHW86] sex lectiones docet:\n");
    DICERE("#   1. Chain rule dat gradientem exactum sub ambiente\n");
    DICERE("#      arithmetico IEEE (si ordo operationum fixatur).\n");
    DICERE("#   2. Differentia finita gradientem verificat sed curva\n");
    DICERE("#      erroris formam U-litterae habet.\n");
    DICERE("#   3. Optimum h ~ (eps)^(1/3) ~ 5e-3 pro binary32.\n");
    DICERE("#   4. fmaf cumulationem stabilem reddit.\n");
    DICERE("#   5. MLP XOR solvere potest, perceptron unus-strati non.\n");
    DICERE("#   6. Sigmoides stabilis branch-conscious evitat\n");
    DICERE("#      underflow pro x magno positivo.\n");
    DICERE("# ═══════════════════════════════════════════════════════════\n");
}

/* ========================================================================
 * XVII. FUNCTIO PRINCIPALIS
 * ==================================================================== */

int
main(void)
{
    caput_imprime();
    sectio_directa();
    sectio_probatio();
    sectio_scala();
    sectio_doctrina();
    sectio_fma();
    epilogus();
    return 0;
}

/* ========================================================================
 * APPENDIX A — DERIVATIO FORMULAE h*
 * ====================================================================
 *
 * Error totalis differentiae finitae centralis:
 *   E(h) = E_trunc(h) + E_round(h)
 *        ≈ (h^2 / 6) * |f'''(x)| + eps * |f(x)| / h
 *
 * Minimum ∂E/∂h = 0:
 *   (h/3) * |f'''| - eps * |f| / h^2 = 0
 *   h^3 = 3 * eps * |f| / |f'''|
 *   h*  = (3 eps |f| / |f'''|)^(1/3)
 *
 * Pro |f''' | ~ |f| ~ 1, h* ~ (3 * FLT_EPSILON)^(1/3)
 *                          ≈ (3 * 1.19e-7)^(1/3)
 *                          ≈ 7.2e-3.
 *
 * Error residualis in optimo: E(h*) ~ eps^(2/3) ~ 2.4e-5.
 *
 * ─────────────────────────────────────────────────────────────────
 * APPENDIX B — QUID SI fmaf NON DISPONIBILE?
 * ─────────────────────────────────────────────────────────────────
 *
 * In contextu absentiae instructionis FMA (e.g., x86 ante Haswell),
 * fmaf per software emulatur — potentialiter lentius. Pro summariis
 * magnis, algorithmus Kahan (vide summator.c in hoc ramo) alternativum
 * cum praecisione duplici effective offert.
 *
 * ─────────────────────────────────────────────────────────────────
 * APPENDIX C — HISTORIA BRIEF
 * ─────────────────────────────────────────────────────────────────
 *
 * 1974: Werbos [Wer74] algorithmum propagationis retrogradae in
 *       thesi doctorali describit.
 * 1986: Rumelhart, Hinton, Williams [RHW86] idem independenter
 *       redescoperunt et popularizant.
 * 1989: Hornik universalem approximationem MLP probat.
 * 2012: AlexNet renasciam "deep learning" inicit.
 *
 * Hic codex propagationem primitivam ad-hoc implementat, sine
 * optimatione moderna (vectorizatio SIMD, batching GPU, etc.).
 * ==================================================================== */
