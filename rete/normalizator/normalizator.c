/*
 * normalizator.c — Normalizatio Strati (Layer-Norm) cum Variantia
 *
 * Computatio mediae et variantiae vectoris binary32 — cum
 * applicatione ad layer-norm (Ba, Kiros, Hinton 2016) et batch-norm
 * (Ioffe, Szegedy 2015).
 *
 * Quaestio fundamentalis: variantia per formulam
 *     Var(X) = E[X^2] - E[X]^2         (duo passus, "naivus")
 * saepe catastrophice errat cum E[X]^2 proximus est E[X^2] —
 * e.g., cum media est magna et variantia parva. Solutio moderna:
 * algorithmus Welford (1962), qui variantiam online computat sine
 * cancellatione.
 *
 * ═══════════════════════════════════════════════════════════════════
 * METHODI VARIANTIAE:
 *   1. NAIVUS ONE-PASS   — Var = (sum x^2) / n - (sum x / n)^2
 *   2. NAIVUS TWO-PASS   — mediane primo, deinde sum (x - mean)^2
 *   3. WELFORD           — online updater, numerice stabilis
 *   4. YOUNGS-CRAMER     — Welford per blocks
 *   5. REFERENCE         — duplex accumulatio
 *
 * EXPERIMENTA:
 *   A. Data "patrona": x_i = 10^7 + noise ∈ [-1, +1] — mean ~ 1e7,
 *      variantia ~ 1/3. Naivus one-pass catastrophice errat.
 *   B. Layer-norm demonstratio: 16 features, mediana 0, sigma 2.5
 *   C. Effectus epsilon in sqrtf(var + eps) cum var ~ eps.
 *   D. Sensitivitas a ordine: eadem data in ordine permutato.
 *
 * ═══════════════════════════════════════════════════════════════════
 * REFERENTIAE:
 *   [Wel62]  Welford B. P. "Note on a method for calculating corrected
 *            sums of squares and products." Technometrics 4(3):
 *            419-420 (1962).
 *   [Cha83]  Chan T. F., Golub G. H., LeVeque R. J. "Algorithms for
 *            computing the sample variance: Analysis and
 *            recommendations." American Statistician 37(3):
 *            242-247 (1983).
 *   [YC71]   Youngs E. A., Cramer E. M. "Some Results Relevant to
 *            Choice of Sum and Sum-of-Product Algorithms."
 *            Technometrics 13(3): 657-665 (1971).
 *   [IS15]   Ioffe S., Szegedy C. "Batch Normalization: Accelerating
 *            Deep Network Training by Reducing Internal Covariate
 *            Shift." ICML (2015).
 *   [BKH16]  Ba J. L., Kiros J. R., Hinton G. E. "Layer Normalization."
 *            arXiv:1607.06450 (2016).
 *   [WBS92]  Wu C. F. J., Bergman J., Stewart W. "A simple method for
 *            computing the sample variance..." (1992).
 *   [Hig02]  Higham N. J. "Accuracy and Stability..." SIAM (2002).
 *            Cap. 1.9, pp. 11-13.
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

/* ========================================================================
 * I. GENERATOR DETERMINISTICUS (ut summator.c)
 * ==================================================================== */

typedef struct { uint32_t status; } Fors;

static inline uint32_t
xorshift32(Fors *f)
{
    uint32_t x = f->status;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    f->status = x ? x : 0xBADDCAFEu;
    return f->status;
}

/* [0, 1) uniformis, 24 bit */
static float
forte_unus(Fors *f)
{
    return (float)(xorshift32(f) >> 8) * ldexpf(1.0f, -24);
}

/* [-1, +1] */
static float
forte_signatus(Fors *f)
{
    return 2.0f * forte_unus(f) - 1.0f;
}

/* Approximatio normalis N(0,1) per theorema centrale limitis,
 * summa 12 uniformium - 6 (variantia = 1). Non perfectus sed
 * deterministicus et binary32-compatibilis. */
static float
forte_normalis(Fors *f)
{
    float s = 0.0f;
    for (int i = 0; i < 12; i++) s += forte_unus(f);
    return s - 6.0f;
}

/* ========================================================================
 * II. ALGORITHMI MEDIAE ET VARIANTIAE
 * ==================================================================== */

/* NAIVUS ONE-PASS — fallit pro media magna.
 * Var = (sum x^2) / n - (sum x / n)^2 */
static void
stat_naivus_onepass(const float *x, int n, float *mean_out, float *var_out)
{
    float s1 = 0.0f, s2 = 0.0f;
    for (int i = 0; i < n; i++) {
        s1 += x[i];
        s2 += x[i] * x[i];
    }
    float m = s1 / (float)n;
    float v = s2 / (float)n - m * m;
    *mean_out = m;
    *var_out = v;
}

/* NAIVUS TWO-PASS — mediane primo, deinde sum (x - mean)^2.
 * Conspicue melius quam one-pass; tamen adhuc non optimum. */
static void
stat_naivus_twopass(const float *x, int n, float *mean_out, float *var_out)
{
    float s = 0.0f;
    for (int i = 0; i < n; i++) s += x[i];
    float m = s / (float)n;
    float ss = 0.0f;
    for (int i = 0; i < n; i++) {
        float d = x[i] - m;
        ss += d * d;
    }
    *mean_out = m;
    *var_out = ss / (float)n;
}

/* WELFORD [Wel62] — online, stabilis.
 * Formula recurrentia:
 *   mean_{k+1} = mean_k + (x_{k+1} - mean_k) / (k+1)
 *   M2_{k+1}   = M2_k   + (x_{k+1} - mean_k) * (x_{k+1} - mean_{k+1})
 *   Var        = M2_n / n   (population)
 *                M2_n / (n-1)  (sample, correctio Bessel) */
static void
stat_welford(const float *x, int n, float *mean_out, float *var_out)
{
    float mean = 0.0f;
    float M2 = 0.0f;
    for (int i = 0; i < n; i++) {
        float delta = x[i] - mean;
        mean = mean + delta / (float)(i + 1);
        float delta2 = x[i] - mean;
        M2 = M2 + delta * delta2;
    }
    *mean_out = mean;
    *var_out = M2 / (float)n;
}

/* YOUNGS-CRAMER [YC71, Cha83] — compensatus "two-pass" per blocks.
 * Simplex forma: conserva (sum, sum_sq_diff_mean) pro quoque blocco,
 * combina pairwise. Hic versio scalaris: aequivalens Welford una-passu. */
static void
stat_youngs_cramer(const float *x, int n, float *mean_out, float *var_out)
{
    /* Pro simplicitate, idem ac Welford — principialiter Youngs-Cramer
     * distinguitur per block-recombination (vide Chan et al. 1983 §3) */
    stat_welford(x, n, mean_out, var_out);
}

/* REFERENCE: accumulatio duplex. Tanquam veritas. */
static void
stat_referens(const float *x, int n, double *mean_out, double *var_out)
{
    double s = 0.0;
    for (int i = 0; i < n; i++) s += (double)x[i];
    double m = s / (double)n;
    double ss = 0.0;
    for (int i = 0; i < n; i++) {
        double d = (double)x[i] - m;
        ss += d * d;
    }
    *mean_out = m;
    *var_out = ss / (double)n;
}

/* ========================================================================
 * III. LAYER NORMALIZATIO [BKH16]
 * ====================================================================
 * y_i = gamma * (x_i - mean(x)) / sqrt(var(x) + eps) + beta
 * Transformatio ensures mean(y) = beta, std(y) = gamma (post-
 * affine). Epsilon typice 1e-5 pro fp32, 1e-3 pro fp16. */

static void
layer_norm(const float *x, int n, float eps, float gamma, float beta,
           float *y, float *mean_out, float *var_out)
{
    float m, v;
    stat_welford(x, n, &m, &v);
    float rstd = 1.0f / sqrtf(v + eps);
    for (int i = 0; i < n; i++) {
        y[i] = gamma * (x[i] - m) * rstd + beta;
    }
    *mean_out = m;
    *var_out = v;
}

/* ========================================================================
 * IV. DIAGNOSTICA
 * ==================================================================== */

static void
imprime_statistica(const char *nomen, float m_f32, float v_f32,
                   double m_ref, double v_ref)
{
    float em = (float)((double)m_f32 - m_ref);
    float ev = (float)((double)v_f32 - v_ref);
    float rel_m = (m_ref != 0.0) ? fabsf(em) / (float)fabs(m_ref) : 0.0f;
    float rel_v = (v_ref > 0.0) ? fabsf(ev) / (float)fabs(v_ref) : 0.0f;
    DICERE("#  %-14s  mean=%+.2e  var=%.2e  (err_rel_m=%.1e, err_rel_v=%.1e)\n",
           nomen, (double)m_f32, (double)v_f32,
           (double)rel_m, (double)rel_v);
    SUSSURRO("#  %-14s  mean=%+.9e  var=%.9e  err_rel_m=%.6e  err_rel_v=%.6e\n",
             nomen, (double)m_f32, (double)v_f32,
             (double)rel_m, (double)rel_v);
}

/* ========================================================================
 * V. EXPERIMENTUM A — PATRONA: MEDIA MAGNA, VARIANTIA PARVA
 * ====================================================================
 * Data x_i = 10^7 + u_i cum u_i ∈ [-1, +1] uniformis.
 * Exacta: mean ~ 1e7, var ~ 1/3 (uniformis in [-1, 1]).
 * Naivus one-pass: E[X^2] ~ 10^14, (E[X])^2 ~ 10^14, differentia
 * cancellat catastrophice — fortasse negativam dat! */

static void
experimentum_patrona(void)
{
    DICERE("# ─── EXPERIMENTUM A: MEDIA MAGNA, VARIANTIA PARVA ──────────\n");
    const int n = 10000;
    float *x = malloc(sizeof(float) * (size_t)n);
    Fors f = { .status = 0x5EED5EEDu };
    for (int i = 0; i < n; i++) {
        x[i] = 1.0e7f + forte_signatus(&f);
    }
    double mref, vref;
    stat_referens(x, n, &mref, &vref);
    float m1, v1, m2, v2, m3, v3;
    stat_naivus_onepass(x, n, &m1, &v1);
    stat_naivus_twopass(x, n, &m2, &v2);
    stat_welford(x, n, &m3, &v3);

    DICERE("# Data: n = %d, x_i = 1e7 + uniform(-1, +1)\n", n);
    DICERE("# Exactum (duplex): mean = %.6f, var = %.6f\n",
           mref, vref);
    imprime_statistica("naivus_1pass",   m1, v1, mref, vref);
    imprime_statistica("naivus_2pass",   m2, v2, mref, vref);
    imprime_statistica("welford",        m3, v3, mref, vref);
    DICERE("# Naivus one-pass potest variantiam NEGATIVAM reddere,\n");
    DICERE("# quod mathematice impossibile est — signum cancellationis.\n");
    SUSSURRO("# [A] mref=%.12e vref=%.12e\n", mref, vref);
    free(x);
    DICERE("#\n");
}

/* ========================================================================
 * VI. EXPERIMENTUM B — LAYER NORM STANDARDIS
 * ==================================================================== */

static void
experimentum_layernorm(void)
{
    DICERE("# ─── EXPERIMENTUM B: LAYER NORMALIZATIO [BKH16] ────────────\n");
    const int n = 16;
    float x[16], y[16];
    Fors f = { .status = 0xABBADABBu };
    for (int i = 0; i < n; i++) x[i] = 2.5f * forte_normalis(&f);
    /* Ante normalizationem */
    float m, v;
    layer_norm(x, n, 1.0e-5f, 1.0f, 0.0f, y, &m, &v);

    DICERE("# n = %d, x ~ 2.5 * N(0, 1) (approximatum)\n", n);
    DICERE("# Ante norm:  mean = %+.6f,  var = %.6f,  sigma = %.6f\n",
           (double)m, (double)v, (double)sqrtf(v));
    /* Post */
    float m2, v2;
    stat_welford(y, n, &m2, &v2);
    DICERE("# Post norm:  mean = %+.6f,  var = %.6f,  sigma = %.6f\n",
           (double)m2, (double)v2, (double)sqrtf(v2));
    DICERE("# Expectatum post: mean ~ 0, var ~ 1.\n");

    /* Ostensio valorum */
    DICERE("#   idx     x[i]         y[i]\n");
    for (int i = 0; i < n; i++) {
        DICERE("#   %2d    %+8.4f     %+8.4f\n",
               i, (double)x[i], (double)y[i]);
    }
    DICERE("#\n");
}

/* ========================================================================
 * VII. EXPERIMENTUM C — EPSILON IN sqrt(var + eps)
 * ====================================================================
 * Si omnes valores constantes sunt, var = 0 exacte, et sqrtf(0 + eps)
 * = sqrtf(eps). Epsilon debet esse tantus ut 1/sqrt(eps) non
 * superfluat ad INF. Typice eps = 1e-5 (fp32), 1e-3 (fp16). */

static void
experimentum_epsilon(void)
{
    DICERE("# ─── EXPERIMENTUM C: EPSILON IN LAYER NORM ─────────────────\n");
    float x_constantes[8];
    for (int i = 0; i < 8; i++) x_constantes[i] = 3.14159f;   /* pi */

    static const float EPSILONS[] = {
        1.0e-10f, 1.0e-8f, 1.0e-6f, 1.0e-5f, 1.0e-4f, 1.0e-3f
    };
    const int NE = (int)(sizeof EPSILONS / sizeof EPSILONS[0]);
    DICERE("# x = [3.14159 x 8] (constans), var = 0 exacte.\n");
    DICERE("#     eps         sqrt(0+eps)     1/sqrt(0+eps)    fpclassify\n");
    for (int k = 0; k < NE; k++) {
        float r = sqrtf(0.0f + EPSILONS[k]);
        float ir = 1.0f / r;
        const char *cl = "NOR";
        int c = fpclassify(ir);
        if (c == FP_INFINITE) cl = "INF";
        else if (c == FP_NAN) cl = "NAN";
        else if (c == FP_SUBNORMAL) cl = "SUB";
        DICERE("#   %.2e     %.4e       %.4e        %s\n",
               (double)EPSILONS[k], (double)r, (double)ir, cl);
    }
    DICERE("# Nota: eps minor quam FLT_MIN^2 ~ 1.4e-76 (pro fp32) potest\n");
    DICERE("# 1/sqrt supereffluere; eps = 1e-5 est tutus defaltus.\n");
    DICERE("#\n");
}

/* ========================================================================
 * VIII. EXPERIMENTUM D — SENSITIVITAS A ORDINE
 * ==================================================================== */

static void
experimentum_ordo(void)
{
    DICERE("# ─── EXPERIMENTUM D: SENSITIVITAS A ORDINE ─────────────────\n");
    const int n = 100;
    float x1[100], x2[100], x3[100];
    Fors f = { .status = 0x12345678u };
    for (int i = 0; i < n; i++) {
        /* Distributio bimodalis: 50% ~ N(-5, 1), 50% ~ N(+5, 1) */
        if (i < 50) x1[i] = -5.0f + forte_normalis(&f);
        else        x1[i] = +5.0f + forte_normalis(&f);
    }
    /* x2: reverse ordo */
    for (int i = 0; i < n; i++) x2[i] = x1[n - 1 - i];
    /* x3: interleave */
    for (int i = 0; i < n; i++) {
        x3[i] = (i % 2 == 0) ? x1[i / 2] : x1[50 + i / 2];
    }
    double mref, vref;
    stat_referens(x1, n, &mref, &vref);

    float m1, v1, m2, v2, m3, v3;
    stat_naivus_onepass(x1, n, &m1, &v1);
    stat_naivus_onepass(x2, n, &m2, &v2);
    stat_naivus_onepass(x3, n, &m3, &v3);
    DICERE("# n = %d, distributio bimodalis N(-5,1) + N(+5,1).\n", n);
    DICERE("# Naivus 1-pass (sensitivus ad ordinem):\n");
    DICERE("#   ordinem 1    mean=%+.3f  var=%.3f\n",
           (double)m1, (double)v1);
    DICERE("#   ordinem 2    mean=%+.3f  var=%.3f\n",
           (double)m2, (double)v2);
    DICERE("#   ordinem 3    mean=%+.3f  var=%.3f\n",
           (double)m3, (double)v3);

    stat_welford(x1, n, &m1, &v1);
    stat_welford(x2, n, &m2, &v2);
    stat_welford(x3, n, &m3, &v3);
    DICERE("# Welford (relative insensitivus):\n");
    DICERE("#   ordinem 1    mean=%+.3f  var=%.3f\n",
           (double)m1, (double)v1);
    DICERE("#   ordinem 2    mean=%+.3f  var=%.3f\n",
           (double)m2, (double)v2);
    DICERE("#   ordinem 3    mean=%+.3f  var=%.3f\n",
           (double)m3, (double)v3);
    DICERE("# Exactum:       mean=%+.3f  var=%.3f\n", mref, vref);
    DICERE("#\n");
}

/* ========================================================================
 * IX. EXPERIMENTUM E — RMSNORM ALTERNATIVUM [Zhang 2019]
 * ====================================================================
 * RMSNorm = x * gamma / sqrt(mean(x^2) + eps)
 * Nulla subtractio mediae. Simplicior quam LayerNorm, tamen
 * praestationem simillimam habet in LLaMA, GPT-NeoX etc. */

static void
experimentum_rmsnorm(void)
{
    DICERE("# ─── EXPERIMENTUM E: RMSNORM [Zhang 2019] ──────────────────\n");
    const int n = 16;
    float x[16];
    Fors f = { .status = 0xBEEFABADu };
    for (int i = 0; i < n; i++) x[i] = forte_normalis(&f);

    /* RMS: root mean square = sqrt(mean(x^2)) */
    float ss = 0.0f;
    for (int i = 0; i < n; i++) ss += x[i] * x[i];
    float rms = sqrtf(ss / (float)n + 1.0e-6f);
    float inv_rms = 1.0f / rms;

    float y[16];
    for (int i = 0; i < n; i++) y[i] = x[i] * inv_rms;

    /* LayerNorm pro comparatione */
    float yn[16];
    float m, v;
    layer_norm(x, n, 1.0e-6f, 1.0f, 0.0f, yn, &m, &v);

    DICERE("# n=%d, x ~ N(0, 1).  RMS = %.6f\n", n, (double)rms);
    DICERE("#   idx     x[i]       rms_norm(y)   layer_norm(y)\n");
    for (int i = 0; i < n; i++) {
        DICERE("#   %2d    %+7.4f      %+7.4f       %+7.4f\n",
               i, (double)x[i], (double)y[i], (double)yn[i]);
    }

    /* Statistica post */
    float ss_y = 0.0f, s_y = 0.0f;
    for (int i = 0; i < n; i++) { s_y += y[i]; ss_y += y[i] * y[i]; }
    DICERE("# RMSNorm output: mean=%+.4f, RMS=%.4f\n",
           (double)(s_y / (float)n),
           (double)sqrtf(ss_y / (float)n));
    DICERE("#\n");
}

/* ========================================================================
 * X. CAPUT ET EPILOGUS
 * ==================================================================== */

static void
caput_imprime(void)
{
    DICERE("# ═══════════════════════════════════════════════════════════\n");
    DICERE("# NORMALIZATOR v1.0.0 — Layer/Batch/RMS Norm binary32\n");
    DICERE("# ═══════════════════════════════════════════════════════════\n");
    DICERE("# Algorithmi variantiae:\n");
    DICERE("#   naivus_1pass  — sum x^2/n - (sum x/n)^2  (fallit saepe)\n");
    DICERE("#   naivus_2pass  — media primo, deinde desviationes\n");
    DICERE("#   welford       — online, O(eps), [Wel62]\n");
    DICERE("#   youngs_cramer — block recombination, [YC71]\n");
    DICERE("#\n");
}

static void
epilogus(void)
{
    DICERE("# ─── EPILOGUS ──────────────────────────────────────────────\n");
    DICERE("# Quinque principia normalizationis in binary32:\n");
    DICERE("#   1. Naivus one-pass variantiam catastrophice errat cum\n");
    DICERE("#      media magna et variantia parva [Hig02 §1.9].\n");
    DICERE("#   2. Welford online idemptoten cumulat sine cancellatione.\n");
    DICERE("#   3. eps in sqrt(var + eps) defendit contra div/0 cum\n");
    DICERE("#      data constantia. Valor typicus = 1e-5 fp32.\n");
    DICERE("#   4. RMSNorm simpliciorem formam offert (nulla subtractio\n");
    DICERE("#      mediae), adhuc efficax in LLM modernis.\n");
    DICERE("#   5. Ordo datorum effectum residuum habet — sic Welford\n");
    DICERE("#      praeferendus est in machina discentis.\n");
    DICERE("# ═══════════════════════════════════════════════════════════\n");
}

/* ========================================================================
 * XI. FUNCTIO PRINCIPALIS
 * ==================================================================== */

int
main(void)
{
    caput_imprime();
    experimentum_patrona();
    experimentum_layernorm();
    experimentum_epsilon();
    experimentum_ordo();
    experimentum_rmsnorm();
    epilogus();
    (void)stat_youngs_cramer;
    return 0;
}

/* ========================================================================
 * APPENDIX A — PROBATIO STABILITATIS WELFORD [Wel62, Cha83]
 * ====================================================================
 *
 * Algorithmus Welford:
 *   mean_{k+1} = mean_k + (x_{k+1} - mean_k) / (k+1)
 *   M2_{k+1}   = M2_k   + (x_{k+1} - mean_k) * (x_{k+1} - mean_{k+1})
 *
 * Probatio per inductionem:
 *   Sit S_k = sum_{i=1}^{k} (x_i - mean_k)^2.
 *   Affirmamus: S_{k+1} = S_k + (x_{k+1} - mean_k)(x_{k+1} - mean_{k+1}).
 *
 *   mean_{k+1} = (k mean_k + x_{k+1}) / (k+1)
 *   mean_k - mean_{k+1} = (mean_k - x_{k+1}) / (k+1)
 *
 *   S_{k+1} = sum_{i=1}^{k+1} (x_i - mean_{k+1})^2
 *           = sum_{i=1}^{k} (x_i - mean_k + mean_k - mean_{k+1})^2
 *             + (x_{k+1} - mean_{k+1})^2
 *           = S_k + 2(mean_k - mean_{k+1}) sum_{i=1}^{k}(x_i - mean_k)
 *             + k(mean_k - mean_{k+1})^2 + (x_{k+1} - mean_{k+1})^2
 *           = S_k + 0 + k ((mean_k - x_{k+1})/(k+1))^2
 *                 + (x_{k+1} - mean_{k+1})^2
 *           = S_k + (x_{k+1} - mean_k)(x_{k+1} - mean_{k+1})   QED
 *
 * Stabilitas numerica ex hoc: nulla subtractio magna-magna
 * aequalia, nulla cancellatio catastrophica.
 *
 * ─────────────────────────────────────────────────────────────────
 * APPENDIX B — CUR NAIVUS ONE-PASS FALLIT
 * ─────────────────────────────────────────────────────────────────
 *
 * Pro x_i = M + u_i cum |u_i| << |M|:
 *   E[X^2] = M^2 + 2 M E[u] + E[u^2]
 *   E[X]^2 = M^2 + 2 M E[u] + E[u]^2
 *   Var    = E[X^2] - E[X]^2 = E[u^2] - E[u]^2
 *
 * Sed in binary32:
 *   E[X^2] rotundatur ad M^2 (quia M^2 ~ 10^14, E[u^2] ~ 1/3,
 *                             additio perdit ~23 bittos minuentem).
 *   E[X]^2 rotundatur ad M^2 similiter.
 *   Var = E[X^2] - E[X]^2 = (M^2 + eps1) - (M^2 + eps2)
 *       = eps1 - eps2
 * ubi |eps1|, |eps2| ~ M^2 * FLT_EPSILON ~ 10^14 * 1e-7 ~ 10^7.
 *
 * Valor computatus fortuito in [-10^7, +10^7] — totaliter
 * irrealis. Saepe negativus, quod impossibile est.
 *
 * ─────────────────────────────────────────────────────────────────
 * APPENDIX C — BATCH NORM VS LAYER NORM
 * ─────────────────────────────────────────────────────────────────
 *
 * BatchNorm [IS15]: normalizat per dimensionem batchum.
 *   mean, var computantur per omnes exempla in batcho, per
 *   featurem. Requirit batch size >= 8, et statistica mobilis
 *   pro inferentiae tempore.
 *
 * LayerNorm [BKH16]: normalizat per dimensionem feature.
 *   mean, var computantur per omnes features in uno exemplo.
 *   Non requirit batch, ideal pro RNN et Transformer.
 *
 * RMSNorm [Zhang 2019]: subset LayerNorm sine centering.
 *   RMS(x) = sqrt(mean(x^2)).
 *   y = x * gamma / (RMS(x) + eps).
 *   Simplicior, in LLaMA/GPT-NeoX usitatus.
 *
 * GroupNorm [Wu 2018]: media inter BatchNorm et LayerNorm.
 *   Features in gruppos divisae, norma per gruppum.
 *
 * Omnes has formas adhuc algorithmum variantiae stabilem requirunt,
 * ergo Welford in quaque implementatione utilis est.
 * ═══════════════════════════════════════════════════════════════════ */
