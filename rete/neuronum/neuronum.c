/*
 * neuronum.c — Neuronum Artificiale Rosenblattianum
 *
 * Demonstratio comprehensiva perceptronis binarii (Rosenblatt 1958)
 * cum regula Hebbiana (Hebb 1949), concentrans in angulis IEEE 754
 * binary32 simplicis praecisionis qui in doctrina synapticarum
 * apparent:
 *
 *   I. DATA DOCENDI      — viginti puncta bidimensionalia, duae classes,
 *                          disiunctio linearis cum hiatu noto.
 *   II. PERCEPTRON       — correctio discreta per regulam Rosenblatt:
 *                          w <- w + eta * (t - y) * x    si t != y
 *                          b <- b + eta * (t - y)
 *   III. REGULA HEBBIANA — "cellulae quae simul accenduntur simul
 *                          conectuntur": w_i <- w_i + eta * y_i * x_i
 *                          sine discriminatore tutoris.
 *   IV. LIMITARE ULP     — quomodo eta * x * (t - y) infra ULP
 *                          ponderis fluit et correctio perditur
 *                          propter rotundationem. Inquisitio per
 *                          nextafterf et FLT_EPSILON.
 *   V. SUBNORMALES       — cum eta tanta ut pondera ad regionem
 *                          denormalizatam (minus quam FLT_MIN)
 *                          descendant, praecisio relativa usque ad
 *                          annihilationem degradatur.
 *   VI. GEOMETRIA MARGINIS — distantia a hyperplano decisionis,
 *                          numeri fpclassify distributi.
 *
 * ═══════════════════════════════════════════════════════════════════
 * REFERENTIAE (referentiae per nomen et annum sub-citantur infra):
 *
 *   [Ros58]  Rosenblatt F. "The Perceptron: A Probabilistic Model
 *            for Information Storage and Organization in the Brain."
 *            Psychological Review 65(6): 386-408 (1958).
 *   [Heb49]  Hebb D. O. "The Organization of Behavior: A
 *            Neuropsychological Theory." Wiley (1949).
 *   [MP69]   Minsky M., Papert S. "Perceptrons." MIT Press (1969).
 *   [Nov62]  Novikoff A. B. J. "On Convergence Proofs on
 *            Perceptrons." Symposium on the Mathematical Theory of
 *            Automata, XII: 615-622 (1962).
 *   [IEEE754] IEEE Std 754-2019, "IEEE Standard for Floating-Point
 *            Arithmetic." IEEE (2019).
 *   [Gol91]  Goldberg D. "What Every Computer Scientist Should Know
 *            About Floating-Point Arithmetic." ACM Computing
 *            Surveys 23(1): 5-48 (1991).
 *   [Hig02]  Higham N. J. "Accuracy and Stability of Numerical
 *            Algorithms." 2nd ed., SIAM (2002).
 *   [Mul10]  Muller J.-M. et al. "Handbook of Floating-Point
 *            Arithmetic." Birkhauser (2010).
 *
 * Nullae optiones CLI, nullum stdin. Egressus deterministicus:
 *   — stdout: praecisio trunciata ad quattuor cifras decimales;
 *   — stderr: diagnostica plenae praecisionis pro observatore curioso.
 * ═══════════════════════════════════════════════════════════════════
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>

/* ========================================================================
 * I. CONSTANTES ET TYPI FUNDAMENTALES
 * ==================================================================== */

#define DIM                2         /* dimensio spatii characteristici  */
#define N_DATA            20         /* numerus punctorum docendi        */
#define MAX_EPOCHAE      200         /* limes epocharum                  */
#define VERSIO_MAIOR       1
#define VERSIO_MINOR       0
#define VERSIO_MINIMA      0

/*
 * Macra diagnostica. DICERE scribit in stdout (deterministice),
 * SUSSURRO in stderr (observatio plenae praecisionis, permissum
 * non-deterministicum).
 */
#define DICERE(...)    fprintf(stdout, __VA_ARGS__)
#define SUSSURRO(...)  fprintf(stderr, __VA_ARGS__)

/* Truncat float ad quattuor cifras decimales ad egressum deterministicum.
 * Rotundatio "ad proximum, pares ad par" per roundf scalatum. */
static float
tronca4(float x)
{
    if (!isfinite(x)) return x;
    return roundf(x * 10000.0f) / 10000.0f;
}

/* Repraesentatio hexadecimalis exacta bittorum (binary32) pro
 * inspectione ULP-scalae sine ambiguitate printf-formae. */
static uint32_t
bitti(float x)
{
    uint32_t u;
    memcpy(&u, &x, sizeof u);
    return u;
}

/* Classificatio IEEE 754: NAMES pro fpclassify macro. */
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
 * II. DATA DOCENDI — PUNCTA FIXA, DUAE CLASSES
 * ====================================================================
 * Viginti puncta in quadrato [-3, +3] x [-3, +3], classis +1 supra
 * lineam y = x/2 + 0.25, classis -1 infra, cum hiatu sufficiente
 * ut perceptron in paucis epochis convergat ([Nov62] theorema). */

static const float PUNCTA_X[N_DATA][DIM] = {
    { -2.5000f,  0.5000f }, { -2.0000f,  1.5000f },
    { -1.5000f,  2.0000f }, { -1.0000f,  1.8750f },
    { -0.5000f,  2.2500f }, {  0.0000f,  1.5000f },
    {  0.5000f,  2.0000f }, {  1.0000f,  2.5000f },
    {  1.5000f,  2.7500f }, {  2.0000f,  2.8750f },
    { -2.5000f, -2.0000f }, { -2.0000f, -1.5000f },
    { -1.5000f, -0.5000f }, { -1.0000f, -0.7500f },
    { -0.5000f, -0.3750f }, {  0.0000f, -1.0000f },
    {  0.5000f, -0.6250f }, {  1.0000f, -0.2500f },
    {  1.5000f,  0.0000f }, {  2.0000f,  0.3750f }
};

static const int8_t CLASSIS_Y[N_DATA] = {
    +1, +1, +1, +1, +1, +1, +1, +1, +1, +1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

/* ========================================================================
 * III. STATUS PERCEPTRONIS
 * ==================================================================== */

typedef struct {
    float    pondera[DIM];
    float    bias;
    int      erratae;          /* numerus errorum epochae ultimae  */
    int      epochae_actae;
} Perceptron;

static void
perceptron_init(Perceptron *p, float pondus_initialis, float bias_init)
{
    for (int i = 0; i < DIM; i++) p->pondera[i] = pondus_initialis;
    p->bias = bias_init;
    p->erratae = 0;
    p->epochae_actae = 0;
}

static float
activatio(const Perceptron *p, const float x[DIM])
{
    /* Productum scalare cum fmaf ut cumulatio sit stabilior
     * ([Hig02] cap. 3): fma(a, b, c) = a*b + c cum UNA rotundatione. */
    float z = p->bias;
    for (int i = 0; i < DIM; i++) z = fmaf(p->pondera[i], x[i], z);
    return z;
}

static int8_t
signum_perceptronis(float z)
{
    /* Convenit ut signum(0) = +1 ad evitandum ambiguitatem decisionis. */
    return (z >= 0.0f) ? +1 : -1;
}

/* ========================================================================
 * IV. DOCTRINA PERCEPTRONIS — ALGORITHMUS ROSENBLATT
 * ====================================================================
 * Epocha per omnia puncta iterat. Si prognostico discrepet, pondera
 * emendantur per eta * (t - y) * x. Theorema Novikoff garantizat
 * convergentiam finitam si data linearitate separabilia. */

static void
perceptron_doce(Perceptron *p, float eta, int max_epochae, bool verbosus)
{
    (void)verbosus;
    for (int epocha = 0; epocha < max_epochae; epocha++) {
        int errores = 0;
        for (int i = 0; i < N_DATA; i++) {
            float z = activatio(p, PUNCTA_X[i]);
            int8_t y = signum_perceptronis(z);
            int8_t t = CLASSIS_Y[i];
            if (y != t) {
                errores++;
                float delta = eta * (float)(t - y);  /* +2*eta aut -2*eta */
                for (int k = 0; k < DIM; k++) {
                    /* w_k <- w_k + delta * x_ik, emendatio stabilis */
                    p->pondera[k] = fmaf(delta, PUNCTA_X[i][k],
                                         p->pondera[k]);
                }
                p->bias += delta;
            }
        }
        p->erratae = errores;
        p->epochae_actae = epocha + 1;
        if (errores == 0) break;
    }
}

/* Regula Hebbiana pura: w_i += eta * y_i * x_i, sine comparatione
 * prognostici. Ducit ad solutionem "centroidum", non hyperplanum
 * optimum — utile pro monstranda differentia. */
static void
hebbianum_doce(Perceptron *p, float eta, int iterationes)
{
    for (int it = 0; it < iterationes; it++) {
        for (int i = 0; i < N_DATA; i++) {
            float y = (float)CLASSIS_Y[i];
            for (int k = 0; k < DIM; k++) {
                p->pondera[k] = fmaf(eta * y, PUNCTA_X[i][k],
                                     p->pondera[k]);
            }
            p->bias += eta * y;
        }
        p->epochae_actae = it + 1;
    }
}

static int
numera_errores(const Perceptron *p)
{
    int err = 0;
    for (int i = 0; i < N_DATA; i++) {
        float z = activatio(p, PUNCTA_X[i]);
        int8_t y = signum_perceptronis(z);
        if (y != CLASSIS_Y[i]) err++;
    }
    return err;
}

/* ========================================================================
 * V. SECTIO EGRESSUS — CAPUT PROLOGUM
 * ==================================================================== */

static void
caput_imprime(void)
{
    DICERE("# ═══════════════════════════════════════════════════════════\n");
    DICERE("# NEURONUM v%d.%d.%d — Perceptron binarius binary32\n",
           VERSIO_MAIOR, VERSIO_MINOR, VERSIO_MINIMA);
    DICERE("# ═══════════════════════════════════════════════════════════\n");
    DICERE("# Constantes IEEE 754 binary32 [IEEE754]:\n");
    DICERE("#   FLT_EPSILON     = %.4e   (distantia a 1.0 ad proximum)\n",
           (double)FLT_EPSILON);
    DICERE("#   FLT_MIN         = %.4e   (minimum normalizatum positivum)\n",
           (double)FLT_MIN);
    DICERE("#   FLT_MAX         = %.4e\n", (double)FLT_MAX);
    DICERE("#   FLT_MANT_DIG    = %d     (bit significantiae implicito)\n",
           FLT_MANT_DIG);
    DICERE("#   FLT_MIN_EXP     = %d\n", FLT_MIN_EXP);
    DICERE("#   FLT_MAX_EXP     = %d\n", FLT_MAX_EXP);
    /* Subnormale minimum positivum: 2^(-149). Non est macro C99
     * (tantum C11 habet FLT_TRUE_MIN) ideo per ldexpf computamus. */
    DICERE("#   min subnormale  = %.4e   (2^-149, [Mul10])\n",
           (double)ldexpf(1.0f, -149));
    DICERE("#\n");
    DICERE("# Data docendi: %d puncta, %d dimensiones, 2 classes.\n",
           N_DATA, DIM);
    DICERE("#\n");

    SUSSURRO("# === DIAGNOSTICA STDERR: plena praecisio permissa ===\n");
    SUSSURRO("# FLT_EPSILON plena   = %.9e\n", FLT_EPSILON);
    SUSSURRO("# FLT_MIN plena       = %.9e\n", FLT_MIN);
    SUSSURRO("# min subnormale plena = %.9e\n", ldexpf(1.0f, -149));
    SUSSURRO("# bitti(1.0f)         = 0x%08" PRIx32 "\n", bitti(1.0f));
    SUSSURRO("# bitti(FLT_EPSILON)  = 0x%08" PRIx32 "\n", bitti(FLT_EPSILON));
    SUSSURRO("# bitti(FLT_MIN)      = 0x%08" PRIx32 "\n", bitti(FLT_MIN));
    SUSSURRO("# bitti(+0.0f)        = 0x%08" PRIx32 "\n", bitti(+0.0f));
    SUSSURRO("# bitti(-0.0f)        = 0x%08" PRIx32 "\n", bitti(-0.0f));
    SUSSURRO("\n");
}

/* ========================================================================
 * VI. SECTIO I — INSPECTIO DATORUM
 * ==================================================================== */

static void
sectio_data(void)
{
    DICERE("# ─── I. DATA DOCENDI ───────────────────────────────────────\n");
    DICERE("#  idx    x[0]     x[1]    classis\n");
    for (int i = 0; i < N_DATA; i++) {
        DICERE("#  %3d  %7.4f  %7.4f     %+d\n", i,
               tronca4(PUNCTA_X[i][0]), tronca4(PUNCTA_X[i][1]),
               CLASSIS_Y[i]);
    }
    /* Separatio nominalis: line y = x/2 + 0.25 */
    DICERE("#\n# Hyperplanum separationis nominale: y = x/2 + 1/4.\n");
    DICERE("#\n");
}

/* ========================================================================
 * VII. SECTIO II — DOCTRINA PERCEPTRONIS STANDARDIS
 * ==================================================================== */

static void
sectio_perceptron(void)
{
    DICERE("# ─── II. PERCEPTRON ROSENBLATTIANUM [Ros58] ────────────────\n");
    Perceptron p;
    perceptron_init(&p, 0.0f, 0.0f);

    const float eta = 0.125f;   /* rata discendi — potestas bina exacta */
    perceptron_doce(&p, eta, MAX_EPOCHAE, false);

    int err = numera_errores(&p);
    DICERE("# eta = %.4f, epochae actae = %d, errores finales = %d / %d\n",
           tronca4(eta), p.epochae_actae, err, N_DATA);
    DICERE("# Pondera finalia:\n");
    DICERE("#   w[0] = %+8.4f    (class=%s)\n",
           tronca4(p.pondera[0]),
           classis_nomen(fpclassify(p.pondera[0])));
    DICERE("#   w[1] = %+8.4f    (class=%s)\n",
           tronca4(p.pondera[1]),
           classis_nomen(fpclassify(p.pondera[1])));
    DICERE("#   b    = %+8.4f    (class=%s)\n",
           tronca4(p.bias),
           classis_nomen(fpclassify(p.bias)));

    SUSSURRO("# [II] w plena: %.9e %.9e  b=%.9e\n",
             p.pondera[0], p.pondera[1], p.bias);
    SUSSURRO("# [II] bitti: 0x%08" PRIx32 " 0x%08" PRIx32 " 0x%08" PRIx32 "\n",
             bitti(p.pondera[0]), bitti(p.pondera[1]), bitti(p.bias));
    DICERE("#\n");
}

/* ========================================================================
 * VIII. SECTIO III — REGULA HEBBIANA [Heb49]
 * ==================================================================== */

static void
sectio_hebb(void)
{
    DICERE("# ─── III. REGULA HEBBIANA [Heb49] ──────────────────────────\n");
    Perceptron p;
    perceptron_init(&p, 0.0f, 0.0f);
    const float eta = 0.0625f;   /* 2^-4 */
    hebbianum_doce(&p, eta, 10);
    int err = numera_errores(&p);
    DICERE("# eta = %.4f, iterationes = %d, errores = %d / %d\n",
           tronca4(eta), p.epochae_actae, err, N_DATA);
    DICERE("# Pondera Hebbiana (approxim. ad centroidum discriminatoris):\n");
    DICERE("#   w[0] = %+8.4f\n", tronca4(p.pondera[0]));
    DICERE("#   w[1] = %+8.4f\n", tronca4(p.pondera[1]));
    DICERE("#   b    = %+8.4f\n", tronca4(p.bias));
    DICERE("# Nota: Hebb sine discrimine tutoris solutionem optimam non\n");
    DICERE("# producit; pondus crescit sine termine (stabilisatio per\n");
    DICERE("# regulam Oja 1982 requiritur).\n");
    DICERE("#\n");
}

/* ========================================================================
 * IX. SECTIO IV — LIMITARE ULP ET EPSILON
 * ====================================================================
 * Si eta * |x| * |t - y| < ULP(|w|) / 2 (in norma L∞), emendatio
 * rotundatur ad zero. [Gol91 §2] theorema ULP: rotundatio ad proximum
 * committit errorem maximum 1/2 ULP.
 *
 * Monstramus per reductionem eta: quando pondus stabilisatur.
 */

static void
sectio_ulp(void)
{
    DICERE("# ─── IV. LIMITARE ULP [Gol91] ──────────────────────────────\n");
    DICERE("# Si eta * ||x||_inf < ULP(|w|)/2, emendatio perditur.\n");
    DICERE("# Exploratio eta per potestates binas 2^-k:\n");
    DICERE("#   k    eta             ||w|| finalis   errores   status\n");

    /* Puncta normalizata ut ||x||_inf <= ~3 */
    for (int k = 0; k <= 30; k += 3) {
        float eta = ldexpf(1.0f, -k);
        Perceptron p;
        perceptron_init(&p, 1.0f, 0.0f);   /* initium non-zero */
        perceptron_doce(&p, eta, 50, false);
        float norma = fmaxf(fabsf(p.pondera[0]), fabsf(p.pondera[1]));
        int err = numera_errores(&p);
        const char *status;
        if (err == 0) status = "convergit";
        else if (norma == 1.0f) status = "stagnans";
        else status = "currens";
        /* eta est potestas bina exacta, sic tuto %.4e exprimere. */
        DICERE("#  %3d   %.4e   %10.4f   %5d     %s\n",
               k, (double)eta, (double)tronca4(norma), err, status);
        SUSSURRO("# [IV] k=%d eta_plena=%.9e  ||w||_plena=%.9e\n",
                 k, (double)eta, (double)norma);
    }
    DICERE("#\n");

    /* Etiam: minimum eta quod ad pondus=1.0 aliquem effectum producit.
     * ULP(1.0) = FLT_EPSILON = 2^-23; ergo eta * x minor quam 2^-24
     * (in rotundatione ad proximum) perditur. */
    DICERE("# ULP(1.0) = 2^-23 = FLT_EPSILON = %.4e\n", (double)FLT_EPSILON);
    DICERE("# Infra eta ~ 2^-24, correctiones minimae stabiliuntur.\n");
    DICERE("#\n");

    /* nextafterf demonstratio: gradus unius ULP a 1.0 */
    float unum = 1.0f;
    float proximus = nextafterf(unum, INFINITY);
    float distantia = proximus - unum;
    DICERE("# nextafterf(1.0, +INF) = 1.0 + ULP = %.8f\n",
           (double)proximus);
    DICERE("# distantia bitti unius = %.4e\n", (double)distantia);
    SUSSURRO("# [IV] nextafterf plena = %.9e, bitti = 0x%08" PRIx32 "\n",
             proximus, bitti(proximus));
    DICERE("#\n");
}

/* ========================================================================
 * X. SECTIO V — TRAIECTUS IN REGIONEM SUBNORMALEM
 * ====================================================================
 * Pondus initiale FLT_MIN (minimum normalizatum). Eta negativus
 * contrahens pondus usque ad regionem denormalizatam. fpclassify
 * progressum NORMAL -> SUBNORMAL -> ZERO revelat.
 */

static void
sectio_subnormales(void)
{
    DICERE("# ─── V. REGIO SUBNORMALIS ──────────────────────────────────\n");
    DICERE("# Pondus w <- w * (1 - 2^-8) per iterationes; initium w=FLT_MIN.\n");
    DICERE("#   iter    w                    status   bitti\n");

    float w = FLT_MIN;
    const float factor = 1.0f - ldexpf(1.0f, -8);  /* 0.99609375 */
    int passi = 0;
    int cl_prec = -1;
    int intra_sub = 0;
    for (int i = 0; i < 100000; i++) {
        int cl = fpclassify(w);
        int transitio = (cl != cl_prec);
        int landmark = (i == 0 || i == 10 || i == 100
                        || i == 1000 || i == 10000);
        /* Primi VI passus intra regionem subnormalem servati. */
        int in_sub_caput = (cl == FP_SUBNORMAL && intra_sub < 6);
        if (transitio || landmark || in_sub_caput) {
            DICERE("#  %6d   %.4e          %s     0x%08" PRIx32 "\n",
                   i, (double)w, classis_nomen(cl), bitti(w));
            SUSSURRO("# [V] i=%d w=%.9e cl=%d\n", i, (double)w, cl);
        }
        if (cl == FP_SUBNORMAL) intra_sub++;
        cl_prec = cl;
        if (cl == FP_ZERO) { passi = i; break; }
        w = w * factor;
        passi = i + 1;
    }
    DICERE("# Annihilatio completa post %d iterationes.\n", passi);
    DICERE("# Notandum: in regione subnormali praecisio relativa\n");
    DICERE("# degradatur — bitti significantiae paulatim perduntur.\n");
    DICERE("#\n");
}

/* ========================================================================
 * XI. SECTIO VI — GEOMETRIA MARGINIS
 * ====================================================================
 * Post convergentiam, distantia signata a hyperplano decisionis
 * (Euclidea): d_i = (w . x_i + b) / ||w||. Quanto maior, tanto
 * robustior decisio contra perturbationem [MP69 discussio]. */

static void
sectio_margin(void)
{
    DICERE("# ─── VI. MARGO GEOMETRICUS ─────────────────────────────────\n");
    Perceptron p;
    perceptron_init(&p, 0.0f, 0.0f);
    perceptron_doce(&p, 0.125f, MAX_EPOCHAE, false);

    float normaq = fmaf(p.pondera[0], p.pondera[0],
                        p.pondera[1] * p.pondera[1]);
    float norma = sqrtf(normaq);
    DICERE("# ||w||_2 = %.4f\n", tronca4(norma));
    DICERE("#  idx   signatum z   distantia    classis   fpclass(d)\n");

    float margo_min = INFINITY;
    for (int i = 0; i < N_DATA; i++) {
        float z = activatio(&p, PUNCTA_X[i]);
        float d = (norma > 0.0f) ? (z / norma) : 0.0f;
        float dm = (float)CLASSIS_Y[i] * d;
        if (dm < margo_min) margo_min = dm;
        DICERE("#  %3d   %+8.4f    %+8.4f      %+d       %s\n",
               i, tronca4(z), tronca4(d), CLASSIS_Y[i],
               classis_nomen(fpclassify(d)));
        SUSSURRO("# [VI] i=%d z=%.9e d=%.9e\n", i, z, d);
    }
    DICERE("# Margo minimus (signatus) = %.4f\n", tronca4(margo_min));
    DICERE("# Perceptron tantum solutionem consistentem invenit, non\n");
    DICERE("# necessario illam cum margine maximo (SVM contra).\n");
    DICERE("#\n");
}

/* ========================================================================
 * XII. SECTIO VII — SUMMATIO DISTRIBUTIONIS FPCLASSIFY
 * ==================================================================== */

static void
sectio_classes(void)
{
    DICERE("# ─── VII. DISTRIBUTIO FPCLASSIFY PONDERUM ──────────────────\n");
    /* Decem perceptrones cum variis etis, collige classes ponderum. */
    int totalia[5] = { 0 };  /* NOR, SUB, NUL, INF, NAN */
    for (int k = 0; k < 10; k++) {
        float eta = ldexpf(1.0f, -k);
        Perceptron p;
        perceptron_init(&p, ldexpf(1.0f, -3 * k), 0.0f);
        perceptron_doce(&p, eta, 30, false);
        int vals[3] = {
            fpclassify(p.pondera[0]),
            fpclassify(p.pondera[1]),
            fpclassify(p.bias)
        };
        for (int j = 0; j < 3; j++) {
            switch (vals[j]) {
                case FP_NORMAL:    totalia[0]++; break;
                case FP_SUBNORMAL: totalia[1]++; break;
                case FP_ZERO:      totalia[2]++; break;
                case FP_INFINITE:  totalia[3]++; break;
                case FP_NAN:       totalia[4]++; break;
            }
        }
    }
    DICERE("# Ex 30 ponderibus colligitis (10 experimenta * 3 pondera):\n");
    DICERE("#   NORMALES   = %d\n", totalia[0]);
    DICERE("#   SUBNORMALES = %d\n", totalia[1]);
    DICERE("#   ZERO       = %d\n", totalia[2]);
    DICERE("#   INFINITI   = %d\n", totalia[3]);
    DICERE("#   NAN        = %d\n", totalia[4]);
    DICERE("#\n");
}

/* ========================================================================
 * XIII. EPILOGUS
 * ==================================================================== */

static void
epilogus_imprime(void)
{
    DICERE("# ─── EPILOGUS ──────────────────────────────────────────────\n");
    DICERE("# Quattuor lectiones praecisionis simplicis in rete synaptico:\n");
    DICERE("#   1. ULP ponit liminare infimum rata discendi efficacis.\n");
    DICERE("#   2. Subnormales praecisionem relativam degradant silenter.\n");
    DICERE("#   3. fmaf cumulationem ponderum stabiliorem reddit.\n");
    DICERE("#   4. Perceptron convergit modo data separabilia [Nov62].\n");
    DICERE("# ═══════════════════════════════════════════════════════════\n");
}

/* ========================================================================
 * XIV. FUNCTIO PRINCIPALIS — nullae optiones, nullum stdin
 * ==================================================================== */

int
main(void)
{
    caput_imprime();
    sectio_data();
    sectio_perceptron();
    sectio_hebb();
    sectio_ulp();
    sectio_subnormales();
    sectio_margin();
    sectio_classes();
    epilogus_imprime();
    return 0;
}

/* ========================================================================
 * APPENDIX A — DETAILS BINARY32 [IEEE754, Mul10]
 * ==================================================================== *
 *
 * Formatus binary32 (aka "float" in C):
 *   1 bit signi | 8 bit exponentis biasati | 23 bit mantissae
 *   Bias = 127.
 *   Numerus normalizatus: (-1)^s * 1.m * 2^(e - 127), e in [1, 254]
 *   Subnormalis:           (-1)^s * 0.m * 2^(-126), e = 0
 *   Infinitus:             e = 255, m = 0
 *   NaN:                   e = 255, m != 0
 *
 * ULP(x) pro x in [2^e, 2^(e+1)) est 2^(e-23).
 * Praecipue: ULP(1.0) = 2^-23 = FLT_EPSILON.
 *
 * Rotundatio defalta: ad proximum, pares ad par (roundTiesToEven).
 * Sub hac regula, summa a + b committit errorem absolutum
 * <= ULP(a+b) / 2.
 *
 * ─────────────────────────────────────────────────────────────────
 * APPENDIX B — THEOREMA CONVERGENTIAE NOVIKOFF [Nov62]
 * ─────────────────────────────────────────────────────────────────
 *
 * Si data {(x_i, t_i)} linearitate separabilia sunt cum margine
 * gamma > 0 et ||x_i|| <= R, tunc perceptron classicus maximum
 * (R/gamma)^2 errores committit ante convergentiam — absque respectu
 * dimensionis spatii. Probatio per arguamentum potentiae:
 *   w_k+1 . w* >= w_k . w* + gamma
 *   ||w_k+1||^2 <= ||w_k||^2 + R^2
 * Coniunctione, k <= (R/gamma)^2.
 *
 * Limes iste rumpitur per rotundationem si eta * gamma < ULP(w_k):
 * emendatio "legitima" perditur, et convergentia theoretica non
 * apparet practica. Haec sectio IV monstrat.
 *
 * ─────────────────────────────────────────────────────────────────
 * APPENDIX C — NOMENCLATURA
 * ─────────────────────────────────────────────────────────────────
 *
 *   neuronum       — cellula neuralis artificialis.
 *   activatio      — functio linearis w.x + b ante transformationem.
 *   pondus         — coefficiens synapticus, "weight".
 *   bias           — terminus constans, "intercept".
 *   epocha         — passus per totum datum docendi.
 *   margo          — distantia signata punctorum a hyperplano.
 *   fmaf           — fused multiply-add cum praecisione simplici.
 *   ULP            — Unit in the Last Place, gradus minimus
 *                    repraesentationis.
 *   subnormalis    — numerus in regione [2^-149, 2^-126) cum
 *                    exponente biasato = 0 et mantissa non-zero.
 * ==================================================================== */
