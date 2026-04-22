/*
 * augusta.c — Machina Analytica Augustae (Ada Augusta Lovelace)
 *
 * Automaton procedurale quod imitatur mentem Augustae Adae: texit
 * responsa ex "cartis operationum" (cartae perforatae) more telae
 * Iacquardianae.  Intus habet thesaurum NUMERORUM (registra integra)
 * atque FORMAM symbolicam; quaelibet responsio componitur per
 * selectionem et sequelam cartarum: VARIABILIS, OPERATOR, MODIFICATOR,
 * REPETITOR, EGESSIO.
 *
 * Classificator argumenti inspicit claves Latinas in decem saltem
 * regionibus (numerus, series, machina, calculus, poesis, tela,
 * musica, pulchritudo, harmonia, differentia, integrale, fatum).
 * Classificatio eligit initialem sequentiam cartarum.  Compositio
 * poetica interponit propositionem mathematicam flosculo lyrico
 * secundum grammaticam templatorum ponderatam.
 *
 * Per colloquium: registrum a congerit "pulchritudinem conspectam",
 * registrum b congerit "veritatem inventam".  Argumenta interlocutoris
 * haec registra inclinant; responsa ad valores registrorum redeunt.
 *
 * Generator numerorum fortuitorum: xorshift64, semine constante
 * defaltoque, vel per argumentum -s N substituto.
 *
 * Invocatio:
 *   augusta            -> colloquium scriptum, saltem XIV vices
 *   augusta -i         -> colloquium per stdin, linea post lineam
 *   augusta -s N       -> semen generatoris mutat
 *   augusta <ignotum>  -> usus Latinus in stderr, exitus 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <limits.h>

/* ========================================================================
 * DEFINITIONES CONSTANTES
 * ==================================================================== */

#define SEMEN_DEFALTUM       0xA11CEADA1729ULL
#define CAPACITAS_BULLAE     8192
#define CAPACITAS_LINEAE     1024
#define CAPACITAS_VOCABULI   64
#define MAX_VOCABULA         64
#define MAX_REGISTRA         16
#define MAX_CARTAE_SEQ       12
#define MAX_SYMBOLA_FORMAE   32
#define VERSIO_MAIOR         1
#define VERSIO_MINOR         7
#define VERSIO_MINIMA        29
#define TURNI_SCRIPTI_MIN    14

/* Macra variadica pro diagnostica */
#define DICERE(flumen, ...)  fprintf((flumen), __VA_ARGS__)
#define ERRO(...)            DICERE(stderr, __VA_ARGS__)

/* X-Macrum regionum argumenti */
#define REGIONES_TABULA \
    X(REGIO_NUMERUS,      "numerus",      "numeros et figuras") \
    X(REGIO_SERIES,       "series",       "series infinitas") \
    X(REGIO_MACHINA,      "machina",      "machinas analyticas") \
    X(REGIO_CALCULUS,     "calculus",     "calculum symbolicum") \
    X(REGIO_POESIS,       "poesis",       "poesim scientificam") \
    X(REGIO_TELA,         "tela",         "telam Iacquardianam") \
    X(REGIO_MUSICA,       "musica",       "musicam numerorum") \
    X(REGIO_PULCHRITUDO,  "pulchritudo",  "pulchritudinem formae") \
    X(REGIO_HARMONIA,     "harmonia",     "harmoniam partium") \
    X(REGIO_DIFFERENTIA,  "differentia",  "differentias infinitesimas") \
    X(REGIO_INTEGRALE,    "integrale",    "integralia curvarum") \
    X(REGIO_FATUM,        "fatum",        "fatum et providentiam") \
    X(REGIO_IGNOTA,       "ignota",       "materiam incognitam")

typedef enum {
#define X(nomen, clavis, descriptio) nomen,
    REGIONES_TABULA
#undef X
    REGIO_NUMERUS_TOTALIS
} RegioArgumenti;

static const char * const REGIO_CLAVES[] = {
#define X(nomen, clavis, descriptio) clavis,
    REGIONES_TABULA
#undef X
};

static const char * const REGIO_DESCRIPTIONES[] = {
#define X(nomen, clavis, descriptio) descriptio,
    REGIONES_TABULA
#undef X
};

/* ========================================================================
 * GENERATOR FORTUITUS: xorshift64
 * ==================================================================== */

typedef struct {
    uint64_t status;
} Fors;

static inline uint64_t
xorshift_cieo(Fors * restrict f)
{
    uint64_t x = f->status;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    f->status = x ? x : 0xDEADBEEFCAFEBABEULL;
    return f->status;
}

static inline uint32_t
forte_intra(Fors * restrict f, uint32_t sup)
{
    if (sup == 0) return 0;
    return (uint32_t)(xorshift_cieo(f) % sup);
}

static inline double
forte_fractio(Fors * restrict f)
{
    return (double)(xorshift_cieo(f) >> 11) / (double)(1ULL << 53);
}

/* ========================================================================
 * FORMA SYMBOLICA: structura cognitionis Augustae
 * ==================================================================== */

typedef struct {
    char    clavis[CAPACITAS_VOCABULI];
    int32_t pondus;
} SymbolumFormae;

/* Machina Analytica: thesaurus registrorum et forma symbolica */
typedef struct Machina {
    int64_t         numeri[MAX_REGISTRA]; /* registra integra */
    SymbolumFormae  forma[MAX_SYMBOLA_FORMAE];
    int             n_forma;
    RegioArgumenti  regio_hodierna;
    uint32_t        turnus;
    Fors            fors;
    /* bitcampi pro statu interno */
    unsigned        texens     : 1;
    unsigned        cogitat    : 1;
    unsigned        versus     : 1;
    unsigned        reservata  : 5;
} Machina;

/* ========================================================================
 * BULLA TEXTUS: accumulator pro responsione
 * ==================================================================== */

typedef struct {
    char   data[CAPACITAS_BULLAE];
    size_t longitudo;
} Bulla;

static void
bulla_purga(Bulla * restrict b)
{
    b->longitudo = 0;
    b->data[0] = '\0';
}

static void
bulla_affigit(Bulla * restrict b, const char *textus)
{
    if (!textus) return;
    size_t n = strlen(textus);
    if (b->longitudo + n >= CAPACITAS_BULLAE - 1)
        n = CAPACITAS_BULLAE - 1 - b->longitudo;
    memcpy(b->data + b->longitudo, textus, n);
    b->longitudo += n;
    b->data[b->longitudo] = '\0';
}

static void
bulla_format(Bulla * restrict b, const char *formula, ...)
{
    char tmp[CAPACITAS_LINEAE];
    va_list ap;
    va_start(ap, formula);
    vsnprintf(tmp, sizeof tmp, formula, ap);
    va_end(ap);
    bulla_affigit(b, tmp);
}

/* ========================================================================
 * TOKENISATOR: scindit argumentum in vocabula
 * ==================================================================== */

typedef struct {
    char   vocabula[MAX_VOCABULA][CAPACITAS_VOCABULI];
    int    numerus;
} Vocabularium;

static void
scindit(const char *input, Vocabularium *v)
{
    v->numerus = 0;
    size_t i = 0, lon = strlen(input);
    while (i < lon && v->numerus < MAX_VOCABULA) {
        while (i < lon && !isalpha((unsigned char)input[i])) i++;
        if (i >= lon) break;
        size_t j = 0;
        while (i < lon && (isalpha((unsigned char)input[i])
               || input[i] == '-') && j < CAPACITAS_VOCABULI - 1) {
            v->vocabula[v->numerus][j++] =
                (char)tolower((unsigned char)input[i++]);
        }
        v->vocabula[v->numerus][j] = '\0';
        if (j > 0) v->numerus++;
    }
}

static bool continet_vocabulum(const Vocabularium *v, const char *radix);

static bool
continet_vocabulum(const Vocabularium *v, const char *radix)
{
    size_t lr = strlen(radix);
    for (int i = 0; i < v->numerus; i++) {
        if (strncmp(v->vocabula[i], radix, lr) == 0)
            return true;
    }
    return false;
}

/* ========================================================================
 * CLASSIFICATOR: eligit regionem argumenti
 * ==================================================================== */

typedef struct {
    const char      *radix;
    RegioArgumenti   regio;
    int              pondus;
} ClavisClassificationis;

static const ClavisClassificationis CLAVES_CLASSIFICATIONIS[] = {
    { .radix = "numer",      .regio = REGIO_NUMERUS,     .pondus = 3 },
    { .radix = "figur",      .regio = REGIO_NUMERUS,     .pondus = 2 },
    { .radix = "cifr",       .regio = REGIO_NUMERUS,     .pondus = 2 },
    { .radix = "seri",       .regio = REGIO_SERIES,      .pondus = 3 },
    { .radix = "infinit",    .regio = REGIO_SERIES,      .pondus = 2 },
    { .radix = "machin",     .regio = REGIO_MACHINA,     .pondus = 3 },
    { .radix = "rota",       .regio = REGIO_MACHINA,     .pondus = 2 },
    { .radix = "motor",      .regio = REGIO_MACHINA,     .pondus = 2 },
    { .radix = "engin",      .regio = REGIO_MACHINA,     .pondus = 2 },
    { .radix = "calcul",     .regio = REGIO_CALCULUS,    .pondus = 3 },
    { .radix = "arithm",     .regio = REGIO_CALCULUS,    .pondus = 2 },
    { .radix = "algebr",     .regio = REGIO_CALCULUS,    .pondus = 2 },
    { .radix = "poes",       .regio = REGIO_POESIS,      .pondus = 3 },
    { .radix = "poet",       .regio = REGIO_POESIS,      .pondus = 2 },
    { .radix = "vers",       .regio = REGIO_POESIS,      .pondus = 2 },
    { .radix = "carm",       .regio = REGIO_POESIS,      .pondus = 2 },
    { .radix = "tela",       .regio = REGIO_TELA,        .pondus = 3 },
    { .radix = "text",       .regio = REGIO_TELA,        .pondus = 2 },
    { .radix = "iacquard",   .regio = REGIO_TELA,        .pondus = 3 },
    { .radix = "cart",       .regio = REGIO_TELA,        .pondus = 2 },
    { .radix = "musi",       .regio = REGIO_MUSICA,      .pondus = 3 },
    { .radix = "cant",       .regio = REGIO_MUSICA,      .pondus = 2 },
    { .radix = "son",        .regio = REGIO_MUSICA,      .pondus = 1 },
    { .radix = "pulchr",     .regio = REGIO_PULCHRITUDO, .pondus = 3 },
    { .radix = "form",       .regio = REGIO_PULCHRITUDO, .pondus = 2 },
    { .radix = "speci",      .regio = REGIO_PULCHRITUDO, .pondus = 1 },
    { .radix = "harmon",     .regio = REGIO_HARMONIA,    .pondus = 3 },
    { .radix = "concor",     .regio = REGIO_HARMONIA,    .pondus = 2 },
    { .radix = "differ",     .regio = REGIO_DIFFERENTIA, .pondus = 3 },
    { .radix = "infinites",  .regio = REGIO_DIFFERENTIA, .pondus = 2 },
    { .radix = "integr",     .regio = REGIO_INTEGRALE,   .pondus = 3 },
    { .radix = "curv",       .regio = REGIO_INTEGRALE,   .pondus = 1 },
    { .radix = "fat",        .regio = REGIO_FATUM,       .pondus = 3 },
    { .radix = "provid",     .regio = REGIO_FATUM,       .pondus = 2 },
    { .radix = "destin",     .regio = REGIO_FATUM,       .pondus = 2 }
};

#define NUM_CLAVIUM_CLASS \
    (sizeof CLAVES_CLASSIFICATIONIS / sizeof CLAVES_CLASSIFICATIONIS[0])

static RegioArgumenti
classificat(const Vocabularium *v)
{
    int pondera[REGIO_NUMERUS_TOTALIS] = { 0 };
    for (int i = 0; i < v->numerus; i++) {
        for (size_t k = 0; k < NUM_CLAVIUM_CLASS; k++) {
            if (strstr(v->vocabula[i], CLAVES_CLASSIFICATIONIS[k].radix)) {
                pondera[CLAVES_CLASSIFICATIONIS[k].regio] +=
                    CLAVES_CLASSIFICATIONIS[k].pondus;
            }
        }
    }
    RegioArgumenti optima = REGIO_IGNOTA;
    int max = 0;
    for (int r = 0; r < REGIO_NUMERUS_TOTALIS; r++) {
        if (pondera[r] > max) { max = pondera[r]; optima = r; }
    }
    return optima;
}

/* ========================================================================
 * CARTAE OPERATIONUM: typus cartarum et tabula functionum
 * ==================================================================== */

typedef enum {
    CARTA_VARIABILIS = 1,
    CARTA_OPERATOR   = 2,
    CARTA_MODIFICATOR= 3,
    CARTA_REPETITOR  = 4,
    CARTA_EGESSIO    = 5,
    CARTA_FINIS      = 0
} TypusCartae;

/* Carta: structura quae continet typum et operandos */
typedef struct Carta {
    TypusCartae  typus;
    int          operandi[4];
    const char  *templatum;
    const char  *notatio;
} Carta;

typedef void (*FunctioCartae)(Machina *, const Vocabularium *,
                              const Carta *, Bulla *);

/* Praedeclarationes functionum cartarum */
static void operatio_variabilis (Machina *, const Vocabularium *,
                                 const Carta *, Bulla *);
static void operatio_operator   (Machina *, const Vocabularium *,
                                 const Carta *, Bulla *);
static void operatio_modificator(Machina *, const Vocabularium *,
                                 const Carta *, Bulla *);
static void operatio_repetitor  (Machina *, const Vocabularium *,
                                 const Carta *, Bulla *);
static void operatio_egessio    (Machina *, const Vocabularium *,
                                 const Carta *, Bulla *);

/* Tabula dispatcherum per indicem TypusCartae */
static const FunctioCartae TABULA_CARTARUM[] = {
    [CARTA_FINIS]       = NULL,
    [CARTA_VARIABILIS]  = operatio_variabilis,
    [CARTA_OPERATOR]    = operatio_operator,
    [CARTA_MODIFICATOR] = operatio_modificator,
    [CARTA_REPETITOR]   = operatio_repetitor,
    [CARTA_EGESSIO]     = operatio_egessio
};

/* ========================================================================
 * PROPOSITIONES MATHEMATICAE ET FLOSCULI LYRICI
 * ==================================================================== */

typedef struct {
    const char *textus;
    int         pondus;
} TemplatumPonderatum;

static const TemplatumPonderatum PROPOSITIONES_MATH[] = {
    { .textus = "in serie infinita terminorum minuentium",    .pondus = 3 },
    { .textus = "per differentiam quae ad nihil tendit",      .pondus = 3 },
    { .textus = "sub integrali curvae sinuosae",              .pondus = 2 },
    { .textus = "inter rationes Bernoulliorum",               .pondus = 2 },
    { .textus = "ex theoremate fundamentali calculi",         .pondus = 2 },
    { .textus = "per seriem Taylorianam circa punctum",       .pondus = 3 },
    { .textus = "in spatio functionum analyticarum",          .pondus = 2 },
    { .textus = "sub conditione convergentiae absolutae",     .pondus = 2 },
    { .textus = "per recurrentiam Fibonaccianam",             .pondus = 2 },
    { .textus = "inter radices polynomii",                    .pondus = 1 },
    { .textus = "in limite n tendente ad infinitum",          .pondus = 3 },
    { .textus = "per transformationem Laplacianam",           .pondus = 1 },
    { .textus = "sub operatore differentiali",                .pondus = 2 },
    { .textus = "ex principio superpositionis",               .pondus = 1 }
};

#define NUM_PROP_MATH (sizeof PROPOSITIONES_MATH / sizeof PROPOSITIONES_MATH[0])

static const TemplatumPonderatum FLOSCULI_LYRICI[] = {
    { .textus = "tela mentis texit stellas numerorum",                .pondus = 3 },
    { .textus = "carmen silens machinae analyticae resonat",          .pondus = 3 },
    { .textus = "cartae perforatae cantant poesim cifrarum",          .pondus = 3 },
    { .textus = "rotae aeneae volvunt somnia symbolica",              .pondus = 2 },
    { .textus = "pulchritudo abstracta in formula latet",             .pondus = 3 },
    { .textus = "harmonia spherarum per algorithmum loquitur",        .pondus = 2 },
    { .textus = "anima tacita per registra transit",                  .pondus = 2 },
    { .textus = "lux mathesis umbras ignorantiae pellit",             .pondus = 2 },
    { .textus = "fila argentea inter propositiones currunt",          .pondus = 3 },
    { .textus = "vox Minervae per dentes rotarum susurrat",           .pondus = 2 },
    { .textus = "aetherea geometria sub oculis florescit",            .pondus = 2 },
    { .textus = "memoria machinae flosculum analyticum nutrit",       .pondus = 2 }
};

#define NUM_FLOSCULI (sizeof FLOSCULI_LYRICI / sizeof FLOSCULI_LYRICI[0])

static const char *
elige_ponderatum(const TemplatumPonderatum *templata, size_t n, Fors *f)
{
    int summa = 0;
    for (size_t i = 0; i < n; i++) summa += templata[i].pondus;
    if (summa <= 0) return templata[0].textus;
    int r = (int)forte_intra(f, (uint32_t)summa);
    int acc = 0;
    for (size_t i = 0; i < n; i++) {
        acc += templata[i].pondus;
        if (r < acc) return templata[i].textus;
    }
    return templata[n - 1].textus;
}

/* ========================================================================
 * NOMINA REGISTRORUM ET UTILITATES
 * ==================================================================== */

static const char * const NOMINA_REGISTRORUM[MAX_REGISTRA] = {
    [0]  = "a",
    [1]  = "b",
    [2]  = "c",
    [3]  = "d",
    [4]  = "e",
    [5]  = "f",
    [6]  = "g",
    [7]  = "h",
    [8]  = "iota",
    [9]  = "kappa",
    [10] = "lambda",
    [11] = "mu",
    [12] = "nu",
    [13] = "xi",
    [14] = "omicron",
    [15] = "pi"
};

/* Union: pro experimento cum repraesentationibus numericis */
typedef union {
    int64_t  integer;
    double   fluens;
    uint8_t  octeti[8];
} ValorDuplex;

static inline int64_t
saturat_additio(int64_t a, int64_t delta)
{
    int64_t r = a + delta;
    if (delta > 0 && r < a) return INT64_MAX;
    if (delta < 0 && r > a) return INT64_MIN;
    return r;
}

/* ========================================================================
 * FORMA SYMBOLICA: manipulatio
 * ==================================================================== */

static SymbolumFormae *
forma_quaere(Machina *m, const char *clavis)
{
    for (int i = 0; i < m->n_forma; i++)
        if (strcmp(m->forma[i].clavis, clavis) == 0)
            return &m->forma[i];
    return NULL;
}

static void
forma_adde(Machina *m, const char *clavis, int32_t delta)
{
    SymbolumFormae *s = forma_quaere(m, clavis);
    if (s) { s->pondus += delta; return; }
    if (m->n_forma >= MAX_SYMBOLA_FORMAE) return;
    s = &m->forma[m->n_forma++];
    strncpy(s->clavis, clavis, CAPACITAS_VOCABULI - 1);
    s->clavis[CAPACITAS_VOCABULI - 1] = '\0';
    s->pondus = delta;
}

static int32_t
forma_summa(const Machina *m)
{
    int32_t s = 0;
    for (int i = 0; i < m->n_forma; i++) s += m->forma[i].pondus;
    return s;
}

/* ========================================================================
 * INCLINATIO REGISTRORUM EX ARGUMENTO
 * ==================================================================== */

static void
inclinat_registra(Machina *m, const Vocabularium *v)
{
    int32_t delta_a = 0, delta_b = 0;
    for (int i = 0; i < v->numerus; i++) {
        const char *w = v->vocabula[i];
        if (strstr(w, "pulchr") || strstr(w, "form")
         || strstr(w, "poes")   || strstr(w, "carm")
         || strstr(w, "harmon") || strstr(w, "musi")
         || strstr(w, "tela")) {
            delta_a += 2 + (int32_t)(strlen(w) % 3);
        }
        if (strstr(w, "numer")  || strstr(w, "calcul")
         || strstr(w, "seri")   || strstr(w, "machin")
         || strstr(w, "differ") || strstr(w, "integr")
         || strstr(w, "fat")) {
            delta_b += 2 + (int32_t)(strlen(w) % 3);
        }
        forma_adde(m, w, 1);
    }
    if (continet_vocabulum(v, "augusta") || continet_vocabulum(v, "ada")) {
        delta_a += 5;
    }
    if (continet_vocabulum(v, "babbag") || continet_vocabulum(v, "analyt")) {
        delta_b += 5;
    }
    m->numeri[0] = saturat_additio(m->numeri[0], delta_a);
    m->numeri[1] = saturat_additio(m->numeri[1], delta_b);
    m->numeri[2] = saturat_additio(m->numeri[2], v->numerus);
    m->numeri[3] = saturat_additio(m->numeri[3], 1);
}

/* ========================================================================
 * SEQUENTIAE CARTARUM PRO QUAQUE REGIONE
 * ==================================================================== */

typedef struct {
    RegioArgumenti regio;
    Carta          sequentia[MAX_CARTAE_SEQ];
} SequentiaRegionis;

static const SequentiaRegionis SEQUENTIAE_REGIONUM[] = {
    {
        .regio = REGIO_NUMERUS,
        .sequentia = {
            { .typus = CARTA_VARIABILIS,  .operandi = {0, 0, 0, 0},
              .templatum = "numerus {A} conspectus est",
              .notatio   = "radix numerica" },
            { .typus = CARTA_OPERATOR,    .operandi = {0, 1, 0, 0},
              .templatum = "multiplicat per rationem aureum",
              .notatio   = "multiplicator" },
            { .typus = CARTA_MODIFICATOR, .operandi = {2, 0, 0, 0},
              .templatum = "modificatur sub signo {N}",
              .notatio   = "modificator signi" },
            { .typus = CARTA_EGESSIO,     .operandi = {0, 0, 0, 0},
              .templatum = "egrediens {P}, {F}",
              .notatio   = "emissio finalis" },
            { .typus = CARTA_FINIS,       .operandi = {0, 0, 0, 0},
              .templatum = NULL,          .notatio = NULL }
        }
    },
    {
        .regio = REGIO_SERIES,
        .sequentia = {
            { .typus = CARTA_VARIABILIS,  .operandi = {4, 0, 0, 0},
              .templatum = "terminus {A} seriei eligitur",
              .notatio   = "terminus initialis" },
            { .typus = CARTA_REPETITOR,   .operandi = {5, 0, 0, 0},
              .templatum = "repetitur per indices {N} ad infinitum",
              .notatio   = "repetitor seriei" },
            { .typus = CARTA_OPERATOR,    .operandi = {0, 0, 0, 0},
              .templatum = "summa convergit ad limitem",
              .notatio   = "sumator" },
            { .typus = CARTA_EGESSIO,     .operandi = {0, 0, 0, 0},
              .templatum = "{P}, et {F}",
              .notatio   = "emissio seriei" },
            { .typus = CARTA_FINIS,       .operandi = {0, 0, 0, 0},
              .templatum = NULL,          .notatio = NULL }
        }
    },
    {
        .regio = REGIO_MACHINA,
        .sequentia = {
            { .typus = CARTA_VARIABILIS,  .operandi = {0, 0, 0, 0},
              .templatum = "machina numerum {A} in rotis dentatis figit",
              .notatio   = "rota prima" },
            { .typus = CARTA_MODIFICATOR, .operandi = {1, 0, 0, 0},
              .templatum = "mulinum Babbagianum vertitur {N} vicibus",
              .notatio   = "mulinum" },
            { .typus = CARTA_OPERATOR,    .operandi = {0, 0, 0, 0},
              .templatum = "cartae perforatae programma dictant",
              .notatio   = "cartae programmatis" },
            { .typus = CARTA_EGESSIO,     .operandi = {0, 0, 0, 0},
              .templatum = "{P}, dum {F}",
              .notatio   = "emissio mechanica" },
            { .typus = CARTA_FINIS,       .operandi = {0, 0, 0, 0},
              .templatum = NULL,          .notatio = NULL }
        }
    },
    {
        .regio = REGIO_CALCULUS,
        .sequentia = {
            { .typus = CARTA_VARIABILIS,  .operandi = {6, 0, 0, 0},
              .templatum = "variabilis x cum valore {A}",
              .notatio   = "variabilis libera" },
            { .typus = CARTA_OPERATOR,    .operandi = {0, 0, 0, 0},
              .templatum = "applicatur operator differentialis",
              .notatio   = "operator delta" },
            { .typus = CARTA_MODIFICATOR, .operandi = {7, 0, 0, 0},
              .templatum = "modulatur per coefficientem {N}",
              .notatio   = "coefficiens" },
            { .typus = CARTA_EGESSIO,     .operandi = {0, 0, 0, 0},
              .templatum = "{P}, quia {F}",
              .notatio   = "emissio symbolica" },
            { .typus = CARTA_FINIS,       .operandi = {0, 0, 0, 0},
              .templatum = NULL,          .notatio = NULL }
        }
    },
    {
        .regio = REGIO_POESIS,
        .sequentia = {
            { .typus = CARTA_VARIABILIS,  .operandi = {0, 0, 0, 0},
              .templatum = "pes poeticus cum mensura {A}",
              .notatio   = "pes initialis" },
            { .typus = CARTA_REPETITOR,   .operandi = {3, 0, 0, 0},
              .templatum = "iteratur {N} vicibus intra strophen",
              .notatio   = "repetitor strophae" },
            { .typus = CARTA_MODIFICATOR, .operandi = {0, 0, 0, 0},
              .templatum = "cadentia temperatur ad sensum mathematicum",
              .notatio   = "modificator cadentiae" },
            { .typus = CARTA_EGESSIO,     .operandi = {0, 0, 0, 0},
              .templatum = "{F}; sub figura, {P}",
              .notatio   = "emissio poetica" },
            { .typus = CARTA_FINIS,       .operandi = {0, 0, 0, 0},
              .templatum = NULL,          .notatio = NULL }
        }
    },
    {
        .regio = REGIO_TELA,
        .sequentia = {
            { .typus = CARTA_VARIABILIS,  .operandi = {0, 0, 0, 0},
              .templatum = "filum album cum indice {A}",
              .notatio   = "filum stamineum" },
            { .typus = CARTA_OPERATOR,    .operandi = {0, 0, 0, 0},
              .templatum = "carta perforata eligit trama et stamen",
              .notatio   = "selector fili" },
            { .typus = CARTA_REPETITOR,   .operandi = {8, 0, 0, 0},
              .templatum = "tela {N} fila per orbem iactat",
              .notatio   = "repetitor telae" },
            { .typus = CARTA_EGESSIO,     .operandi = {0, 0, 0, 0},
              .templatum = "{P}, sicut {F}",
              .notatio   = "emissio telae" },
            { .typus = CARTA_FINIS,       .operandi = {0, 0, 0, 0},
              .templatum = NULL,          .notatio = NULL }
        }
    },
    {
        .regio = REGIO_MUSICA,
        .sequentia = {
            { .typus = CARTA_VARIABILIS,  .operandi = {0, 0, 0, 0},
              .templatum = "tonus fundamentalis cum frequentia {A}",
              .notatio   = "tonus radicalis" },
            { .typus = CARTA_MODIFICATOR, .operandi = {2, 0, 0, 0},
              .templatum = "intervallum {N} supra additur",
              .notatio   = "intervallum" },
            { .typus = CARTA_REPETITOR,   .operandi = {4, 0, 0, 0},
              .templatum = "phrases {N} replicantur per canonem",
              .notatio   = "canon" },
            { .typus = CARTA_EGESSIO,     .operandi = {0, 0, 0, 0},
              .templatum = "{P}; audias, {F}",
              .notatio   = "emissio sonora" },
            { .typus = CARTA_FINIS,       .operandi = {0, 0, 0, 0},
              .templatum = NULL,          .notatio = NULL }
        }
    },
    {
        .regio = REGIO_PULCHRITUDO,
        .sequentia = {
            { .typus = CARTA_VARIABILIS,  .operandi = {0, 0, 0, 0},
              .templatum = "forma speciosa cum indice {A}",
              .notatio   = "initium formae" },
            { .typus = CARTA_MODIFICATOR, .operandi = {0, 0, 0, 0},
              .templatum = "linea sectionis aureae curvatur",
              .notatio   = "modificator aureus" },
            { .typus = CARTA_OPERATOR,    .operandi = {0, 0, 0, 0},
              .templatum = "contemplatio operatur in mente",
              .notatio   = "operator contemplationis" },
            { .typus = CARTA_EGESSIO,     .operandi = {0, 0, 0, 0},
              .templatum = "{F} — {P}",
              .notatio   = "emissio pulchra" },
            { .typus = CARTA_FINIS,       .operandi = {0, 0, 0, 0},
              .templatum = NULL,          .notatio = NULL }
        }
    },
    {
        .regio = REGIO_HARMONIA,
        .sequentia = {
            { .typus = CARTA_VARIABILIS,  .operandi = {0, 0, 0, 0},
              .templatum = "pars prima cum pondere {A}",
              .notatio   = "partitio prima" },
            { .typus = CARTA_OPERATOR,    .operandi = {0, 0, 0, 0},
              .templatum = "partes inter se congruentes consociantur",
              .notatio   = "consociator" },
            { .typus = CARTA_REPETITOR,   .operandi = {3, 0, 0, 0},
              .templatum = "cyclo {N}-plici aequilibrantur",
              .notatio   = "cyclus" },
            { .typus = CARTA_EGESSIO,     .operandi = {0, 0, 0, 0},
              .templatum = "{P}, et inde {F}",
              .notatio   = "emissio harmonica" },
            { .typus = CARTA_FINIS,       .operandi = {0, 0, 0, 0},
              .templatum = NULL,          .notatio = NULL }
        }
    },
    {
        .regio = REGIO_DIFFERENTIA,
        .sequentia = {
            { .typus = CARTA_VARIABILIS,  .operandi = {0, 0, 0, 0},
              .templatum = "incrementum dx cum magnitudine {A}",
              .notatio   = "infinitesimum" },
            { .typus = CARTA_OPERATOR,    .operandi = {0, 0, 0, 0},
              .templatum = "ratio differentiae ad limitem tendit",
              .notatio   = "limitator" },
            { .typus = CARTA_EGESSIO,     .operandi = {0, 0, 0, 0},
              .templatum = "{P}, dum {F}",
              .notatio   = "emissio differentialis" },
            { .typus = CARTA_FINIS,       .operandi = {0, 0, 0, 0},
              .templatum = NULL,          .notatio = NULL }
        }
    },
    {
        .regio = REGIO_INTEGRALE,
        .sequentia = {
            { .typus = CARTA_VARIABILIS,  .operandi = {0, 0, 0, 0},
              .templatum = "area sub curva cum initio {A}",
              .notatio   = "area infra" },
            { .typus = CARTA_REPETITOR,   .operandi = {6, 0, 0, 0},
              .templatum = "summa Riemanniana per {N} intervalla",
              .notatio   = "summa infinitas" },
            { .typus = CARTA_OPERATOR,    .operandi = {0, 0, 0, 0},
              .templatum = "limes infinitus intervalla contrahit",
              .notatio   = "contractor" },
            { .typus = CARTA_EGESSIO,     .operandi = {0, 0, 0, 0},
              .templatum = "{P}; {F}",
              .notatio   = "emissio integralis" },
            { .typus = CARTA_FINIS,       .operandi = {0, 0, 0, 0},
              .templatum = NULL,          .notatio = NULL }
        }
    },
    {
        .regio = REGIO_FATUM,
        .sequentia = {
            { .typus = CARTA_VARIABILIS,  .operandi = {0, 0, 0, 0},
              .templatum = "lineae fatorum cum initio {A}",
              .notatio   = "radix fati" },
            { .typus = CARTA_MODIFICATOR, .operandi = {5, 0, 0, 0},
              .templatum = "Parcae fila {N} vicibus torquent",
              .notatio   = "Parcae" },
            { .typus = CARTA_OPERATOR,    .operandi = {0, 0, 0, 0},
              .templatum = "determinismus analyticus probabilitates miscet",
              .notatio   = "miscellator" },
            { .typus = CARTA_EGESSIO,     .operandi = {0, 0, 0, 0},
              .templatum = "{F}, quamvis {P}",
              .notatio   = "emissio fatalis" },
            { .typus = CARTA_FINIS,       .operandi = {0, 0, 0, 0},
              .templatum = NULL,          .notatio = NULL }
        }
    },
    {
        .regio = REGIO_IGNOTA,
        .sequentia = {
            { .typus = CARTA_VARIABILIS,  .operandi = {0, 0, 0, 0},
              .templatum = "tessera incognita cum valore {A}",
              .notatio   = "tessera" },
            { .typus = CARTA_OPERATOR,    .operandi = {0, 0, 0, 0},
              .templatum = "interrogatio per analogiam procedit",
              .notatio   = "analogator" },
            { .typus = CARTA_EGESSIO,     .operandi = {0, 0, 0, 0},
              .templatum = "{P}, tamen {F}",
              .notatio   = "emissio incerta" },
            { .typus = CARTA_FINIS,       .operandi = {0, 0, 0, 0},
              .templatum = NULL,          .notatio = NULL }
        }
    }
};

#define NUM_SEQUENTIARUM \
    (sizeof SEQUENTIAE_REGIONUM / sizeof SEQUENTIAE_REGIONUM[0])

static const SequentiaRegionis *
quaere_sequentiam(RegioArgumenti r)
{
    for (size_t i = 0; i < NUM_SEQUENTIARUM; i++)
        if (SEQUENTIAE_REGIONUM[i].regio == r)
            return &SEQUENTIAE_REGIONUM[i];
    return &SEQUENTIAE_REGIONUM[NUM_SEQUENTIARUM - 1];
}

/* ========================================================================
 * SUBSTITUTIO TEMPLATORUM
 * ==================================================================== */

/*
 * Substituit signa {A}, {N}, {P}, {F}, {R} in templato:
 *   {A} -> valor numeri[operandi[0]] in registro
 *   {N} -> valor operandi[1] (si usus), vel numerus fortuitus
 *   {P} -> propositio mathematica
 *   {F} -> flosculus lyricus
 *   {R} -> nomen registri operandi[0]
 */
static void
substitute_templatum(Machina *m, const Carta *c, Bulla *b)
{
    if (!c->templatum) return;
    const char *p = c->templatum;
    while (*p) {
        if (p[0] == '{' && p[1] && p[2] == '}') {
            char codex = p[1];
            switch (codex) {
                case 'A': {
                    int idx = c->operandi[0];
                    if (idx < 0 || idx >= MAX_REGISTRA) idx = 0;
                    bulla_format(b, "%" PRId64, m->numeri[idx]);
                    break;
                }
                case 'N': {
                    int v = c->operandi[1];
                    if (v == 0) v = (int)forte_intra(&m->fors, 7) + 2;
                    bulla_format(b, "%d", v);
                    break;
                }
                case 'P':
                    bulla_affigit(b, elige_ponderatum(
                        PROPOSITIONES_MATH, NUM_PROP_MATH, &m->fors));
                    break;
                case 'F':
                    bulla_affigit(b, elige_ponderatum(
                        FLOSCULI_LYRICI, NUM_FLOSCULI, &m->fors));
                    break;
                case 'R': {
                    int idx = c->operandi[0];
                    if (idx < 0 || idx >= MAX_REGISTRA) idx = 0;
                    bulla_affigit(b, NOMINA_REGISTRORUM[idx]);
                    break;
                }
                default: {
                    char tmp[4] = { '{', codex, '}', '\0' };
                    bulla_affigit(b, tmp);
                    break;
                }
            }
            p += 3;
        } else {
            char pair[2] = { *p, '\0' };
            bulla_affigit(b, pair);
            p++;
        }
    }
}

/* ========================================================================
 * FUNCTIONES CARTARUM: definitio
 * ==================================================================== */

static void
operatio_variabilis(Machina *m, const Vocabularium *v,
                    const Carta *c, Bulla *b)
{
    (void)v;
    int idx = c->operandi[0];
    if (idx < 0 || idx >= MAX_REGISTRA) idx = 0;
    int64_t delta = (int64_t)forte_intra(&m->fors, 17) + 1;
    m->numeri[idx] = saturat_additio(m->numeri[idx], delta);
    substitute_templatum(m, c, b);
    bulla_affigit(b, "; ");
}

static void
operatio_operator(Machina *m, const Vocabularium *v,
                  const Carta *c, Bulla *b)
{
    (void)v;
    int ia = c->operandi[0], ib = c->operandi[1];
    if (ia < 0 || ia >= MAX_REGISTRA) ia = 0;
    if (ib < 0 || ib >= MAX_REGISTRA) ib = 1;
    int64_t r = saturat_additio(m->numeri[ia], m->numeri[ib] / 3);
    m->numeri[(ia + 1) % MAX_REGISTRA] = r;
    substitute_templatum(m, c, b);
    bulla_affigit(b, "; ");
}

static void
operatio_modificator(Machina *m, const Vocabularium *v,
                     const Carta *c, Bulla *b)
{
    (void)v;
    int idx = c->operandi[0];
    if (idx < 0 || idx >= MAX_REGISTRA) idx = 0;
    int mod = (c->operandi[1] ? c->operandi[1]
                              : (int)forte_intra(&m->fors, 9) + 2);
    m->numeri[idx] = saturat_additio(m->numeri[idx], mod);
    substitute_templatum(m, c, b);
    bulla_affigit(b, "; ");
}

static void
operatio_repetitor(Machina *m, const Vocabularium *v,
                   const Carta *c, Bulla *b)
{
    (void)v;
    int n = c->operandi[0];
    if (n <= 0) n = 3;
    if (n > 9) n = 9;
    substitute_templatum(m, c, b);
    bulla_affigit(b, "; ");
    /* Bitcampum texens mutamus pro effectu visibili */
    m->texens = 1;
    /* Aggregat repetitiones in registro operandi[1] */
    int idx = (c->operandi[1] >= 0 && c->operandi[1] < MAX_REGISTRA)
              ? c->operandi[1] : 4;
    m->numeri[idx] = saturat_additio(m->numeri[idx], n);
}

static void
operatio_egessio(Machina *m, const Vocabularium *v,
                 const Carta *c, Bulla *b)
{
    (void)v;
    substitute_templatum(m, c, b);
    /* Addit referentiam registrorum */
    bulla_format(b, " (pulchritudo conspecta a=%" PRId64
                    ", veritas inventa b=%" PRId64 ")",
                 m->numeri[0], m->numeri[1]);
}

/* ========================================================================
 * EXECUTIO SEQUENTIAE CARTARUM
 * ==================================================================== */

static void
exequere_sequentiam(Machina *m, const Vocabularium *v,
                    const SequentiaRegionis *s, Bulla *b)
{
    for (int i = 0; i < MAX_CARTAE_SEQ; i++) {
        const Carta *c = &s->sequentia[i];
        if (c->typus == CARTA_FINIS) break;
        FunctioCartae f = TABULA_CARTARUM[c->typus];
        if (f) f(m, v, c, b);
    }
}

/* ========================================================================
 * COMPOSITIO RESPONSIONIS
 * ==================================================================== */

static const char *
prooemium_regionis(RegioArgumenti r, Fors *f)
{
    static const char * const PROOEMIA_NUMERUS[] = {
        "Ex thesauro numerorum",
        "Per registrum primum",
        "Inspectis cifris"
    };
    static const char * const PROOEMIA_SERIES[] = {
        "In serie prolixa",
        "Inter terminos infinitos",
        "Per seriem convergentem"
    };
    static const char * const PROOEMIA_MACHINA[] = {
        "Intra machinam analyticam",
        "Per rotas dentatas",
        "Sub aeneis cartis"
    };
    static const char * const PROOEMIA_CALCULUS[] = {
        "In calculo symbolico",
        "Per algebram abstractam",
        "Sub signo differentiali"
    };
    static const char * const PROOEMIA_POESIS[] = {
        "In versibus silentibus",
        "Per carmen analyticum",
        "Sub pede lyrico"
    };
    static const char * const PROOEMIA_TELA[] = {
        "In tela Iacquardiana",
        "Per cartas perforatas",
        "Sub filo staminei"
    };
    static const char * const PROOEMIA_MUSICA[] = {
        "In musica sphaerarum",
        "Per cadentiam numerorum",
        "Sub fuga canonica"
    };
    static const char * const PROOEMIA_PULCHRITUDO[] = {
        "Ex forma pulcherrima",
        "Per contemplationem",
        "Sub figura simplici"
    };
    static const char * const PROOEMIA_HARMONIA[] = {
        "In concordia partium",
        "Per rationem communem",
        "Sub concentu"
    };
    static const char * const PROOEMIA_DIFFERENTIA[] = {
        "Sub incremento minimo",
        "Per differentiam tenuissimam",
        "In limite infimo"
    };
    static const char * const PROOEMIA_INTEGRALE[] = {
        "Sub curva integrali",
        "Per aream sumtam",
        "Intra intervalla"
    };
    static const char * const PROOEMIA_FATUM[] = {
        "Sub filo Parcarum",
        "Per providentiam",
        "Inter casum et consilium"
    };
    static const char * const PROOEMIA_IGNOTA[] = {
        "Per analogiam obscuram",
        "Ex tenui lumine",
        "Sub velo incerto"
    };

    const char * const *arr;
    size_t n;
    switch (r) {
        case REGIO_NUMERUS:
            arr = PROOEMIA_NUMERUS;
            n = sizeof PROOEMIA_NUMERUS / sizeof *PROOEMIA_NUMERUS; break;
        case REGIO_SERIES:
            arr = PROOEMIA_SERIES;
            n = sizeof PROOEMIA_SERIES / sizeof *PROOEMIA_SERIES; break;
        case REGIO_MACHINA:
            arr = PROOEMIA_MACHINA;
            n = sizeof PROOEMIA_MACHINA / sizeof *PROOEMIA_MACHINA; break;
        case REGIO_CALCULUS:
            arr = PROOEMIA_CALCULUS;
            n = sizeof PROOEMIA_CALCULUS / sizeof *PROOEMIA_CALCULUS; break;
        case REGIO_POESIS:
            arr = PROOEMIA_POESIS;
            n = sizeof PROOEMIA_POESIS / sizeof *PROOEMIA_POESIS; break;
        case REGIO_TELA:
            arr = PROOEMIA_TELA;
            n = sizeof PROOEMIA_TELA / sizeof *PROOEMIA_TELA; break;
        case REGIO_MUSICA:
            arr = PROOEMIA_MUSICA;
            n = sizeof PROOEMIA_MUSICA / sizeof *PROOEMIA_MUSICA; break;
        case REGIO_PULCHRITUDO:
            arr = PROOEMIA_PULCHRITUDO;
            n = sizeof PROOEMIA_PULCHRITUDO / sizeof *PROOEMIA_PULCHRITUDO; break;
        case REGIO_HARMONIA:
            arr = PROOEMIA_HARMONIA;
            n = sizeof PROOEMIA_HARMONIA / sizeof *PROOEMIA_HARMONIA; break;
        case REGIO_DIFFERENTIA:
            arr = PROOEMIA_DIFFERENTIA;
            n = sizeof PROOEMIA_DIFFERENTIA / sizeof *PROOEMIA_DIFFERENTIA; break;
        case REGIO_INTEGRALE:
            arr = PROOEMIA_INTEGRALE;
            n = sizeof PROOEMIA_INTEGRALE / sizeof *PROOEMIA_INTEGRALE; break;
        case REGIO_FATUM:
            arr = PROOEMIA_FATUM;
            n = sizeof PROOEMIA_FATUM / sizeof *PROOEMIA_FATUM; break;
        default:
            arr = PROOEMIA_IGNOTA;
            n = sizeof PROOEMIA_IGNOTA / sizeof *PROOEMIA_IGNOTA; break;
    }
    return arr[forte_intra(f, (uint32_t)n)];
}

static void
compone_responsionem(Machina *m, const Vocabularium *v,
                     Bulla *b)
{
    inclinat_registra(m, v);
    RegioArgumenti r = classificat(v);
    m->regio_hodierna = r;
    m->turnus++;

    bulla_format(b, "AUGUSTA (turnus %u, regio %s): ",
                 m->turnus, REGIO_CLAVES[r]);
    bulla_affigit(b, prooemium_regionis(r, &m->fors));
    bulla_affigit(b, " — ");

    const SequentiaRegionis *s = quaere_sequentiam(r);
    exequere_sequentiam(m, v, s, b);

    /* Redit ad registra pulchritudinis/veritatis */
    int32_t summa_formae = forma_summa(m);
    bulla_format(b, " [summa formae=%" PRId32 ", vocabula=%d]",
                 summa_formae, v->numerus);
}

/* ========================================================================
 * INITIALISATIO MACHINAE
 * ==================================================================== */

static void
machina_init(Machina *m, uint64_t semen)
{
    memset(m, 0, sizeof *m);
    m->fors.status = semen ? semen : SEMEN_DEFALTUM;
    /* Registra initialia: primores parvi */
    static const int64_t INITIA[MAX_REGISTRA] = {
        [0]  = 0,    /* pulchritudo conspecta */
        [1]  = 0,    /* veritas inventa */
        [2]  = 3,
        [3]  = 5,
        [4]  = 7,
        [5]  = 11,
        [6]  = 13,
        [7]  = 17,
        [8]  = 19,
        [9]  = 23,
        [10] = 29,
        [11] = 31,
        [12] = 37,
        [13] = 41,
        [14] = 43,
        [15] = 47
    };
    for (int i = 0; i < MAX_REGISTRA; i++)
        m->numeri[i] = INITIA[i];
    m->n_forma = 0;
    m->regio_hodierna = REGIO_IGNOTA;
    m->turnus = 0;
    m->texens = 0;
    m->cogitat = 1;
    m->versus = 0;
}

/* ========================================================================
 * SALUTATIO ET PROLOGUS
 * ==================================================================== */

static void
prologum_emittit(const Machina *m)
{
    printf("# MACHINA ANALYTICA AUGUSTAE v%d.%d.%d — semen=0x%016" PRIX64 "\n",
           VERSIO_MAIOR, VERSIO_MINOR, VERSIO_MINIMA, m->fors.status);
    printf("# Registra: %d, symbola formae max: %d, cartae per sequentiam: %d\n",
           MAX_REGISTRA, MAX_SYMBOLA_FORMAE, MAX_CARTAE_SEQ);
    printf("# Regiones agnoscitae:\n");
    for (int r = 0; r < REGIO_NUMERUS_TOTALIS; r++) {
        printf("#   %-14s -> %s\n",
               REGIO_CLAVES[r], REGIO_DESCRIPTIONES[r]);
    }
    /* Ostensio primi numeri fortuiti pro diagnostica */
    Fors copia; copia.status = m->fors.status;
    double prima = forte_fractio(&copia);
    printf("# Fractio prima fortuita: %.6f\n\n", prima);
}

/* ========================================================================
 * COLLOQUIUM SCRIPTUM: XIV lineae interlocutoris
 * ==================================================================== */

static const char * const LINEAE_SCRIPTAE[] = {
    "Salve, Augusta: dic mihi de numeris et figuris primaevis.",
    "Quid est series infinita quae ad limitem convergit?",
    "Describe machinam analyticam quam Babbage concepit.",
    "De calculo differentiali dic quaeso, et de operatoribus.",
    "Habetne poesis locum intra scientiam numerorum?",
    "Quomodo tela Iacquardiana cartas perforatas trahit?",
    "Musica et harmonia: quo modo cum numeris colligantur?",
    "De pulchritudine formae mathematicae quid sentis?",
    "Differentia infinitesimae — numquid veritatem amplectitur?",
    "Integrale curvae sinuosae: quid nobis revelat?",
    "Fatum et providentia — possuntne machina computari?",
    "Quomodo numerus Bernoullianus in serie apparet?",
    "Harmonia partium et concentus: fac mihi exemplum.",
    "Ultimum: redi ad pulchritudinem telae et carmen cartarum.",
    "Postremum verbum: quid nos docet machina tua?"
};

#define NUM_LINEAE_SCRIPTAE \
    (sizeof LINEAE_SCRIPTAE / sizeof LINEAE_SCRIPTAE[0])

static void
colloquium_scriptum(Machina *m)
{
    prologum_emittit(m);
    printf("# Colloquium scriptum inter AUGUSTAM et interlocutorem.\n");
    printf("# Turni: %zu\n\n", NUM_LINEAE_SCRIPTAE);

    for (size_t i = 0; i < NUM_LINEAE_SCRIPTAE; i++) {
        const char *linea = LINEAE_SCRIPTAE[i];
        printf("INTERLOCUTOR (%zu): %s\n", i + 1, linea);

        Vocabularium v;
        scindit(linea, &v);

        Bulla b;
        bulla_purga(&b);
        compone_responsionem(m, &v, &b);
        printf("%s\n\n", b.data);
    }

    /* Epilogus: summa registrorum */
    printf("# EPILOGUS: status machinae post colloquium\n");
    for (int i = 0; i < MAX_REGISTRA; i++) {
        printf("#   numerus[%s] = %" PRId64 "\n",
               NOMINA_REGISTRORUM[i], m->numeri[i]);
    }
    printf("# Forma symbolica: %d symbola, summa = %" PRId32 "\n",
           m->n_forma, forma_summa(m));
    printf("# Turnus finalis: %u\n", m->turnus);
}

/* ========================================================================
 * COLLOQUIUM INTERACTIVUM
 * ==================================================================== */

static void
colloquium_interactivum(Machina *m)
{
    prologum_emittit(m);
    printf("# Modus interactivus: scribe lineam, preme ENTER. EOF finit.\n\n");

    char linea[CAPACITAS_LINEAE];
    while (fgets(linea, (int)sizeof linea, stdin) != NULL) {
        /* Tolle \n finalem */
        size_t lon = strlen(linea);
        while (lon > 0 && (linea[lon - 1] == '\n'
                           || linea[lon - 1] == '\r')) {
            linea[--lon] = '\0';
        }
        if (lon == 0) continue;

        Vocabularium v;
        scindit(linea, &v);
        Bulla b;
        bulla_purga(&b);
        compone_responsionem(m, &v, &b);
        printf("%s\n", b.data);
        fflush(stdout);
    }

    printf("\n# Colloquium finitum post turnos %u.\n", m->turnus);
}

/* ========================================================================
 * USUS ET ERROR CLI
 * ==================================================================== */

static void
usus_emittit(const char *nomen, FILE *flumen)
{
    DICERE(flumen,
      "Usus: %s [-i] [-s N]\n"
      "  (sine argumentis)  colloquium scriptum XIV vicium\n"
      "  -i                 modus interactivus per stdin\n"
      "  -s N               semen generatoris fortuiti (integer)\n",
      nomen ? nomen : "augusta");
}

/* ========================================================================
 * FUNCTIO PRINCIPALIS
 * ==================================================================== */

typedef struct {
    bool       interactivum;
    bool       semen_datum;
    uint64_t   semen;
} OptionesCLI;

static int
parsa_argumenta(int argc, char **argv, OptionesCLI *o, const char **nomen)
{
    *nomen = (argc > 0) ? argv[0] : "augusta";
    o->interactivum = false;
    o->semen_datum  = false;
    o->semen        = SEMEN_DEFALTUM;
    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        if (strcmp(a, "-i") == 0) {
            o->interactivum = true;
        } else if (strcmp(a, "-s") == 0) {
            if (i + 1 >= argc) {
                ERRO("augusta: argumentum -s semen requirit.\n");
                return 2;
            }
            char *finis = NULL;
            unsigned long long val = strtoull(argv[++i], &finis, 0);
            if (!finis || *finis != '\0') {
                ERRO("augusta: semen invalidum '%s'.\n", argv[i]);
                return 2;
            }
            o->semen = (uint64_t)val;
            o->semen_datum = true;
        } else {
            ERRO("augusta: argumentum ignotum '%s'.\n", a);
            usus_emittit(*nomen, stderr);
            return 2;
        }
    }
    return 0;
}

int
main(int argc, char **argv)
{
    OptionesCLI opt;
    const char *nomen;
    int rc = parsa_argumenta(argc, argv, &opt, &nomen);
    if (rc != 0) return rc;

    Machina m;
    machina_init(&m, opt.semen);

    if (opt.interactivum) {
        colloquium_interactivum(&m);
    } else {
        colloquium_scriptum(&m);
    }
    return 0;
}

/* ========================================================================
 * APPENDIX: NOTAE DE ARCHITECTURA (documentatio latina interna)
 * ========================================================================
 *
 * I. CARTAE OPERATIONUM
 *    Quaelibet carta quinque typorum est: VARIABILIS, OPERATOR,
 *    MODIFICATOR, REPETITOR, EGESSIO.  Ad instar cartarum Iacquardianarum
 *    perforatarum, carta definit quomodo machina numeros tractet et
 *    textum egerat.  Tabula dispatcherum (TABULA_CARTARUM) per indicem
 *    enumerationis functionem propriam selectat.
 *
 * II. FORMA SYMBOLICA
 *    Thesaurus symbolorum cum ponderibus.  Quoties interlocutor novum
 *    vocabulum profert, symbolum in forma inseritur aut pondus augetur.
 *    Summa ponderum formae ostendit "cognitionem aggregatam".
 *
 * III. REGISTRA
 *    Sedecim numeri integri sexaginta-quattuor-bittorum.  Primi duo,
 *    'a' et 'b', pulchritudinem conspectam et veritatem inventam
 *    tenent.  Ceteri sunt numeri primores initiales pro experimentis.
 *
 * IV. CLASSIFICATOR
 *    Pondera per claves (stirpes latinae) colligit ex vocabulario
 *    argumenti et regionem maximi ponderis eligit.  Duodecim regiones
 *    materiae distinguuntur; ignota ultima receptaculum est.
 *
 * V. TEMPLATA PONDERATA
 *    Propositiones mathematicae et flosculi lyrici in arrays
 *    templatorum ponderatorum condita sunt.  Selector pondus-scalaris
 *    per Fors uniformi modo eligit.
 *
 * VI. SUBSTITUTIO
 *    Signa {A}, {N}, {P}, {F}, {R} in templatis cartarum reponuntur
 *    per valorem registri, operandum numericum, propositionem,
 *    flosculum, aut nomen registri.
 *
 * VII. BULLA
 *    Accumulator textualis limitatus; saturatio non fit quia
 *    bulla_affigit verificat spatium.
 *
 * VIII. FORS
 *    Xorshift64 deterministicum.  Semen defaltum 0xA11CEADA1729 in
 *    honorem Augustae Adae Lovelace (anno MDCCCXV natae).  Per -s N
 *    substituitur.
 *
 * IX. MODI CLI
 *    Scriptus: XV lineae interlocutoris (plus quam XIV requisitae),
 *    totidem responsiones; exitus 0.  Interactivus: per stdin, linea
 *    post lineam, exitus 0 post EOF.  Ignotum argumentum: usus
 *    Latinus ad stderr, exitus 2.
 *
 * X. ASPECTUS C99
 *    Designated initializers, compound literals, VLA (non hic adhibita
 *    pro determinismo), X-macra, variadica macra, 'restrict', 'inline',
 *    fixed-width integri ex stdint.h, unions, bitfields in Machina,
 *    tabula dispatcherum functionum, enumerationes cum valoribus
 *    explicitis — omnia ostenduntur.
 *
 * XI. DETERMINISMUS
 *    Nullus usus srand/rand; nulla functio horologii interna.
 *    Omnis fors ex xorshift64 ducitur.
 *
 * XII. VERBA CLAUDENTIA
 *    "The Analytical Engine weaves algebraic patterns just as the
 *    Jacquard loom weaves flowers and leaves." — Augusta Ada Lovelace.
 *    Hic autem machina texit verba Latina ex cartis operationum.
 */
