/*
 * summator.c — Summatio Compensata [Kahan 1965, Neumaier 1974]
 *
 * Summa n numerorum fluitantium, si naive computata, errorem
 * committit O(n * eps * sum |a_i|). Algorithmi compensati hunc
 * terminum ad O(eps * sum |a_i|) reducunt — ut summa n-millium
 * praecisio quasi duplex emergat ex binary32.
 *
 * Hoc programma VIII methodos summationis comparat in contextu
 * machinae discentis: computatio denominatoris softmax, summatio
 * gradientum per batchum stochasticum, et rare illustrant
 * catastropham cum summa partialis proxima zero evenit.
 *
 * ═══════════════════════════════════════════════════════════════════
 * METHODI:
 *   1. NAIVA            - s += a[i]                    O(n eps |S|)
 *   2. KAHAN            - cum termino correctionis     O(eps |S|)
 *   3. NEUMAIER (KBN)   - Kahan-Babuska-Neumaier       O(eps |S|)
 *   4. KLEIN            - duplex compensatio           O(eps |S|)
 *   5. PAIRWISE         - reductio recursiva           O(log n * eps |S|)
 *   6. SORTED_ASC       - praesortitio ascendens       O(n eps |S|) reductum
 *   7. SORTED_DSC       - praesortitio descendens      O(n eps |S|) auctum
 *   8. REFERENCE        - accumulatio in duplici       tanquam veritas
 *
 * EXPERIMENTA:
 *   A. Serien harmonicam H_n = sum 1/k computat pro n = 10^6.
 *   B. Denominator softmax: sum exp(logit_i) cum logitis varii.
 *   C. Cancellatio catastrophica: sum a_i cum a_i ex gaussiano
 *      circa zero, partialis summa oscillans.
 *   D. Gradientes SGD: summa correctionum in batcho 1024.
 *
 * ═══════════════════════════════════════════════════════════════════
 * REFERENTIAE:
 *   [Kah65]  Kahan W. "Further remarks on reducing truncation errors."
 *            Communications of the ACM 8(1): 40 (1965).
 *   [Neu74]  Neumaier A. "Rundungsfehleranalyse einiger Verfahren zur
 *            Summation endlicher Summen." Zeitschrift fuer Angewandte
 *            Mathematik und Mechanik 54(1): 39-51 (1974).
 *   [Kle06]  Klein A. "A Generalized Kahan-Babuska-Summation-
 *            Algorithm." Computing 76(3-4): 279-293 (2006).
 *   [Hig93]  Higham N. J. "The accuracy of floating point summation."
 *            SIAM J. Sci. Comput. 14(4): 783-799 (1993).
 *   [Hig02]  Higham N. J. "Accuracy and Stability of Numerical
 *            Algorithms." 2nd ed., SIAM (2002). Cap. 4.
 *   [Gol91]  Goldberg D. "What Every Computer Scientist..." §4.
 *
 * Nullae optiones CLI, nullum stdin. Egressus deterministicus.
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

#define DICERE(...)    fprintf(stdout, __VA_ARGS__)
#define SUSSURRO(...)  fprintf(stderr, __VA_ARGS__)

/* Utilitas diagnostica: bitti exactos 32 exprimit (pro debug stderr).
 * Inline ne compilator inutilem queretur. */
static inline uint32_t
bitti(float x)
{
    uint32_t u;
    memcpy(&u, &x, sizeof u);
    return u;
}
static void bitti_emit(float x) {
    SUSSURRO("# bitti = 0x%08" PRIx32 "\n", bitti(x));
}

/* ========================================================================
 * I. GENERATOR DETERMINISTICUS — xorshift32
 * ==================================================================== */

typedef struct { uint32_t status; } Fors;

static inline uint32_t
xorshift32(Fors *f)
{
    uint32_t x = f->status;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    f->status = x ? x : 0xCAFEBABEu;
    return f->status;
}

/* Numerus in [-1, +1] per 24 bittos mantissae. */
static float
forte_signatus(Fors *f)
{
    uint32_t u = xorshift32(f) >> 8;    /* 24 bit */
    float x = (float)u * ldexpf(1.0f, -23) - 1.0f;
    return x;
}

/* ========================================================================
 * II. ALGORITHMI SUMMATIONIS
 * ==================================================================== */

/* NAIVA: summa simplex per ordinem indicis.
 * Error vinculum: |s_computata - s_exacta| <= n * eps * sum |a_i|. */
static float
summa_naiva(const float *a, int n)
{
    float s = 0.0f;
    for (int i = 0; i < n; i++) s += a[i];
    return s;
}

/* KAHAN [Kah65]: termine correctionis "c" errorem per iterationem
 * proxime annihilat. Volatile cogit compilatorem ordinem servare
 * contra optimationem quae compensationem elimineret. */
static float
summa_kahan(const float *a, int n)
{
    volatile float s = 0.0f;
    volatile float c = 0.0f;   /* compensatio */
    for (int i = 0; i < n; i++) {
        volatile float y = a[i] - c;
        volatile float t = s + y;
        c = (t - s) - y;        /* bitta perdita cumulatur in c */
        s = t;
    }
    return s;
}

/* NEUMAIER (KBN) [Neu74]: emendat Kahan pro casu |a_i| > |s|.
 * In eo casu bitti perditi sunt in a_i, non s. */
static float
summa_neumaier(const float *a, int n)
{
    volatile float s = a[0];
    volatile float c = 0.0f;
    for (int i = 1; i < n; i++) {
        volatile float t = s + a[i];
        if (fabsf(s) >= fabsf(a[i]))
            c += (s - t) + a[i];
        else
            c += (a[i] - t) + s;
        s = t;
    }
    return s + c;
}

/* KLEIN [Kle06]: compensatio iterata — correctio ipsa corrigitur.
 * Approximatio praecisionis quadruplicis. */
static float
summa_klein(const float *a, int n)
{
    volatile float s = 0.0f;
    volatile float cs = 0.0f;
    volatile float ccs = 0.0f;
    for (int i = 0; i < n; i++) {
        volatile float t = s + a[i];
        volatile float c;
        if (fabsf(s) >= fabsf(a[i]))
            c = (s - t) + a[i];
        else
            c = (a[i] - t) + s;
        s = t;
        volatile float t2 = cs + c;
        volatile float cc;
        if (fabsf(cs) >= fabsf(c))
            cc = (cs - t2) + c;
        else
            cc = (c - t2) + cs;
        cs = t2;
        ccs = ccs + cc;
    }
    return s + cs + ccs;
}

/* PAIRWISE: reductio arborea. Error O(log2(n) * eps). */
static float
summa_pairwise_rec(const float *a, int lo, int hi)
{
    int n = hi - lo;
    if (n <= 0) return 0.0f;
    if (n == 1) return a[lo];
    if (n <= 8) {
        float s = 0.0f;
        for (int i = lo; i < hi; i++) s += a[i];
        return s;
    }
    int mid = lo + n / 2;
    return summa_pairwise_rec(a, lo, mid)
         + summa_pairwise_rec(a, mid, hi);
}
static float
summa_pairwise(const float *a, int n)
{
    return summa_pairwise_rec(a, 0, n);
}

/* Comparator ad praesortandum. */
static int
cmp_abs_asc(const void *pa, const void *pb)
{
    float x = fabsf(*(const float *)pa);
    float y = fabsf(*(const float *)pb);
    if (x < y) return -1;
    if (x > y) return +1;
    return 0;
}
static int
cmp_abs_dsc(const void *pa, const void *pb)
{
    return -cmp_abs_asc(pa, pb);
}

/* SORTED_ASC: minimos primo (reduction error).
 * SORTED_DSC: maximos primo (amplificat error). */
static float
summa_sorted(const float *a, int n, int ascendens)
{
    float *copia = malloc(sizeof(float) * (size_t)n);
    memcpy(copia, a, sizeof(float) * (size_t)n);
    qsort(copia, (size_t)n, sizeof(float),
          ascendens ? cmp_abs_asc : cmp_abs_dsc);
    float s = summa_naiva(copia, n);
    free(copia);
    return s;
}

/* (summa_duplex olim hic definita erat; eliminata quia evalua
 *  directe in duplici accumulat.) */

/* ========================================================================
 * III. FUNCTIONES DIAGNOSTICAE
 * ==================================================================== */

typedef struct {
    const char *nomen;
    float       valor;
    float       err_abs;
    float       err_rel;
} Resultum;

static void
evalua(const char *nomen, float valor, double veritas, Resultum *r)
{
    r->nomen = nomen;
    r->valor = valor;
    float err = (float)((double)valor - veritas);
    r->err_abs = fabsf(err);
    float den = (float)fabs(veritas);
    r->err_rel = (den > 0.0f) ? r->err_abs / den : 0.0f;
}

static void
tabula_resulta(const Resultum *tab, int n, double veritas)
{
    DICERE("#  methodus       valor binary32\n");
    for (int i = 0; i < n; i++) {
        DICERE("#  %-13s  %+.3e\n", tab[i].nomen, (double)tab[i].valor);
        SUSSURRO("#  %-13s  valor=%+.9e  err_abs=%.6e  err_rel=%.6e\n",
                 tab[i].nomen, (double)tab[i].valor,
                 (double)tab[i].err_abs, (double)tab[i].err_rel);
    }
    DICERE("#  REFERENCE      %+.3e    (veritas duplex)\n", veritas);
    SUSSURRO("#  REFERENCE     veritas=%+.15e\n", veritas);
}

/* ========================================================================
 * IV. EXPERIMENTUM A — SERIES HARMONICA
 * ====================================================================
 * H_n = sum_{k=1}^{n} 1/k. Pro n=10^6 terminis, termini parvi
 * post k=10^5 fere negligentur in summa naiva. */

static void
experimentum_harmonicum(void)
{
    DICERE("# ─── EXPERIMENTUM A: SERIES HARMONICA H_n ──────────────────\n");
    const int n = 1000000;
    float *a = malloc(sizeof(float) * (size_t)n);
    for (int i = 0; i < n; i++) a[i] = 1.0f / (float)(i + 1);

    double veritas = 0.0;
    for (int i = 0; i < n; i++) veritas += 1.0 / (double)(i + 1);

    Resultum tab[7];
    evalua("naiva",      summa_naiva(a, n),              veritas, &tab[0]);
    evalua("kahan",      summa_kahan(a, n),              veritas, &tab[1]);
    evalua("neumaier",   summa_neumaier(a, n),           veritas, &tab[2]);
    evalua("klein",      summa_klein(a, n),              veritas, &tab[3]);
    evalua("pairwise",   summa_pairwise(a, n),           veritas, &tab[4]);
    evalua("sorted_asc", summa_sorted(a, n, 1),          veritas, &tab[5]);
    evalua("sorted_dsc", summa_sorted(a, n, 0),          veritas, &tab[6]);

    DICERE("# n = %d termini\n", n);
    tabula_resulta(tab, 7, veritas);
    SUSSURRO("# [A] veritas plena = %.15e\n", veritas);
    free(a);
    DICERE("#\n");
}

/* ========================================================================
 * V. EXPERIMENTUM B — DENOMINATOR SOFTMAX
 * ====================================================================
 * Z = sum exp(logit_i). Si logiti post shift-by-max locantur, omnes
 * in [-K, 0], et exp(logit) in (0, 1]. Summatio paucarum centum
 * valorum fit in parvo dynamic range — sed bene illustrat artem. */

static void
experimentum_softmax(void)
{
    DICERE("# ─── EXPERIMENTUM B: DENOMINATOR SOFTMAX ───────────────────\n");
    const int n = 2048;
    float *logit = malloc(sizeof(float) * (size_t)n);
    float *expl  = malloc(sizeof(float) * (size_t)n);
    Fors f = { .status = 0xDEADBEEFu };
    float maximus = -INFINITY;
    for (int i = 0; i < n; i++) {
        logit[i] = forte_signatus(&f) * 8.0f;   /* logiti in [-8, +8] */
        if (logit[i] > maximus) maximus = logit[i];
    }
    /* Shift-by-max pro stabilitate (vide exponens.c) */
    for (int i = 0; i < n; i++) expl[i] = expf(logit[i] - maximus);

    double veritas = 0.0;
    for (int i = 0; i < n; i++) {
        veritas += exp((double)logit[i] - (double)maximus);
    }

    Resultum tab[5];
    evalua("naiva",    summa_naiva(expl, n),    veritas, &tab[0]);
    evalua("kahan",    summa_kahan(expl, n),    veritas, &tab[1]);
    evalua("neumaier", summa_neumaier(expl, n), veritas, &tab[2]);
    evalua("pairwise", summa_pairwise(expl, n), veritas, &tab[3]);
    evalua("sorted_a", summa_sorted(expl, n, 1), veritas, &tab[4]);

    DICERE("# n = %d, logiti ~ Uniform[-8, +8], max = %.4f\n",
           n, (double)maximus);
    tabula_resulta(tab, 5, veritas);
    SUSSURRO("# [B] maximus plena = %.9e\n", (double)maximus);

    /* Omni resulta inter se praecisione binary32 consistentia sunt
     * quia dynamic range parvus est — vera vis compensationis
     * in casibus magnis extenditur. */
    free(logit);
    free(expl);
    DICERE("#\n");
}

/* ========================================================================
 * VI. EXPERIMENTUM C — CANCELLATIO CATASTROPHICA
 * ====================================================================
 * Data ex gaussiano approximato cum media zero, sed praecedit
 * valorem magnum qui per summa partialis minime-praecisione ruinat
 * adiunctiones subsequentes. */

static void
experimentum_cancellatio(void)
{
    DICERE("# ─── EXPERIMENTUM C: CANCELLATIO CATASTROPHICA ─────────────\n");
    const int n = 100000;
    float *a = malloc(sizeof(float) * (size_t)n);
    Fors f = { .status = 0xC0DEFACEu };

    /* Termini: unus magnus 1e7, alter -1e7, dein n-2 termini parvi
     * de magnitudine 1e-3 cum summa expectata ~ 1e-3 * (n-2). */
    a[0] = +1.0e7f;
    a[1] = -1.0e7f;
    for (int i = 2; i < n; i++) {
        a[i] = forte_signatus(&f) * 0.001f + 0.001f;
    }
    double veritas = 0.0;
    for (int i = 0; i < n; i++) veritas += (double)a[i];

    Resultum tab[7];
    evalua("naiva",      summa_naiva(a, n),              veritas, &tab[0]);
    evalua("kahan",      summa_kahan(a, n),              veritas, &tab[1]);
    evalua("neumaier",   summa_neumaier(a, n),           veritas, &tab[2]);
    evalua("klein",      summa_klein(a, n),              veritas, &tab[3]);
    evalua("pairwise",   summa_pairwise(a, n),           veritas, &tab[4]);
    evalua("sorted_asc", summa_sorted(a, n, 1),          veritas, &tab[5]);
    evalua("sorted_dsc", summa_sorted(a, n, 0),          veritas, &tab[6]);

    DICERE("# a = [+1e7, -1e7, ..., rand(0..2e-3) x %d]\n", n - 2);
    tabula_resulta(tab, 7, veritas);
    DICERE("# Nota: naiva et sorted_dsc maximum errorem patiuntur,\n");
    DICERE("# quia terminos magnos primo sumant et parvos deinde\n");
    DICERE("# perdant per rotundationem.\n");
    SUSSURRO("# [C] veritas plena = %.15e\n", veritas);
    free(a);
    DICERE("#\n");
}

/* ========================================================================
 * VII. EXPERIMENTUM D — GRADIENTES SGD
 * ====================================================================
 * Summatio gradientum in batcho 1024 — imitat step SGD realis. */

static void
experimentum_sgd(void)
{
    DICERE("# ─── EXPERIMENTUM D: GRADIENTES SGD BATCH ──────────────────\n");
    const int n = 1024;
    float *grad = malloc(sizeof(float) * (size_t)n);
    Fors f = { .status = 0xFEEDFACEu };
    for (int i = 0; i < n; i++) {
        /* Gradientes ~ N(0, 0.1) approximatum */
        float g = 0.0f;
        for (int k = 0; k < 6; k++) g += forte_signatus(&f);
        grad[i] = g * 0.04f;
    }
    double veritas = 0.0;
    for (int i = 0; i < n; i++) veritas += (double)grad[i];

    Resultum tab[5];
    evalua("naiva",    summa_naiva(grad, n),    veritas, &tab[0]);
    evalua("kahan",    summa_kahan(grad, n),    veritas, &tab[1]);
    evalua("neumaier", summa_neumaier(grad, n), veritas, &tab[2]);
    evalua("klein",    summa_klein(grad, n),    veritas, &tab[3]);
    evalua("pairwise", summa_pairwise(grad, n), veritas, &tab[4]);

    DICERE("# n = %d, grad ~ N(0, 0.1) approximatum\n", n);
    tabula_resulta(tab, 5, veritas);
    free(grad);
    DICERE("#\n");
}

/* ========================================================================
 * VIII. EXPERIMENTUM E — DUO NUMERI "VENENATI"
 * ====================================================================
 * Exemplum textbook [Neu74] cum tribus valoribus ubi Kahan fallit
 * sed Neumaier succedit: a = [1, 1e100, 1, -1e100].
 * Pro binary32: 1e38 loco 1e100 quia FLT_MAX ~ 3.4e38. */

static void
experimentum_venenatum(void)
{
    DICERE("# ─── EXPERIMENTUM E: CASUS KAHAN FALLIT [Neu74] ────────────\n");
    float a[4] = { 1.0f, 1.0e30f, 1.0f, -1.0e30f };
    double veritas = 2.0;   /* exactus */

    Resultum tab[4];
    evalua("naiva",    summa_naiva(a, 4),    veritas, &tab[0]);
    evalua("kahan",    summa_kahan(a, 4),    veritas, &tab[1]);
    evalua("neumaier", summa_neumaier(a, 4), veritas, &tab[2]);
    evalua("klein",    summa_klein(a, 4),    veritas, &tab[3]);

    DICERE("# a = [1, 1e30, 1, -1e30], expectatum = 2\n");
    tabula_resulta(tab, 4, veritas);
    DICERE("# Observatio: Kahan erraverit quia correctio perdita in\n");
    DICERE("# passu 1 (cum 1e30 sumptus, 1 in c inseritur, sed\n");
    DICERE("# subtractio 1e30 etiam c annihilabit). Neumaier vigilat.\n");
    DICERE("#\n");
}

/* ========================================================================
 * IX. CAPUT ET EPILOGUS
 * ==================================================================== */

static void
caput_imprime(void)
{
    DICERE("# ═══════════════════════════════════════════════════════════\n");
    DICERE("# SUMMATOR v1.0.0 — Summatio Compensata binary32\n");
    DICERE("# ═══════════════════════════════════════════════════════════\n");
    DICERE("# Methodi comparatae:\n");
    DICERE("#   naiva     — s += a[i]\n");
    DICERE("#   kahan     — [Kah65] termini correctionis\n");
    DICERE("#   neumaier  — [Neu74] emendatio pro |a_i| > |s|\n");
    DICERE("#   klein     — [Kle06] duplex compensatio\n");
    DICERE("#   pairwise  — reductio arborea\n");
    DICERE("#   sorted_*  — praesortitio magnitudinis\n");
    DICERE("#\n");
    DICERE("# FLT_EPSILON = %.4e\n", (double)FLT_EPSILON);
    DICERE("#\n");
    (void)bitti_emit;    /* tacet warning si debug non activum */
}

static void
epilogus(void)
{
    DICERE("# ─── EPILOGUS ──────────────────────────────────────────────\n");
    DICERE("# Quinque principia summationis fluitantis:\n");
    DICERE("#   1. Error naivus O(n eps); compensatus O(eps).\n");
    DICERE("#   2. Kahan Neumaier Klein praecisionem quasi duplicem\n");
    DICERE("#      per praecisionem simplicem procurant.\n");
    DICERE("#   3. Pairwise sine memoria compensationis O(log n eps).\n");
    DICERE("#   4. Praesortitio ascendens errorem minuit, descendens\n");
    DICERE("#      errorem auget [Hig93].\n");
    DICERE("#   5. Volatile cogit compilatorem ordinem servare — sine\n");
    DICERE("#      hoc, -O3 compensationem annihilare potest.\n");
    DICERE("# ═══════════════════════════════════════════════════════════\n");
}

/* ========================================================================
 * X. FUNCTIO PRINCIPALIS
 * ==================================================================== */

int
main(void)
{
    caput_imprime();
    experimentum_harmonicum();
    experimentum_softmax();
    experimentum_cancellatio();
    experimentum_sgd();
    experimentum_venenatum();
    epilogus();
    return 0;
}

/* ========================================================================
 * APPENDIX A — PROBATIO VINCULI KAHAN [Kah65]
 * ====================================================================
 *
 * Theorema: Algorithmus Kahan producit summam quae differt a summa
 * exacta per:
 *   |s_comp - sum a_i| <= (2 * eps + O(n * eps^2)) * sum |a_i|
 *
 * Loco O(n * eps) pro summa naiva — reductio factoris n est.
 *
 * Probatio usi key: in quoque passu,
 *   y = a_i - c_i
 *   t = s_i + y
 *   c_{i+1} = (t - s_i) - y   (pars y perdita)
 *   s_{i+1} = t
 *
 * Error absolutus commissus per summam s_i + y est bound per 2*eps*|y|.
 * Per traceam per n passus, error totalis habet coeficientem 2*eps,
 * independentem ab n. Cf. [Hig93, Thm. 4.7] pro rigore pleno.
 *
 * ─────────────────────────────────────────────────────────────────
 * APPENDIX B — CUR NEUMAIER KAHAN SUPERAT
 * ─────────────────────────────────────────────────────────────────
 *
 * Kahan praesupponit |s| >= |a_i|. Si contrarium — e.g., primus a_0
 * magnus, deinde a_1 adhuc maior — tunc "bitti perditi" sunt in s,
 * non in a_i. Kahan in hoc casu falso in c parte a_i addit, non
 * bittos servatos.
 *
 * Neumaier ramum inspicit magnitudinis et ponit correctionem in
 * membrum minorem. In [Neu74, p. 43] exemplum canonicum:
 *   a = [1.0, 1e100, 1.0, -1e100]
 *   exactum: 2.0
 *   Kahan:   0.0   (corruptus)
 *   Neumaier: 2.0  (correctus)
 *
 * ─────────────────────────────────────────────────────────────────
 * APPENDIX C — QUIS IN PRACTICA?
 * ─────────────────────────────────────────────────────────────────
 *
 * Recommendatio:
 *   — Summatio <= 100 terminorum: naiva satis est.
 *   — Summatio 10^4–10^6 terminorum: pairwise (sine overhead Kahan).
 *   — Mixed-magnitude vel adversarial: Neumaier aut Klein.
 *   — Critical path GPU: pairwise (sine branches).
 *   — Compiler aggresivus (-Ofast, -ffast-math): Kahan/Neumaier
 *     potest rumpi sine volatile aut pragma.
 *
 * ═══════════════════════════════════════════════════════════════════ */
