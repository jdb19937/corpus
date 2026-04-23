/*
 * sigmoides.c — Activationes Saturatae et Derivationes
 *
 * Comparatio activationum classicarum retis neuralis sub
 * praecisione binary32, cum concentratione in saturatione
 * (gradiens evanescit), zero signato, et branchless idiomatibus.
 *
 *   sigma(x)  = 1 / (1 + exp(-x))          — Bernoulli 1844, probit
 *   tanh(x)   = (e^x - e^-x) / (e^x + e^-x) — Lambert 1770
 *   ReLU(x)   = max(0, x)                  — Fukushima 1980
 *   LReLU(x)  = max(alpha*x, x), alpha=0.01 — Maas 2013
 *   ELU(x)    = x si x>0; alpha(e^x - 1)    — Clevert 2015
 *   GELU(x)   = x * Phi(x)                  — Hendrycks 2016
 *   Swish(x)  = x * sigma(beta*x)           — Ramachandran 2017
 *   softplus  = log(1 + exp(x))             — Dugas 2001
 *
 * ═══════════════════════════════════════════════════════════════════
 * SECTIONES:
 *   I.   Implementationes stabiles vs naivae (sigma et softplus).
 *   II.  Saturatio: gradiens evanescit pro |x| magno.
 *   III. Proprietates identitatis et relationes inter activationes.
 *   IV.  Branchless ReLU: fmaxf vs conditional, bitti comparati.
 *   V.   Zero signatum in ReLU: copysignf, signbit diagnosticum.
 *   VI.  Tabula comparativa pro x ∈ {-10, ..., +10}.
 *   VII. Diagramma activationum in forma ASCII.
 *
 * REFERENTIAE:
 *   [LBO12]  LeCun Y., Bottou L., Orr G. B., Mueller K.-R. "Efficient
 *            BackProp." in Neural Networks: Tricks of the Trade
 *            (Springer 2012). §4.4 Sigmoides tanh praeferenda.
 *   [NH10]   Nair V., Hinton G. E. "Rectified Linear Units Improve
 *            Restricted Boltzmann Machines." ICML (2010).
 *   [MHN13]  Maas A. L., Hannun A. Y., Ng A. Y. "Rectifier Nonlinearities
 *            Improve Neural Network Acoustic Models." ICML WDL (2013).
 *   [CUH15]  Clevert D.-A., Unterthiner T., Hochreiter S. "Fast and
 *            Accurate Deep Network Learning by Exponential Linear
 *            Units (ELUs)." ICLR (2015).
 *   [HG16]   Hendrycks D., Gimpel K. "Gaussian Error Linear Units
 *            (GELUs)." arXiv:1606.08415 (2016).
 *   [RZL17]  Ramachandran P., Zoph B., Le Q. V. "Swish: a Self-Gated
 *            Activation Function." arXiv:1710.05941 (2017).
 *   [Dug01]  Dugas C. et al. "Incorporating Second-Order Functional
 *            Knowledge for Better Option Pricing." NeurIPS (2001).
 *            [Softplus, activatio prima cum gradiente non-zero in 0.]
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

/* ========================================================================
 * I. SIGMOIDES — NAIVUS VS STABILIS
 * ====================================================================
 * Sigma naivus: 1 / (1 + exp(-x)).
 *   Pro x = +100 : exp(-100) subefflux ad 0, sigma = 1 exacte. OK.
 *   Pro x = -100 : exp(+100) superfluit ad INF, 1/INF = 0 exacte. OK.
 *   Pro x = -90  : exp(+90) magnus sed finitus, OK.
 *   Sed gradiens: sigma'(x) = sigma(x)(1-sigma(x)). Pro x = +20,
 *     sigma ~ 1 - 2e-9; sed 1 - sigma rotundatur ad 0.0f quia
 *     1 - 2e-9 = 1.0f in binary32. Gradiens = 0.0 ex annihilatione.
 *
 * Sigma stabilis: branch per signum.
 *   x >= 0: sigma = 1 / (1 + exp(-x))
 *   x <  0: sigma = exp(x) / (1 + exp(x))
 * Haec forma cumulationem erroris minuit pro x magno. */

static float
sigma_naivus(float x)
{
    return 1.0f / (1.0f + expf(-x));
}

static float
sigma_stabilis(float x)
{
    if (x >= 0.0f) {
        float e = expf(-x);
        return 1.0f / (1.0f + e);
    }
    float e = expf(x);
    return e / (1.0f + e);
}

/* Derivatio sigma: stabilis forma evitat cancellationem. */
static float
sigma_prime(float x)
{
    float s = sigma_stabilis(x);
    return s * (1.0f - s);
}

/* Derivatio stabilis: multiplicat per exp(-|x|) directe.
 *   sigma'(x) = exp(-|x|) / (1 + exp(-|x|))^2 */
static float
sigma_prime_stabilis(float x)
{
    float e = expf(-fabsf(x));
    float d = 1.0f + e;
    return e / (d * d);
}

/* ========================================================================
 * II. TANH
 * ==================================================================== */

static float
tanh_bibl(float x)
{
    return tanhf(x);
}

/* Manualis stabilis: tanh(x) = (1 - e^-2x) / (1 + e^-2x) pro x > 0. */
static float
tanh_stabilis(float x)
{
    if (x >= 0.0f) {
        float e = expf(-2.0f * x);
        return (1.0f - e) / (1.0f + e);
    }
    float e = expf(2.0f * x);
    return (e - 1.0f) / (e + 1.0f);
}

/* tanh'(x) = 1 - tanh^2(x) */
static float
tanh_prime(float x)
{
    float t = tanhf(x);
    return 1.0f - t * t;
}

/* ========================================================================
 * III. RELU ET VARIANTES
 * ==================================================================== */

static float
relu_branchless(float x)
{
    /* fmaxf propagat NaN cum altero argumento non-NaN [C99 §7.12.12]:
     * si uno argumento NaN, resultum est alter. Hoc non nos tangit
     * quia ambiguum: alii compilatores NaN propagant. In contextu
     * retis, NaN in input saepe signum problematis gradientis est. */
    return fmaxf(0.0f, x);
}

static float
relu_branching(float x)
{
    return (x > 0.0f) ? x : 0.0f;
}

static float
lrelu(float x, float alpha)
{
    return (x > 0.0f) ? x : alpha * x;
}

static float
elu(float x, float alpha)
{
    if (x > 0.0f) return x;
    return alpha * (expf(x) - 1.0f);
}

/* ========================================================================
 * IV. GELU, SWISH, SOFTPLUS
 * ==================================================================== */

/* GELU tanh-approximatio [HG16]:
 *   GELU(x) ~ 0.5 * x * (1 + tanh(sqrt(2/pi) * (x + 0.044715 * x^3)))
 * Accuratio ~1e-4 vs erf exactum. */
static float
gelu(float x)
{
    const float SQRT_2_PI = 0.7978845608f;
    float x3 = x * x * x;
    float u = SQRT_2_PI * (x + 0.044715f * x3);
    return 0.5f * x * (1.0f + tanhf(u));
}

/* Swish / SiLU [RZL17]: x * sigma(beta*x), beta default = 1. */
static float
swish(float x, float beta)
{
    return x * sigma_stabilis(beta * x);
}

/* Softplus naivus: log(1 + exp(x)). Subefflux pro x << 0. */
static float
softplus_naivus(float x)
{
    return logf(1.0f + expf(x));
}

/* Softplus stabilis: evitat supereffluxionem pro x > 0.
 *   log(1 + exp(x)) = max(x, 0) + log(1 + exp(-|x|))
 * Vide [Gol91] de cancellatione catastrophica in log(1+y) pro y parvo. */
static float
softplus_stabilis(float x)
{
    float ax = fabsf(x);
    float m = (x > 0.0f) ? x : 0.0f;
    return m + log1pf(expf(-ax));
}

/* ========================================================================
 * V. DIAGNOSTICA ET OUTPUT
 * ==================================================================== */

static const char *
classis(int c)
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
 * VI. SECTIO I — IMPLEMENTATIO STABILIS VS NAIVA
 * ==================================================================== */

static void
sectio_naivus_stabilis(void)
{
    DICERE("# ─── I. SIGMA: NAIVUS vs STABILIS ──────────────────────────\n");
    static const float X[] = {
        -200.0f, -100.0f, -50.0f, -20.0f, -10.0f,
          -1.0f,    0.0f,   1.0f,  10.0f,  20.0f,
          50.0f, 100.0f, 200.0f
    };
    const int N = (int)(sizeof X / sizeof X[0]);
    DICERE("#     x           naivus       stabilis     sigma'(x)\n");
    for (int i = 0; i < N; i++) {
        float sn = sigma_naivus(X[i]);
        float ss = sigma_stabilis(X[i]);
        float sp = sigma_prime_stabilis(X[i]);
        DICERE("#   %+7.1f     %.6f     %.6f     %.4e\n",
               (double)X[i], (double)sn, (double)ss, (double)sp);
        SUSSURRO("# [I] x=%.3f sn=%.9e ss=%.9e sp=%.9e\n",
                 (double)X[i], (double)sn, (double)ss, (double)sp);
    }
    DICERE("# Nota: pro x = -200, sigma_naivus committit NaN aut INF\n");
    DICERE("# secundum libm (1 + INF = INF, 1/INF = 0 — OK in nostro).\n");
    DICERE("# Stabilis semper [0, 1] sine overflow intermediato.\n");
    DICERE("#\n");
}

/* ========================================================================
 * VII. SECTIO II — SATURATIO ET GRADIENS EVANESCENS
 * ==================================================================== */

static void
sectio_saturatio(void)
{
    DICERE("# ─── II. SATURATIO: GRADIENS EVANESCIT [LBO12] ─────────────\n");
    DICERE("# sigma(x), tanh(x), et earum derivationes pro |x| crescens:\n");
    DICERE("#   x       sigma(x)    sigma'(x)    tanh(x)    tanh'(x)\n");
    static const float X[] = {
        0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f,
        6.0f, 7.0f, 8.0f, 10.0f, 12.0f, 16.0f, 20.0f
    };
    const int N = (int)(sizeof X / sizeof X[0]);
    for (int i = 0; i < N; i++) {
        float s = sigma_stabilis(X[i]);
        float sp = sigma_prime_stabilis(X[i]);
        float t = tanh_stabilis(X[i]);
        float tp = tanh_prime(X[i]);
        DICERE("#  %5.1f    %.6f    %.4e    %.6f    %.4e\n",
               (double)X[i], (double)s, (double)sp, (double)t, (double)tp);
        SUSSURRO("# [II] x=%.1f s'=%.6e t'=%.6e\n",
                 (double)X[i], (double)sp, (double)tp);
    }
    DICERE("# x > 8: sigma'(x) < 1e-3; x > 20: sigma'(x) subefflux.\n");
    DICERE("# Hoc est causa famosa 'gradiens evanescit' in retibus\n");
    DICERE("# profundis ante ReLU [NH10].\n");
    DICERE("#\n");
}

/* ========================================================================
 * VIII. SECTIO III — IDENTITATES INTER ACTIVATIONES
 * ==================================================================== */

static void
sectio_identitates(void)
{
    DICERE("# ─── III. IDENTITATES ET RELATIONES ────────────────────────\n");
    DICERE("# Probationes numericae (x = 0.5):\n");
    float x = 0.5f;

    /* tanh(x) = 2*sigma(2x) - 1 */
    float t_direct = tanh_stabilis(x);
    float t_via_sigma = 2.0f * sigma_stabilis(2.0f * x) - 1.0f;
    DICERE("#   tanh(x)           = %.6f\n", (double)t_direct);
    DICERE("#   2*sigma(2x) - 1   = %.6f    [identitas: = tanh]\n",
           (double)t_via_sigma);

    /* sigma(x) = (1 + tanh(x/2)) / 2 */
    float s_direct = sigma_stabilis(x);
    float s_via_tanh = 0.5f * (1.0f + tanh_stabilis(0.5f * x));
    DICERE("#   sigma(x)          = %.6f\n", (double)s_direct);
    DICERE("#   (1+tanh(x/2))/2   = %.6f    [identitas: = sigma]\n",
           (double)s_via_tanh);

    /* Swish(x) cum beta=1 ~ GELU(x) approximatim */
    float sw = swish(x, 1.0f);
    float gl = gelu(x);
    DICERE("#   swish(x, 1)       = %.6f\n", (double)sw);
    DICERE("#   gelu(x)           = %.6f    [approximatim similes]\n",
           (double)gl);

    /* softplus'(x) = sigma(x) */
    float sp_derivatio = sigma_stabilis(x);
    DICERE("#   softplus'(x) = sigma(x) = %.6f\n",
           (double)sp_derivatio);

    /* tanh(0) = 0, sigma(0) = 0.5 exacte */
    DICERE("#   tanh(0)           = %.6f    (exactum)\n",
           (double)tanh_stabilis(0.0f));
    DICERE("#   sigma(0)          = %.6f    (exactum)\n",
           (double)sigma_stabilis(0.0f));
    DICERE("#\n");
}

/* ========================================================================
 * IX. SECTIO IV — RELU BRANCHLESS VS BRANCHING
 * ==================================================================== */

static void
sectio_relu(void)
{
    DICERE("# ─── IV. RELU: BRANCHLESS vs BRANCHING ─────────────────────\n");
    static const float X[] = {
        -2.0f, -1.0f, -0.5f, -0.0f, 0.0f, 0.5f, 1.0f, 2.0f, INFINITY, -INFINITY
    };
    const int N = (int)(sizeof X / sizeof X[0]);
    DICERE("#      x           fmaxf      ternarius    aequales?\n");
    for (int i = 0; i < N; i++) {
        float a = relu_branchless(X[i]);
        float b = relu_branching(X[i]);
        int equal = (bitti(a) == bitti(b));
        char bx[16], ba[16], bb[16];
        const char *sx, *sa, *sb;
        if (isfinite(X[i])) {
            snprintf(bx, sizeof bx, "%+7.2f", (double)X[i]);
            sx = bx;
        } else {
            sx = (X[i] > 0) ? "   +INF" : "   -INF";
        }
        if (isfinite(a)) {
            snprintf(ba, sizeof ba, "%+7.2f", (double)a);
            sa = ba;
        } else {
            sa = "   +INF";
        }
        if (isfinite(b)) {
            snprintf(bb, sizeof bb, "%+7.2f", (double)b);
            sb = bb;
        } else {
            sb = "   +INF";
        }
        DICERE("#   %s    %s    %s       %s\n",
               sx, sa, sb, equal ? "ita" : "non");
    }
    DICERE("#\n");
    DICERE("# NaN propagatio: fmaxf(NaN, 0) = 0 per [C99 §7.12.12.2]\n");
    DICERE("# (NaN tacitum est 'missing data'); fmaxf(0, NaN) = 0.\n");
    DICERE("# Ternarius (NaN > 0) = falsum, sic ternarius redit 0.\n");
    {
        float nan_v = NAN;
        float a = relu_branchless(nan_v);
        float b = relu_branching(nan_v);
        DICERE("# relu_branchless(NaN) = %.4f (bitti = 0x%08" PRIx32 ")\n",
               (double)a, bitti(a));
        DICERE("# relu_branching(NaN)  = %.4f (bitti = 0x%08" PRIx32 ")\n",
               (double)b, bitti(b));
    }
    DICERE("#\n");
}

/* ========================================================================
 * X. SECTIO V — ZERO SIGNATUM
 * ==================================================================== */

static void
sectio_zero(void)
{
    DICERE("# ─── V. ZERO SIGNATUM IN RELU ──────────────────────────────\n");
    float pz = +0.0f;
    float nz = -0.0f;
    DICERE("# +0.0f  bitti = 0x%08" PRIx32 "\n", bitti(pz));
    DICERE("# -0.0f  bitti = 0x%08" PRIx32 "\n", bitti(nz));
    DICERE("# signbit(+0) = %d, signbit(-0) = %d\n",
           signbit(pz), signbit(nz));
    DICERE("# +0 == -0 (comparatio)? %s\n",
           (pz == nz) ? "ita" : "non");

    /* fmaxf(0, -0) semantica */
    float r1 = fmaxf(pz, nz);
    float r2 = fmaxf(nz, pz);
    DICERE("# fmaxf(+0, -0) bitti = 0x%08" PRIx32 "\n", bitti(r1));
    DICERE("# fmaxf(-0, +0) bitti = 0x%08" PRIx32 "\n", bitti(r2));
    DICERE("# Nota: IEEE 754 non determinat quid fmaxf pro 0 signata\n");
    DICERE("# reddat; implementationes variae, attende.\n");

    /* copysignf: transferit signum */
    float one = 1.0f;
    float neg = copysignf(one, -2.0f);
    DICERE("# copysignf(1, -2) = %.4f (bitti = 0x%08" PRIx32 ")\n",
           (double)neg, bitti(neg));
    DICERE("#\n");
}

/* ========================================================================
 * XI. SECTIO VI — TABULA COMPARATIVA
 * ==================================================================== */

static void
sectio_tabula(void)
{
    DICERE("# ─── VI. TABULA COMPARATIVA ACTIVATIONUM ───────────────────\n");
    DICERE("#   x      sigma   tanh    ReLU   LReLU   ELU    GELU   Swish  softplus\n");
    for (int i = -10; i <= 10; i++) {
        float x = (float)i;
        float s  = sigma_stabilis(x);
        float t  = tanh_stabilis(x);
        float r  = relu_branchless(x);
        float lr = lrelu(x, 0.01f);
        float el = elu(x, 1.0f);
        float g  = gelu(x);
        float sw = swish(x, 1.0f);
        float sp = softplus_stabilis(x);
        DICERE("#  %+3.0f    %.4f  %+.4f  %5.1f  %+6.3f  %+6.3f  %+6.3f  %+6.3f  %7.3f\n",
               (double)x, (double)s, (double)t, (double)r,
               (double)lr, (double)el, (double)g, (double)sw, (double)sp);
    }
    DICERE("#\n");
}

/* ========================================================================
 * XII. SECTIO VII — DIAGRAMMA ASCII
 * ==================================================================== */

static void
traceat_funum(const char *nomen, float (*f)(float), float x_min,
              float x_max, float y_min, float y_max)
{
    const int W = 60, H = 15;
    DICERE("# %s, x ∈ [%.1f, %.1f], y ∈ [%.1f, %.1f]\n",
           nomen, (double)x_min, (double)x_max,
           (double)y_min, (double)y_max);
    /* Malla character */
    char grid[16][64];
    for (int r = 0; r < H; r++) {
        for (int c = 0; c < W; c++) grid[r][c] = ' ';
        grid[r][W] = '\0';
    }
    /* Axis horizontalis y=0 */
    int r0 = (int)((double)(y_max / (y_max - y_min)) * (H - 1));
    if (r0 >= 0 && r0 < H) {
        for (int c = 0; c < W; c++) grid[r0][c] = '-';
    }
    /* Axis verticalis x=0 */
    int c0 = (int)((double)(-x_min / (x_max - x_min)) * (W - 1));
    if (c0 >= 0 && c0 < W) {
        for (int r = 0; r < H; r++)
            grid[r][c0] = (grid[r][c0] == '-') ? '+' : '|';
    }
    for (int c = 0; c < W; c++) {
        float x = x_min + (x_max - x_min) * (float)c / (float)(W - 1);
        float y = f(x);
        if (!isfinite(y)) continue;
        if (y < y_min || y > y_max) continue;
        int r = (int)((double)((y_max - y) / (y_max - y_min)) * (H - 1));
        if (r < 0 || r >= H) continue;
        grid[r][c] = '*';
    }
    for (int r = 0; r < H; r++) {
        DICERE("#   %s\n", grid[r]);
    }
}

static float gelu_wrap(float x) { return gelu(x); }
static float swish_wrap(float x) { return swish(x, 1.0f); }
static float softplus_wrap(float x) { return softplus_stabilis(x); }
static float sigma_wrap(float x) { return sigma_stabilis(x); }
static float tanh_wrap(float x) { return tanh_stabilis(x); }
static float relu_wrap(float x) { return relu_branchless(x); }

static void
sectio_diagramma(void)
{
    DICERE("# ─── VII. DIAGRAMMATA ACTIVATIONUM ─────────────────────────\n");
    traceat_funum("sigma(x)",    sigma_wrap,    -6.0f, 6.0f, -0.1f, 1.1f);
    DICERE("#\n");
    traceat_funum("tanh(x)",     tanh_wrap,     -3.0f, 3.0f, -1.1f, 1.1f);
    DICERE("#\n");
    traceat_funum("ReLU(x)",     relu_wrap,     -3.0f, 3.0f, -0.5f, 3.0f);
    DICERE("#\n");
    traceat_funum("GELU(x)",     gelu_wrap,     -3.0f, 3.0f, -0.5f, 3.0f);
    DICERE("#\n");
    traceat_funum("Swish(x,1)",  swish_wrap,    -3.0f, 3.0f, -0.5f, 3.0f);
    DICERE("#\n");
    traceat_funum("softplus(x)", softplus_wrap, -3.0f, 3.0f, 0.0f,  3.5f);
    DICERE("#\n");
}

/* ========================================================================
 * XIII. CAPUT ET EPILOGUS
 * ==================================================================== */

static void
caput_imprime(void)
{
    DICERE("# ═══════════════════════════════════════════════════════════\n");
    DICERE("# SIGMOIDES v1.0.0 — Activationes Saturatae binary32\n");
    DICERE("# ═══════════════════════════════════════════════════════════\n");
    DICERE("# Activationes comparatae: sigma, tanh, ReLU, LReLU, ELU,\n");
    DICERE("#                          GELU, Swish, softplus.\n");
    DICERE("#\n");
}

static void
epilogus(void)
{
    DICERE("# ─── EPILOGUS ──────────────────────────────────────────────\n");
    DICERE("# Septem principia activationum in binary32:\n");
    DICERE("#   1. Sigma stabilis: branch per signum evitat overflow.\n");
    DICERE("#   2. Saturatio: sigma'(x), tanh'(x) -> 0 pro |x| > 8.\n");
    DICERE("#   3. ReLU gradiens constans = 1 pro x > 0 ; 0 pro x <= 0.\n");
    DICERE("#   4. NaN propagatio per fmaxf: altero non-NaN redit.\n");
    DICERE("#   5. Softplus stabilis: m + log1p(exp(-|x|)).\n");
    DICERE("#   6. Identitates: tanh(x) = 2*sigma(2x) - 1.\n");
    DICERE("#   7. GELU et Swish — activationes 'smoothae' modernae.\n");
    DICERE("# ═══════════════════════════════════════════════════════════\n");
}

/* ========================================================================
 * XIV. FUNCTIO PRINCIPALIS
 * ==================================================================== */

int
main(void)
{
    caput_imprime();
    sectio_naivus_stabilis();
    sectio_saturatio();
    sectio_identitates();
    sectio_relu();
    sectio_zero();
    sectio_tabula();
    sectio_diagramma();
    epilogus();
    /* Functiones alternae documentationi servatae; cogit
     * compilatorem cessare de warning. */
    (void)tanh_bibl;
    (void)sigma_prime;
    (void)softplus_naivus;
    (void)classis;
    return 0;
}

/* ========================================================================
 * APPENDIX A — STABILITAS SIGMA: PROBATIO
 * ====================================================================
 *
 * Naivus: sigma(x) = 1 / (1 + exp(-x)).
 *
 * Pro x = +100:
 *   exp(-100) ~ 3.7e-44 (subnormalis aut zero).
 *   1 + exp(-100) rotundatur ad 1.0f.
 *   sigma = 1.0f exacte. OK.
 *
 * Pro x = -100:
 *   exp(+100) = +INF (supereffluxio).
 *   1 + INF = INF.
 *   1 / INF = 0.0f exacte.
 *   OK per [IEEE754 §7.12.11].
 *
 * Pro x = -90:
 *   exp(+90) ~ 1.2e39 > FLT_MAX (~3.4e38). INF!
 *   Idem.
 *
 * Pro x = +20:
 *   exp(-20) ~ 2.06e-9.
 *   1 + 2.06e-9 = 1.0f post rotundationem (quia 2e-9 < FLT_EPSILON).
 *   sigma = 1.0f exacte, non 1.0 - 2.06e-9.
 *   Derivatio sigma(1-sigma) = 0 — gradiens perditus!
 *
 * Stabilis idem valorem fundamentalem computat sed per formulam
 * magnitude-sensitivam:
 *   x >= 0: e = exp(-x), 1/(1+e)   — numerator stabilis
 *   x <  0: e = exp(x),  e/(1+e)   — numerator et denominator
 *                                     magnitudinis simillimae
 * Haec forma gradientem ~e/(1+e)^2 stabilius producit.
 *
 * ─────────────────────────────────────────────────────────────────
 * APPENDIX B — SOFTPLUS STABILIS
 * ─────────────────────────────────────────────────────────────────
 *
 * Naivus: softplus(x) = log(1 + exp(x)).
 *
 * Pro x = +100:
 *   exp(+100) = +INF; log(INF) = +INF. Sed analytice softplus(100) ~ 100.
 *   Naivus falsum INF dat.
 *
 * Pro x = -100:
 *   exp(-100) ~ subnormal aut 0; log(1 + 0) = 0. Sed analytice
 *   softplus(-100) ~ e^-100 ~ 3.7e-44. Naivus 0 dat, magnitudinem
 *   perdit (sed 0 est bonus proximum tamen).
 *
 * Identitas stabilis:
 *   log(1 + exp(x)) = max(0, x) + log(1 + exp(-|x|))
 *
 * Pro x magno positivo: max(0, x) = x, log(1 + exp(-x)) ~ exp(-x) ~ 0.
 *   Summa = x + O(exp(-x)). Exactum analyticum.
 *
 * Pro x magno negativo: max(0, x) = 0, log(1 + exp(-|x|)) = log(1 + exp(x))
 *   quod bene definitur quia exp(x) -> 0 sine overflow.
 *
 * Loco logf(1 + e), utimur log1pf(e) pro praecisione additionali
 * cum e << 1. [C99 §7.12.6.7]
 *
 * ─────────────────────────────────────────────────────────────────
 * APPENDIX C — GELU EXACTUS VS APPROXIMATUS
 * ─────────────────────────────────────────────────────────────────
 *
 * Exactus: GELU(x) = x * 0.5 * (1 + erf(x / sqrt(2)))
 *   ubi erf est functio erroris Gaussiana.
 * Approximatus tanh-based (hoc codice):
 *   GELU(x) ~ 0.5 * x * (1 + tanh(sqrt(2/pi) * (x + 0.044715 x^3)))
 *
 * Error max ~ 1e-4 in region ~ ±4. Ponit constantem magicam
 * 0.044715 quae minimum erroris fit.
 *
 * Exactus requirit erff (C99 §7.12.8.1). Approximatus compatibilis
 * cum hardware sine erf instruction. Transformer BERT originem
 * usum approximatum; GPT-3 usum exactum.
 *
 * ═══════════════════════════════════════════════════════════════════ */
