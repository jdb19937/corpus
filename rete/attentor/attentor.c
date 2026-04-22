/*
 * attentor.c — Attentio Scalata per Productum Scalarum
 *
 * Mechanismus attentionis per productum scalare (Vaswani et al.
 * 2017) est cor transformatoris moderni. Hoc programma
 * implementationem minimalem offert in binary32, focans in
 * proprietatibus numericis:
 *
 *   Attentio(Q, K, V) = softmax(Q K^T / sqrt(d_k)) V
 *
 *   Elementa:
 *     Q (n x d_k)  — interrogationes (queries)
 *     K (m x d_k)  — claves        (keys)
 *     V (m x d_v)  — valores       (values)
 *     scores (n x m) = Q K^T / sqrt(d_k)
 *     attentio (n x m) = softmax-per-row(scores)
 *     output (n x d_v) = attentio V
 *
 * ═══════════════════════════════════════════════════════════════════
 * QUAESTIONES NUMERICAE:
 *   I.   Sine scalatione 1/sqrt(d_k), valor Q.K crescit O(sqrt(d_k))
 *        sub initializatione standardi — softmax temperatura
 *        effective ad zero, gradiens evanescit.
 *   II.  Masca attentionis (causalis, padding): -INFINITY in scores
 *        redeunt ad 0 exacte post softmax per expf(-INF) = 0.
 *   III. Accumulatio producti Q.K in fmaf minimat errorem.
 *   IV.  Stabilitas softmax per translatio max (vide exponens.c).
 *   V.   Multi-head attentio: decomposit d_k in h capitula, concat.
 *   VI.  Cross-attentio vs self-attentio — eadem operatio, diversus
 *        ingressus.
 *
 * ═══════════════════════════════════════════════════════════════════
 * REFERENTIAE:
 *   [VSP17]  Vaswani A. et al. "Attention Is All You Need."
 *            NeurIPS 30 (2017). arXiv:1706.03762.
 *   [BCB14]  Bahdanau D., Cho K., Bengio Y. "Neural Machine
 *            Translation by Jointly Learning to Align and Translate."
 *            ICLR (2015). arXiv:1409.0473. [Attentio additiva]
 *   [LPM15]  Luong M.-T., Pham H., Manning C. D. "Effective Approaches
 *            to Attention-based Neural Machine Translation." EMNLP
 *            (2015). [Attentio multiplicativa]
 *   [DCL19]  Devlin J. et al. "BERT: Pre-training of Deep
 *            Bidirectional Transformers..." NAACL (2019).
 *   [RSR20]  Raffel C. et al. "T5: Exploring the Limits of Transfer
 *            Learning with a Unified Text-to-Text Transformer."
 *            JMLR 21: 1-67 (2020).
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

/* Dimensiones fixae pro demonstratione */
#define N_QRY    6      /* numerus queries (positiones decoder)      */
#define N_KEY    8      /* numerus keys (positiones encoder)         */
#define D_MOD    16     /* dimensio modelli (d_model)                */
#define N_HEAD   4      /* numerus capitum attentionis               */
#define D_HEAD   (D_MOD / N_HEAD)   /* = 4, dimensio per capitum     */

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
    f->status = x ? x : 0xF00DFEEDu;
    return f->status;
}
static float
forte_unus(Fors *f)
{
    return (float)(xorshift32(f) >> 8) * ldexpf(1.0f, -24);
}
static float
forte_norm(Fors *f)
{
    float s = 0.0f;
    for (int i = 0; i < 12; i++) s += forte_unus(f);
    return s - 6.0f;
}

/* ========================================================================
 * II. SOFTMAX-PER-ROW (STABILIS)
 * ==================================================================== */

static void
softmax_row(const float *src, float *dst, int m)
{
    float M = src[0];
    for (int j = 1; j < m; j++) if (src[j] > M) M = src[j];
    if (!isfinite(M)) {
        /* Si omnes -INF, distributio uniformis exitus (convention) */
        int finitus = 0;
        for (int j = 0; j < m; j++) if (isfinite(src[j])) { finitus = 1; break; }
        if (!finitus) {
            for (int j = 0; j < m; j++) dst[j] = 1.0f / (float)m;
            return;
        }
        M = -INFINITY;
        for (int j = 0; j < m; j++) if (isfinite(src[j]) && src[j] > M) M = src[j];
    }
    float Z = 0.0f;
    for (int j = 0; j < m; j++) {
        dst[j] = expf(src[j] - M);
        Z += dst[j];
    }
    if (Z > 0.0f) {
        float invZ = 1.0f / Z;
        for (int j = 0; j < m; j++) dst[j] *= invZ;
    }
}

/* ========================================================================
 * III. PRODUCTUM SCALARE STABILIS
 * ==================================================================== */

static float
productum_scalare(const float *a, const float *b, int d)
{
    float s = 0.0f;
    for (int i = 0; i < d; i++) s = fmaf(a[i], b[i], s);
    return s;
}

/* ========================================================================
 * IV. SCALED DOT-PRODUCT ATTENTION [VSP17]
 * ==================================================================== */

static void
attentio_scalata(
    const float *Q,           /* [n_qry x d_k] */
    const float *K,           /* [n_key x d_k] */
    const float *V,           /* [n_key x d_v] */
    int n_qry, int n_key, int d_k, int d_v,
    const float *masca,       /* NULL aut [n_qry x n_key] */
    float *output,            /* [n_qry x d_v] */
    float *attentio_out       /* NULL aut [n_qry x n_key] */
)
{
    const float scale = 1.0f / sqrtf((float)d_k);
    float *scores = malloc(sizeof(float) * (size_t)(n_qry * n_key));
    float *probs  = malloc(sizeof(float) * (size_t)(n_qry * n_key));

    /* Compute scores = Q K^T / sqrt(d_k) */
    for (int i = 0; i < n_qry; i++) {
        for (int j = 0; j < n_key; j++) {
            float s = productum_scalare(&Q[i * d_k], &K[j * d_k], d_k) * scale;
            if (masca) s += masca[i * n_key + j];
            scores[i * n_key + j] = s;
        }
    }
    /* Softmax per row */
    for (int i = 0; i < n_qry; i++) {
        softmax_row(&scores[i * n_key], &probs[i * n_key], n_key);
    }
    /* output = probs @ V */
    for (int i = 0; i < n_qry; i++) {
        for (int d = 0; d < d_v; d++) {
            float s = 0.0f;
            for (int j = 0; j < n_key; j++) {
                s = fmaf(probs[i * n_key + j], V[j * d_v + d], s);
            }
            output[i * d_v + d] = s;
        }
    }
    if (attentio_out) {
        memcpy(attentio_out, probs,
               sizeof(float) * (size_t)(n_qry * n_key));
    }
    free(scores);
    free(probs);
}

/* Versio SINE scalatione pro comparatione */
static void
attentio_non_scalata(
    const float *Q, const float *K, const float *V,
    int n_qry, int n_key, int d_k, int d_v,
    float *attentio_out)
{
    float *scores = malloc(sizeof(float) * (size_t)(n_qry * n_key));
    float *probs  = malloc(sizeof(float) * (size_t)(n_qry * n_key));
    for (int i = 0; i < n_qry; i++)
        for (int j = 0; j < n_key; j++)
            scores[i * n_key + j] =
                productum_scalare(&Q[i * d_k], &K[j * d_k], d_k);
    for (int i = 0; i < n_qry; i++)
        softmax_row(&scores[i * n_key], &probs[i * n_key], n_key);
    if (attentio_out)
        memcpy(attentio_out, probs,
               sizeof(float) * (size_t)(n_qry * n_key));
    (void)V; (void)d_v;
    free(scores);
    free(probs);
}

/* ========================================================================
 * V. INITIALIZATIO PARAMETRORUM
 * ====================================================================
 * Xavier/Glorot approximatum: stddev = 1/sqrt(d_in).
 * Post initializationem, product Q.K habet stddev ~ 1 si normalizamus. */

static void
init_matrix(float *M, int rows, int cols, float sigma, Fors *f)
{
    for (int i = 0; i < rows * cols; i++) M[i] = sigma * forte_norm(f);
}

/* ========================================================================
 * VI. SECTIO I — DEMONSTRATIO FUNDAMENTALIS
 * ==================================================================== */

static void
sectio_fundamentalis(void)
{
    DICERE("# ─── I. ATTENTIO SCALATA FUNDAMENTALIS ─────────────────────\n");
    /* Configuratio minima: 3 queries, 4 keys, d_k = 8 */
    const int n_q = 3, n_k = 4, d_k = 8, d_v = 4;
    float Q[3 * 8], K[4 * 8], V[4 * 4];
    float out[3 * 4], attn[3 * 4];
    Fors f = { .status = 0x11223344u };
    init_matrix(Q, n_q, d_k, 0.5f, &f);
    init_matrix(K, n_k, d_k, 0.5f, &f);
    init_matrix(V, n_k, d_v, 1.0f, &f);

    attentio_scalata(Q, K, V, n_q, n_k, d_k, d_v, NULL, out, attn);

    DICERE("# n_q=%d, n_k=%d, d_k=%d, d_v=%d\n", n_q, n_k, d_k, d_v);
    DICERE("# scale = 1/sqrt(d_k) = %.4f\n",
           (double)(1.0f / sqrtf((float)d_k)));
    DICERE("# Matrix attentionis (%d x %d), summa per row = 1:\n",
           n_q, n_k);
    DICERE("#         j=0     j=1     j=2     j=3    sum\n");
    for (int i = 0; i < n_q; i++) {
        float sum = 0.0f;
        DICERE("#   i=%d   ", i);
        for (int j = 0; j < n_k; j++) {
            DICERE("%.4f  ", (double)attn[i * n_k + j]);
            sum += attn[i * n_k + j];
        }
        DICERE("%.4f\n", (double)sum);
    }
    DICERE("# Output (%d x %d):\n", n_q, d_v);
    DICERE("#       d=0       d=1       d=2       d=3\n");
    for (int i = 0; i < n_q; i++) {
        DICERE("#   i=%d  ", i);
        for (int d = 0; d < d_v; d++) {
            DICERE("%+8.4f  ", (double)out[i * d_v + d]);
        }
        DICERE("\n");
    }
    DICERE("#\n");
}

/* ========================================================================
 * VII. SECTIO II — CUR SCALA 1/sqrt(d_k) NECESSARIA EST
 * ====================================================================
 * Sine scala: Q.K ~ sqrt(d_k) sub initializatione Normalis.
 * Ergo pro d_k=256, scores ~ ±16, softmax collapsit ad one-hot.
 * Gradiens evanescit (vide sigmoides.c, sectio saturatio). */

static void
sectio_scala(void)
{
    DICERE("# ─── II. CUR SCALA 1/sqrt(d_k) NECESSARIA EST [VSP17 §3.2.1]─\n");
    DICERE("# Comparatio entropiae attentionis per d_k crescens:\n");
    DICERE("#  d_k    sine_scala   cum_scala   var_scores  ratio\n");

    static const int D_KS[] = { 4, 16, 64, 256, 1024 };
    const int N = (int)(sizeof D_KS / sizeof D_KS[0]);
    Fors f = { .status = 0xDEADBEEFu };

    for (int k = 0; k < N; k++) {
        int d_k = D_KS[k];
        const int n_q = 1, n_k = 16;
        float *Q = malloc(sizeof(float) * (size_t)(n_q * d_k));
        float *K = malloc(sizeof(float) * (size_t)(n_k * d_k));
        float *V = malloc(sizeof(float) * (size_t)(n_k * 1));
        float attn_n[16], attn_s[16];
        init_matrix(Q, n_q, d_k, 1.0f, &f);
        init_matrix(K, n_k, d_k, 1.0f, &f);
        for (int i = 0; i < n_k; i++) V[i] = 0.0f;

        attentio_non_scalata(Q, K, V, n_q, n_k, d_k, 1, attn_n);
        attentio_scalata(Q, K, V, n_q, n_k, d_k, 1, NULL, V, attn_s);

        /* Entropia */
        float H_n = 0.0f, H_s = 0.0f;
        for (int j = 0; j < n_k; j++) {
            if (attn_n[j] > 0.0f) H_n -= attn_n[j] * logf(attn_n[j]);
            if (attn_s[j] > 0.0f) H_s -= attn_s[j] * logf(attn_s[j]);
        }
        /* Variantia empirica scores (sine scale) */
        float mean = 0.0f, sqs = 0.0f;
        for (int j = 0; j < n_k; j++) {
            float dot = productum_scalare(Q, &K[j * d_k], d_k);
            mean += dot;
            sqs += dot * dot;
        }
        mean /= (float)n_k;
        float var = sqs / (float)n_k - mean * mean;

        DICERE("#  %4d   %.4f       %.4f       %.4f    %.2f\n",
               d_k, (double)H_n, (double)H_s,
               (double)var, (double)(var / (float)d_k));
        SUSSURRO("# [II] d_k=%d H_n=%.6e H_s=%.6e var=%.6e\n",
                 d_k, (double)H_n, (double)H_s, (double)var);
        free(Q); free(K); free(V);
    }
    DICERE("# H_max = log(16) = %.4f (distributio uniformis)\n",
           (double)logf(16.0f));
    DICERE("# Sine scala, entropia collapsit cum d_k crescit.\n");
    DICERE("# Cum scala, entropia stabilis prope H_max manet.\n");
    DICERE("# Theoria: Var(Q.K) = d_k sub init N(0,1); scale = 1/sqrt(d_k)\n");
    DICERE("# restituit variantiam ad 1, ergo softmax temperatura normalis.\n");
    DICERE("#\n");
}

/* ========================================================================
 * VIII. SECTIO III — MASCA CAUSALIS [VSP17 §3.2.3]
 * ====================================================================
 * Pro decoder, positio i non potest attendere ad positiones j > i.
 * Implementatur per addendum -INFINITY ad scores[i][j] pro j > i
 * ante softmax. expf(-INF) = 0, ergo attn[i][j] = 0 exacte. */

static void
sectio_causalis(void)
{
    DICERE("# ─── III. MASCA ATTENTIONIS CAUSALIS ───────────────────────\n");
    const int n = 6, d = 8;
    float Q[6 * 8], K[6 * 8], V[6 * 4];
    float out[6 * 4], attn[6 * 6], masca[6 * 6];
    Fors f = { .status = 0x1A2B3C4Du };
    init_matrix(Q, n, d, 0.5f, &f);
    init_matrix(K, n, d, 0.5f, &f);
    init_matrix(V, n, 4, 1.0f, &f);
    /* Masca: 0 inferior-triangularis (permissa), -INFINITY superior */
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            masca[i * n + j] = (j > i) ? -INFINITY : 0.0f;

    attentio_scalata(Q, K, V, n, n, d, 4, masca, out, attn);

    DICERE("# Masca causalis (n = %d): M_{ij} = -INF si j > i.\n", n);
    DICERE("# Matrix attentionis (inferior triangularis):\n");
    DICERE("#           j=0    j=1    j=2    j=3    j=4    j=5\n");
    for (int i = 0; i < n; i++) {
        DICERE("#   i=%d   ", i);
        for (int j = 0; j < n; j++) {
            DICERE("%.4f ", (double)attn[i * n + j]);
        }
        DICERE("\n");
    }
    /* Probatio: superior triangularis exacte zero */
    int omnia_zero = 1;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (attn[i * n + j] != 0.0f) omnia_zero = 0;
    DICERE("# Superior triangularis exacte zero? %s\n",
           omnia_zero ? "ita (per expf(-INF) = 0)" : "non");
    DICERE("#\n");
}

/* ========================================================================
 * IX. SECTIO IV — MULTI-HEAD ATTENTIO
 * ==================================================================== */

static void
sectio_multihead(void)
{
    DICERE("# ─── IV. MULTI-HEAD ATTENTIO [VSP17 §3.2.2] ────────────────\n");
    DICERE("# d_model = %d, h = %d capita, d_head = d_model/h = %d\n",
           D_MOD, N_HEAD, D_HEAD);

    /* Sequentia: n = N_QRY = 6 positiones, cadaque dimensionis D_MOD */
    float X[N_QRY * D_MOD];
    Fors f = { .status = 0x55AA55AAu };
    init_matrix(X, N_QRY, D_MOD, 0.5f, &f);

    /* Proiectiones WQ, WK, WV ad [D_MOD x D_MOD] */
    float WQ[D_MOD * D_MOD], WK[D_MOD * D_MOD], WV[D_MOD * D_MOD];
    init_matrix(WQ, D_MOD, D_MOD, 0.25f, &f);
    init_matrix(WK, D_MOD, D_MOD, 0.25f, &f);
    init_matrix(WV, D_MOD, D_MOD, 0.25f, &f);

    /* Computa Q, K, V per proiectionem */
    float Q[N_QRY * D_MOD], K[N_QRY * D_MOD], V[N_QRY * D_MOD];
    for (int i = 0; i < N_QRY; i++) {
        for (int j = 0; j < D_MOD; j++) {
            float sq = 0.0f, sk = 0.0f, sv = 0.0f;
            for (int l = 0; l < D_MOD; l++) {
                sq = fmaf(X[i * D_MOD + l], WQ[l * D_MOD + j], sq);
                sk = fmaf(X[i * D_MOD + l], WK[l * D_MOD + j], sk);
                sv = fmaf(X[i * D_MOD + l], WV[l * D_MOD + j], sv);
            }
            Q[i * D_MOD + j] = sq;
            K[i * D_MOD + j] = sk;
            V[i * D_MOD + j] = sv;
        }
    }

    /* Pro quoque capite h, compute attention sub-matrix */
    float heads[N_HEAD][N_QRY * N_QRY];
    for (int h = 0; h < N_HEAD; h++) {
        /* Extract Q_h, K_h, V_h: columns [h*D_HEAD, (h+1)*D_HEAD) */
        float Qh[N_QRY * D_HEAD], Kh[N_QRY * D_HEAD], Vh[N_QRY * D_HEAD];
        for (int i = 0; i < N_QRY; i++) {
            for (int j = 0; j < D_HEAD; j++) {
                Qh[i * D_HEAD + j] = Q[i * D_MOD + h * D_HEAD + j];
                Kh[i * D_HEAD + j] = K[i * D_MOD + h * D_HEAD + j];
                Vh[i * D_HEAD + j] = V[i * D_MOD + h * D_HEAD + j];
            }
        }
        float out_h[N_QRY * D_HEAD], attn_h[N_QRY * N_QRY];
        attentio_scalata(Qh, Kh, Vh, N_QRY, N_QRY, D_HEAD, D_HEAD,
                         NULL, out_h, attn_h);
        memcpy(heads[h], attn_h,
               sizeof(float) * N_QRY * N_QRY);
    }

    DICERE("# Attentio per capitum, diagonal dominans (self):\n");
    DICERE("#  capit   attn[0,0] attn[1,1] attn[2,2] attn[3,3] entropia_medium\n");
    for (int h = 0; h < N_HEAD; h++) {
        /* Entropia media per rows */
        float H_sum = 0.0f;
        for (int i = 0; i < N_QRY; i++) {
            float H = 0.0f;
            for (int j = 0; j < N_QRY; j++) {
                float p = heads[h][i * N_QRY + j];
                if (p > 0.0f) H -= p * logf(p);
            }
            H_sum += H;
        }
        H_sum /= (float)N_QRY;
        DICERE("#   h=%d   %.4f    %.4f    %.4f    %.4f    %.4f\n",
               h,
               (double)heads[h][0 * N_QRY + 0],
               (double)heads[h][1 * N_QRY + 1],
               (double)heads[h][2 * N_QRY + 2],
               (double)heads[h][3 * N_QRY + 3],
               (double)H_sum);
    }
    DICERE("# Diversa capita diversos patterns attendent — cardinalis\n");
    DICERE("# beneficium multi-head attentionis.\n");
    DICERE("#\n");
}

/* ========================================================================
 * X. SECTIO V — PADDING MASCA
 * ====================================================================
 * In batchis sequentia longitudine varia, positiones "padding" ad
 * longitudinem uniformem additae. Scores pro padding -INFINITY ut
 * attentio eas ignoret. */

static void
sectio_padding(void)
{
    DICERE("# ─── V. MASCA PADDING (BATCH UNIFORMITAS) ──────────────────\n");
    /* Sequentia reali longitudinis 3, paddata ad 8 */
    const int n_q = 1, n_k = 8, d_k = 4, d_v = 4;
    const int n_reales = 3;
    float Q[1 * 4], K[8 * 4], V[8 * 4];
    float masca[1 * 8];
    float out[1 * 4], attn[1 * 8];
    Fors f = { .status = 0x99887766u };
    init_matrix(Q, n_q, d_k, 0.5f, &f);
    init_matrix(K, n_k, d_k, 0.5f, &f);
    init_matrix(V, n_k, d_v, 1.0f, &f);
    for (int j = 0; j < n_k; j++)
        masca[j] = (j < n_reales) ? 0.0f : -INFINITY;

    attentio_scalata(Q, K, V, n_q, n_k, d_k, d_v, masca, out, attn);

    DICERE("# Sequentia reali = %d positiones, paddata ad %d.\n",
           n_reales, n_k);
    DICERE("# Masca: [0, 0, 0, -INF, -INF, -INF, -INF, -INF]\n");
    DICERE("# Attentio:\n");
    DICERE("#   pos:     0      1      2      3      4      5      6      7\n");
    DICERE("#   attn: ");
    for (int j = 0; j < n_k; j++) DICERE("%.4f ", (double)attn[j]);
    DICERE("\n");
    float s_real = 0.0f, s_pad = 0.0f;
    for (int j = 0; j < n_reales; j++) s_real += attn[j];
    for (int j = n_reales; j < n_k; j++) s_pad += attn[j];
    DICERE("# Summa attn[0..2] (reales)      = %.4f (~1.0)\n",
           (double)s_real);
    DICERE("# Summa attn[3..7] (padding)     = %.4f (exacte 0)\n",
           (double)s_pad);
    DICERE("#\n");
}

/* ========================================================================
 * XI. CAPUT ET EPILOGUS
 * ==================================================================== */

static void
caput_imprime(void)
{
    DICERE("# ═══════════════════════════════════════════════════════════\n");
    DICERE("# ATTENTOR v1.0.0 — Scaled Dot-Product Attention binary32\n");
    DICERE("# ═══════════════════════════════════════════════════════════\n");
    DICERE("# Formula [VSP17 §3.2.1]:\n");
    DICERE("#   Attentio(Q, K, V) = softmax(Q K^T / sqrt(d_k)) V\n");
    DICERE("#\n");
}

static void
epilogus(void)
{
    DICERE("# ─── EPILOGUS ──────────────────────────────────────────────\n");
    DICERE("# Sex principia attentionis in binary32:\n");
    DICERE("#   1. Scale 1/sqrt(d_k) conservat variantiam scores = 1,\n");
    DICERE("#      ergo softmax non-saturatur.\n");
    DICERE("#   2. Masca -INFINITY: expf(-INF) = 0 exacte, nulla\n");
    DICERE("#      conditional branch necessaria post softmax.\n");
    DICERE("#   3. Productum scalare per fmaf: cumulatio stabilis.\n");
    DICERE("#   4. Softmax-per-row cum translatione max: evitat\n");
    DICERE("#      supereffluxionem expf pro scores magnis.\n");
    DICERE("#   5. Multi-head: capita diverse patronum atender poterant.\n");
    DICERE("#   6. Padding masca: sequentia mixtae longitudinis\n");
    DICERE("#      efficienter batchis tractatae.\n");
    DICERE("# ═══════════════════════════════════════════════════════════\n");
}

/* ========================================================================
 * XII. FUNCTIO PRINCIPALIS
 * ==================================================================== */

int
main(void)
{
    caput_imprime();
    sectio_fundamentalis();
    sectio_scala();
    sectio_causalis();
    sectio_multihead();
    sectio_padding();
    epilogus();
    return 0;
}

/* ========================================================================
 * APPENDIX A — DERIVATIO SCALAE 1/sqrt(d_k)
 * ====================================================================
 *
 * Initializatio: Q_i, K_j ~ N(0, 1) vectores independentes in R^{d_k}.
 * Productum scalare:
 *   Z = Q.K = sum_{l=1}^{d_k} Q_l K_l
 * Quia Q_l, K_l mutuae independentes et utrumque N(0, 1):
 *   E[Q_l K_l] = 0
 *   Var(Q_l K_l) = E[(Q_l K_l)^2] = E[Q_l^2] E[K_l^2] = 1
 * Ergo:
 *   Var(Z) = sum Var(Q_l K_l) = d_k
 *   std(Z) = sqrt(d_k)
 *
 * Si d_k = 64, scores ~ ±8; softmax temperatura effective 1/std ~ 1/8,
 * distributio acuta, gradiens minor. Pro d_k = 1024, scores ~ ±32;
 * softmax collapsit ad one-hot.
 *
 * Scale 1/sqrt(d_k):
 *   Z' = Z / sqrt(d_k)
 *   Var(Z') = Var(Z) / d_k = 1
 * Ergo softmax temperatura stabilis trans d_k omnia.
 *
 * ─────────────────────────────────────────────────────────────────
 * APPENDIX B — COMPARATIO ATTENTIO ADDITIVA VS MULTIPLICATIVA
 * ─────────────────────────────────────────────────────────────────
 *
 * Additiva [BCB14]:
 *   score(q, k) = v^T tanh(W_q q + W_k k)
 *   Complexitas: 2*d_h parametres extra per dimensionem.
 *
 * Multiplicativa / dot-product [LPM15]:
 *   score(q, k) = q . k    (sine scala)
 *
 * Multiplicativa scalata [VSP17]:
 *   score(q, k) = (q . k) / sqrt(d_k)
 *   Complexitas: 0 parametres extra.
 *   In hardware: MATMUL primitivum altamente optimizatum.
 *
 * Multiplicativa scalata est nunc standard quia cost-efficienter et
 * GPU-friendly.
 *
 * ─────────────────────────────────────────────────────────────────
 * APPENDIX C — FLASH ATTENTION [Dao et al. 2022]
 * ─────────────────────────────────────────────────────────────────
 *
 * Implementatio naïva attentionis memory O(n^2): matrix scores
 * plenae conservatur. Pro n = 2048, matrix = 16 MB in fp32.
 *
 * Flash Attention tile-based computation usat: fuses scores,
 * softmax, et multiplication V in uno loop per blocks in cache.
 * Memory O(n), speedup ~2-4x in practica.
 *
 * Algorithmus tamen eandem outputem numericam producit (modulo
 * ordinem summationis in softmax numerator) — non implementatur
 * hic propter complexitatem.
 *
 * ═══════════════════════════════════════════════════════════════════ */
