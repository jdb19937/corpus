/*
 * hildegardis.c — Automaton Visionum Mysticarum
 *
 * Simulacrum procedurale sanctae Hildegardis Bingensis, magistrae
 * Rupertsberga, quae visiones suas in libro SCIVIAS DOMINI conscripsit.
 * Machina haec procedit per octo modos ecclesiasticos (DORIUS et ceteri)
 * tamquam per catenam Markovianam; quisque modus inclinat electionem
 * verborum ex lexico mystico (viriditas, ignis, columba, lumen, fons
 * vivens, ordo virtutum, serpens antiquus, caritas, humilitas...).
 *
 * Sermo Hildegardis quinque partibus constat:
 *   INVOCATIO     — "Et vidi lucem..." / "Et audivi vocem de caelo..."
 *   DESCRIPTIO    — scena symbolica ex thesauro imaginum composita
 *   INTERPRETATIO — glossa allegorica sensum visionis aperiens
 *   EXHORTATIO    — monitio ethica ad animam auditricem
 *   BENEDICTIO    — optionalis clausula benedictionis
 *
 * Numerus VIRIDITATIS servatur globaliter; cum descendit, sermones de
 * ariditate et umbra loquuntur; cum ascendit, florent virides imagines.
 * Virtutes ex Ordine Virtutum (Humilitas, Caritas, Timor Domini, etc.)
 * interdum loquuntur personatae intra ipsam visionem.
 *
 * Usus:
 *   hildegardis              — colloquium scriptum cum sorore et aliis
 *   hildegardis -i           — modus interactivus (lege ex stdin)
 *   hildegardis -s N         — semen determinatum pro generatore pseudo
 *
 * Semen omissum sumitur ex tempore; alioquin numerus ille regit
 * generatorem xorshift64 deterministicum.  Scriptor: automaton C99,
 * nullis bibliothecis praeter libc utens.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdarg.h>

/* --------------------------------------------------------------------- */
/* Macrones et constantes                                                */
/* --------------------------------------------------------------------- */

#define LIMES_LINEAE       512
#define LIMES_VERBI         64
#define LIMES_SCENAE        16
#define LIMES_VIRTUTUM      16
#define NUMERUS_MODORUM      8
#define NUMERUS_DOMINIORUM   7
#define VIRIDITAS_INITIALIS 50
#define VIRIDITAS_MAXIMA   100
#define VIRIDITAS_MINIMA     0
#define COLLOQUIUM_MINIMUM  14

/* Macron variadicus pro nuntio debuggationis ad stderr */
#define QUERIMONIA(...) do { \
    fprintf(stderr, "hildegardis: " __VA_ARGS__); \
    fputc('\n', stderr); \
} while (0)

/* Macron pro scriptura benigna ad fluxum */
#define EFFUNDE(stream, ...) ((void) fprintf((stream), __VA_ARGS__))

/* --------------------------------------------------------------------- */
/* X-macrones pro modis ecclesiasticis                                    */
/* --------------------------------------------------------------------- */

#define MODI_ECCLESIASTICI_X \
    X(DORIUS,          "Dorius",          "gravis et severus")      \
    X(HYPODORIUS,      "Hypodorius",      "profundus et quietus")   \
    X(PHRYGIUS,        "Phrygius",        "ardens et mysticus")     \
    X(HYPOPHRYGIUS,    "Hypophrygius",    "lugubris et contritus")  \
    X(LYDIUS,          "Lydius",          "laetus et splendidus")   \
    X(HYPOLYDIUS,      "Hypolydius",      "devotus et suavis")      \
    X(MIXOLYDIUS,      "Mixolydius",      "angelicus et clarus")    \
    X(HYPOMIXOLYDIUS,  "Hypomixolydius",  "perfectus et profundus")

typedef enum {
#define X(clavis, nomen, descriptio) MODUS_##clavis,
    MODI_ECCLESIASTICI_X
#undef X
    MODUS_NUMERUS_TOTUS
} ModusEcclesiasticus;

static const char *const nomina_modorum[NUMERUS_MODORUM] = {
#define X(clavis, nomen, descriptio) nomen,
    MODI_ECCLESIASTICI_X
#undef X
};

static const char *const ingenia_modorum[NUMERUS_MODORUM] = {
#define X(clavis, nomen, descriptio) descriptio,
    MODI_ECCLESIASTICI_X
#undef X
};

/* --------------------------------------------------------------------- */
/* X-macrones pro fragmentis visionum (corpus imaginum)                   */
/* --------------------------------------------------------------------- */

#define FRAGMENTA_VISIONUM_X \
    X("lux vivens splendens sicut sol meridianus")                       \
    X("flamma rubicunda descendens de altissimo caelo")                  \
    X("columna lucis a terra usque ad thronum Dei")                      \
    X("mulier amicta sole et luna sub pedibus eius")                     \
    X("fons aquae vivae manans de monte sancto")                         \
    X("arbor viridissima flores aureos pariens")                         \
    X("rota ignea quattuor alis circumdata")                             \
    X("angelus alis tectus speculum lucis ferens")                       \
    X("serpens antiquus inter radices arboris cubans")                   \
    X("civitas aurea murorum duodecim gemmis ornatorum")                 \
    X("agnus candidus stans super montem Sion")                          \
    X("librum signatum sigillis septem prope altare")                    \
    X("vox tonitrui de throno dicens verba occulta")                     \
    X("pluma candida ex ore viventium procedens")                        \
    X("ignis sine fumo ascendens ad firmamentum")                        \
    X("ros caelestis super herbam viridem cadens")                       \
    X("corona sidera gemmantium duodecim super caput")                   \
    X("navicula aurea in mari vitreo natans")                            \
    X("mons sapphirinus nubibus involutus")                              \
    X("virgo nivea palmam martyrii tenens")                              \
    X("leo ex tribu Iuda rugiens in deserto")                            \
    X("porta orientalis clausa nemine transeunte")                       \
    X("speculum aeternitatis in sinu Patris abditum")                    \
    X("cithara sine chordis sonans harmoniam caelestem")

static const char *const fragmenta_visionum[] = {
#define X(textus) textus,
    FRAGMENTA_VISIONUM_X
#undef X
};
static const int numerus_fragmentorum =
    (int)(sizeof(fragmenta_visionum) / sizeof(fragmenta_visionum[0]));

/* --------------------------------------------------------------------- */
/* X-macrones pro virtutibus ex Ordine Virtutum                           */
/* --------------------------------------------------------------------- */

#define VIRTUTES_ORDINIS_X \
    X(HUMILITAS,         "Humilitas",         "humilis ancilla Domini") \
    X(CARITAS,           "Caritas",           "ignis amoris divini")    \
    X(TIMOR_DOMINI,      "Timor Domini",      "custos sapientiae")      \
    X(OBEDIENTIA,        "Obedientia",        "filia crucis")           \
    X(FIDES,             "Fides",             "lumen in tenebris")      \
    X(SPES,              "Spes",              "ancora animae")          \
    X(CASTITAS,          "Castitas",          "lilium convallium")      \
    X(INNOCENTIA,        "Innocentia",        "columba sine felle")     \
    X(CONTEMPTUS_MUNDI,  "Contemptus Mundi",  "peregrina inter saecula")\
    X(CELESTIS_AMOR,     "Celestis Amor",     "flamma non deficiens")   \
    X(DISCIPLINA,        "Disciplina",        "virga directionis")      \
    X(VERECUNDIA,        "Verecundia",        "velum pudicitiae")       \
    X(MISERICORDIA,      "Misericordia",      "balsamum vulneratorum")  \
    X(VICTORIA,          "Victoria",          "corona certantium")      \
    X(DISCRETIO,         "Discretio",         "mater virtutum")         \
    X(PATIENTIA,         "Patientia",         "radix iustitiae")

typedef enum {
#define X(clavis, nomen, epitheton) VIRTUS_##clavis,
    VIRTUTES_ORDINIS_X
#undef X
    VIRTUS_NUMERUS_TOTUS
} VirtusOrdinis;

static const char *const nomina_virtutum[VIRTUS_NUMERUS_TOTUS] = {
#define X(clavis, nomen, epitheton) nomen,
    VIRTUTES_ORDINIS_X
#undef X
};

static const char *const epitheta_virtutum[VIRTUS_NUMERUS_TOTUS] = {
#define X(clavis, nomen, epitheton) epitheton,
    VIRTUTES_ORDINIS_X
#undef X
};

/* --------------------------------------------------------------------- */
/* Dominia spiritualia                                                    */
/* --------------------------------------------------------------------- */

typedef enum {
    DOMINIUM_INFIRMITAS = 0,
    DOMINIUM_DUBITATIO,
    DOMINIUM_SUPERBIA,
    DOMINIUM_GRATIA,
    DOMINIUM_CREATIO,
    DOMINIUM_IUSTITIA,
    DOMINIUM_AMOR_DIVINUS
} DominiumSpirituale;

static const char *const nomina_dominiorum[NUMERUS_DOMINIORUM] = {
    "Infirmitas",
    "Dubitatio",
    "Superbia",
    "Gratia",
    "Creatio",
    "Iustitia",
    "Amor Divinus"
};

/* Claves verbales pro quoque dominio (ad recognoscendam interrogationem) */
static const char *const claves_infirmitatis[] = {
    "aegritudo", "morbus", "dolor", "infirmus", "infirma", "languor",
    "febris", "vulnus", "corpus", "caro", "medicus", "sanare", "aeger",
    "languens", "mors", "moribundus", NULL
};
static const char *const claves_dubitationis[] = {
    "dubium", "quaero", "nescio", "cur", "quomodo", "quare", "unde",
    "fides", "credo", "incredulus", "haesito", "dubitatio", "cogito",
    "intellego", "ratio", "sensus", NULL
};
static const char *const claves_superbiae[] = {
    "superbia", "ego", "mihi", "gloria", "laus", "dignus", "maior",
    "potens", "excelsus", "arrogantia", "fastus", "elatus", "regnum",
    "dominium", "potestas", NULL
};
static const char *const claves_gratiae[] = {
    "gratia", "donum", "misericordia", "veniam", "ignosce", "benedictio",
    "munus", "pietas", "clementia", "favor", "dono", "largiri", NULL
};
static const char *const claves_creationis[] = {
    "creatio", "mundus", "terra", "caelum", "stella", "sol", "luna",
    "flos", "herba", "arbor", "animal", "homo", "fecit", "creavit",
    "natura", "elementum", "viriditas", "virens", NULL
};
static const char *const claves_iustitiae[] = {
    "iustitia", "iudex", "iudicium", "peccatum", "culpa", "lex",
    "veritas", "iustus", "iniquitas", "reus", "sententia", "punitio",
    "damnum", "aequitas", NULL
};
static const char *const claves_amoris[] = {
    "amor", "caritas", "diligo", "amicus", "amica", "sponsa", "sponsus",
    "dulcis", "suavis", "cor", "amabilis", "amans", "osculum", NULL
};

static const char *const *const claves_dominiorum[NUMERUS_DOMINIORUM] = {
    claves_infirmitatis,
    claves_dubitationis,
    claves_superbiae,
    claves_gratiae,
    claves_creationis,
    claves_iustitiae,
    claves_amoris
};

/* --------------------------------------------------------------------- */
/* Lexicon mysticum — verba cum inclinatione modali                       */
/* --------------------------------------------------------------------- */

typedef struct {
    const char *verbum;
    uint16_t pondera[NUMERUS_MODORUM]; /* inclinatio pro quoque modo */
} VerbumMysticum;

/* Ingenium modi biasat electionem: maior pondus = maior probabilitas */
static const VerbumMysticum lexicon_mysticum[] = {
    { "viriditas",        { 40, 30, 20, 10, 90, 70, 50, 40 } },
    { "ignis vivens",     { 30, 10, 90, 50, 60, 30, 40, 40 } },
    { "columba candida",  { 60, 70, 20, 30, 50, 80, 70, 60 } },
    { "lumen aeternum",   { 50, 40, 60, 40, 80, 60, 90, 70 } },
    { "fons vivens",      { 70, 80, 30, 40, 50, 70, 40, 50 } },
    { "ordo virtutum",    { 80, 60, 40, 50, 40, 70, 60, 90 } },
    { "serpens antiquus", { 20, 10, 70, 90, 10, 20, 20, 30 } },
    { "caritas ardens",   { 40, 30, 80, 40, 70, 60, 70, 80 } },
    { "humilitas vera",   { 60, 90, 30, 80, 30, 70, 50, 60 } },
    { "scientia Dei",     { 70, 60, 50, 40, 60, 60, 80, 90 } },
    { "sapientia alta",   { 80, 70, 40, 40, 50, 60, 70, 80 } },
    { "gratia infusa",    { 50, 60, 40, 60, 70, 80, 70, 60 } },
    { "timor sanctus",    { 40, 80, 60, 90, 20, 50, 40, 50 } },
    { "pax altissima",    { 60, 70, 30, 50, 60, 80, 70, 70 } },
    { "ardor spiritus",   { 30, 20, 90, 40, 70, 40, 60, 50 } },
    { "aqua baptismi",    { 70, 80, 40, 50, 40, 70, 50, 60 } },
    { "spiritus sanctus", { 50, 50, 70, 60, 70, 60, 90, 80 } },
    { "nox contritionis", { 30, 80, 60, 90, 10, 30, 20, 40 } },
    { "dies novus",       { 60, 40, 50, 30, 90, 50, 80, 60 } },
    { "aula caelestis",   { 80, 60, 50, 40, 70, 70, 80, 90 } }
};
static const int numerus_lexici =
    (int)(sizeof(lexicon_mysticum) / sizeof(lexicon_mysticum[0]));

/* --------------------------------------------------------------------- */
/* Templa pro invocationibus, exhortationibus, benedictionibus            */
/* --------------------------------------------------------------------- */

static const char *const invocationes[] = {
    "Et vidi, et ecce",
    "Et audivi vocem de caelo dicentem",
    "In visione animae meae apparuit",
    "Ostendit mihi Dominus viventium",
    "Et vidi velut",
    "Et cecidi in pavorem magnum cum viderem",
    "Tunc aperti sunt oculi cordis mei et vidi",
    "Ecce in contemplatione sublevata sum et conspexi"
};
static const int numerus_invocationum =
    (int)(sizeof(invocationes) / sizeof(invocationes[0]));

static const char *const interpretationes_praefationes[] = {
    "Et vox de throno interpretata est mihi dicens",
    "Quod significat",
    "Haec autem visio ostendit",
    "Ita enim intellexi",
    "Ecce sensus mysticus",
    "Quae omnia figurae sunt"
};
static const int numerus_interpretationum =
    (int)(sizeof(interpretationes_praefationes) /
          sizeof(interpretationes_praefationes[0]));

static const char *const exhortationes_infirmitatis[] = {
    "O filia Syon, confide in Eo qui vulnera sanat lino caritatis.",
    "In aegritudine corporis quaere medicum animae tuae.",
    "Non est languor qui non cedat oleo misericordiae Dei altissimi.",
    "Conservet se homo ne caro eius corpus spiritui imperet."
};
static const char *const exhortationes_dubitationis[] = {
    "Noli incredula esse, sed fidelis inter umbras noctis.",
    "Non quaerit Deus ut intellegas, sed ut credendo videas.",
    "Ubi ratio titubat, ibi fides alis altius volat.",
    "Sinite quaestiones vestras evanescere ante faciem Viventis."
};
static const char *const exhortationes_superbiae[] = {
    "Humilia te sub potenti manu Dei, ne elatio te deicit.",
    "Qui se exaltat humiliabitur, dicit Dominus exercituum.",
    "Fuge fastum sicut serpentem antiquum in horto.",
    "Memento quia pulvis es et in pulverem reverteris."
};
static const char *const exhortationes_gratiae[] = {
    "Accipe donum gratiae nec clauderis vasculum ab alto.",
    "Gratia enim superabundat ubi abundavit peccatum.",
    "Aperi cor tuum et implebitur rore caelesti.",
    "Gaudium magnum est gratias agere sine intermissione."
};
static const char *const exhortationes_creationis[] = {
    "Omnis creatura laudat Creatorem voce sua.",
    "In viridi herba contemplare Artificem summum.",
    "Mundus non est finis, sed vestigium Dei viventis.",
    "Stellae canunt Deo in silentio noctis suavissimo."
};
static const char *const exhortationes_iustitiae[] = {
    "Iustitia Dei non dormit, sed sicut mons immobilis stat.",
    "Peccatum quaerit latibulum, sed lux omnia revelat.",
    "Iudex venturus est cum igne purgante non destruente.",
    "Ambula in semita rectitudinis et non deviabis a vita."
};
static const char *const exhortationes_amoris[] = {
    "Caritas non deficit umquam, etsi prophetiae evacuentur.",
    "Amor Dei maior est mari et profundior abysso.",
    "In caritate ardens fit anima speculum Christi amantis.",
    "Dilige et fac quod vis, si dilectio vera fuerit."
};

static const char *const *const exhortationes_per_dominium[NUMERUS_DOMINIORUM] = {
    exhortationes_infirmitatis,
    exhortationes_dubitationis,
    exhortationes_superbiae,
    exhortationes_gratiae,
    exhortationes_creationis,
    exhortationes_iustitiae,
    exhortationes_amoris
};

static const int numera_exhortationum[NUMERUS_DOMINIORUM] = { 4, 4, 4, 4, 4, 4, 4 };

static const char *const benedictiones[] = {
    "Benedicat te Pater luminum et custodiat Filius in pace.",
    "Viriditas Spiritus Sancti maneat super te in aeternum.",
    "Vade in osculo caritatis aeternae et nil timeas.",
    "Pax tecum et lumen aeternum super vultum tuum.",
    "Sancta Trinitas corroboret gressus tuos usque ad patriam."
};
static const int numerus_benedictionum =
    (int)(sizeof(benedictiones) / sizeof(benedictiones[0]));

/* --------------------------------------------------------------------- */
/* Matrix transitionum inter modos (octo stati, singulae lineae           */
/* summam habentes prope 1000 milesimas); initializata per designationem. */
/* --------------------------------------------------------------------- */

static const int matrix_transitionum[NUMERUS_MODORUM][NUMERUS_MODORUM] = {
    [MODUS_DORIUS] = {
        [MODUS_DORIUS] = 300, [MODUS_HYPODORIUS] = 200,
        [MODUS_PHRYGIUS] = 100, [MODUS_HYPOPHRYGIUS] = 50,
        [MODUS_LYDIUS] = 100, [MODUS_HYPOLYDIUS] = 100,
        [MODUS_MIXOLYDIUS] = 100, [MODUS_HYPOMIXOLYDIUS] = 50
    },
    [MODUS_HYPODORIUS] = {
        [MODUS_DORIUS] = 250, [MODUS_HYPODORIUS] = 250,
        [MODUS_PHRYGIUS] = 50, [MODUS_HYPOPHRYGIUS] = 150,
        [MODUS_LYDIUS] = 50, [MODUS_HYPOLYDIUS] = 150,
        [MODUS_MIXOLYDIUS] = 50, [MODUS_HYPOMIXOLYDIUS] = 50
    },
    [MODUS_PHRYGIUS] = {
        [MODUS_DORIUS] = 100, [MODUS_HYPODORIUS] = 50,
        [MODUS_PHRYGIUS] = 250, [MODUS_HYPOPHRYGIUS] = 250,
        [MODUS_LYDIUS] = 50, [MODUS_HYPOLYDIUS] = 50,
        [MODUS_MIXOLYDIUS] = 150, [MODUS_HYPOMIXOLYDIUS] = 100
    },
    [MODUS_HYPOPHRYGIUS] = {
        [MODUS_DORIUS] = 50, [MODUS_HYPODORIUS] = 200,
        [MODUS_PHRYGIUS] = 200, [MODUS_HYPOPHRYGIUS] = 300,
        [MODUS_LYDIUS] = 50, [MODUS_HYPOLYDIUS] = 100,
        [MODUS_MIXOLYDIUS] = 50, [MODUS_HYPOMIXOLYDIUS] = 50
    },
    [MODUS_LYDIUS] = {
        [MODUS_DORIUS] = 100, [MODUS_HYPODORIUS] = 50,
        [MODUS_PHRYGIUS] = 50, [MODUS_HYPOPHRYGIUS] = 50,
        [MODUS_LYDIUS] = 300, [MODUS_HYPOLYDIUS] = 200,
        [MODUS_MIXOLYDIUS] = 150, [MODUS_HYPOMIXOLYDIUS] = 100
    },
    [MODUS_HYPOLYDIUS] = {
        [MODUS_DORIUS] = 100, [MODUS_HYPODORIUS] = 150,
        [MODUS_PHRYGIUS] = 50, [MODUS_HYPOPHRYGIUS] = 100,
        [MODUS_LYDIUS] = 200, [MODUS_HYPOLYDIUS] = 250,
        [MODUS_MIXOLYDIUS] = 100, [MODUS_HYPOMIXOLYDIUS] = 50
    },
    [MODUS_MIXOLYDIUS] = {
        [MODUS_DORIUS] = 100, [MODUS_HYPODORIUS] = 50,
        [MODUS_PHRYGIUS] = 150, [MODUS_HYPOPHRYGIUS] = 50,
        [MODUS_LYDIUS] = 150, [MODUS_HYPOLYDIUS] = 100,
        [MODUS_MIXOLYDIUS] = 250, [MODUS_HYPOMIXOLYDIUS] = 150
    },
    [MODUS_HYPOMIXOLYDIUS] = {
        [MODUS_DORIUS] = 50, [MODUS_HYPODORIUS] = 100,
        [MODUS_PHRYGIUS] = 100, [MODUS_HYPOPHRYGIUS] = 100,
        [MODUS_LYDIUS] = 100, [MODUS_HYPOLYDIUS] = 150,
        [MODUS_MIXOLYDIUS] = 150, [MODUS_HYPOMIXOLYDIUS] = 250
    }
};

/* --------------------------------------------------------------------- */
/* Unio pro vasculo semini — ostendit bytes singulos et quod integrum     */
/* --------------------------------------------------------------------- */

typedef union {
    uint64_t integrum;
    uint8_t  octeta[8];
    struct {
        uint32_t infimum;
        uint32_t supremum;
    } partes;
} VasculumSeminis;

/* --------------------------------------------------------------------- */
/* Status auditoris: structura cum campis bit (bitfields) et FAM          */
/* --------------------------------------------------------------------- */

typedef struct {
    unsigned est_ancilla       : 1;
    unsigned est_episcopus     : 1;
    unsigned est_peregrinus    : 1;
    unsigned petit_benedictionem : 1;
    unsigned monet_contra_haeresim : 1;
    unsigned reservatum        : 11;
} IngeniumAuditoris;

typedef struct {
    char nomen[64];
    IngeniumAuditoris ingenium;
    int turnus;                 /* numerus turni in colloquio */
    int longitudo_dictorum;     /* numerus verborum in dictis */
    char dicta[];               /* membrum flexibile finale */
} Auditor;

/* --------------------------------------------------------------------- */
/* Status automati — mutabilis                                            */
/* --------------------------------------------------------------------- */

typedef struct {
    uint64_t semen;
    ModusEcclesiasticus modus_currens;
    int viriditas;              /* 0..100 */
    int numerus_turni;
    int ultima_virtus_locuta;   /* -1 si nulla */
    int numerus_visionum;
    DominiumSpirituale ultimum_dominium;
} StatusHildegardis;

/* --------------------------------------------------------------------- */
/* Generator pseudo-aleatorius xorshift64 (deterministicus)               */
/* --------------------------------------------------------------------- */

static inline uint64_t
sors_misce(uint64_t *restrict vas)
{
    uint64_t x = *vas;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    /* Protege ne zero perpetuum fiat */
    if (x == 0)
        x = UINT64_C(0x9E3779B97F4A7C15);
    *vas = x;
    return x;
}

static inline int
sors_infra(uint64_t *restrict vas, int limes)
{
    if (limes <= 0)
        return 0;
    return (int)(sors_misce(vas) % (uint64_t)limes);
}

static inline int
sors_pondere(uint64_t *restrict vas, const int *pondera, int n)
{
    int summa = 0;
    for (int i = 0; i < n; i++)
        summa += pondera[i];
    if (summa <= 0)
        return sors_infra(vas, n);
    int punctum = sors_infra(vas, summa);
    int cumulus = 0;
    for (int i = 0; i < n; i++) {
        cumulus += pondera[i];
        if (punctum < cumulus)
            return i;
    }
    return n - 1;
}

/* --------------------------------------------------------------------- */
/* Adminicula viriditatis                                                 */
/* --------------------------------------------------------------------- */

static inline void
viriditas_auge(StatusHildegardis *restrict s, int quantitas)
{
    s->viriditas += quantitas;
    if (s->viriditas > VIRIDITAS_MAXIMA)
        s->viriditas = VIRIDITAS_MAXIMA;
    if (s->viriditas < VIRIDITAS_MINIMA)
        s->viriditas = VIRIDITAS_MINIMA;
}

static const char *
viriditas_descriptio(int v)
{
    if (v >= 80) return "plenitudine virentissima florens";
    if (v >= 60) return "viridi sanitate vegetans";
    if (v >= 40) return "moderata viriditate quiescens";
    if (v >= 20) return "tenui viriditate languescens";
    return "arida et sicca tamquam caro sine oleo";
}

/* --------------------------------------------------------------------- */
/* Analysis linguistica                                                   */
/* --------------------------------------------------------------------- */

static int
verbum_continet(const char *haec, const char *quaesitum)
{
    size_t lh = strlen(haec), lq = strlen(quaesitum);
    if (lq > lh) return 0;
    for (size_t i = 0; i + lq <= lh; i++) {
        int concordat = 1;
        for (size_t j = 0; j < lq; j++) {
            char a = (char)tolower((unsigned char)haec[i + j]);
            char b = (char)tolower((unsigned char)quaesitum[j]);
            if (a != b) { concordat = 0; break; }
        }
        if (concordat) return 1;
    }
    return 0;
}

static DominiumSpirituale
dominium_inveni(const char *inputus)
{
    int scores[NUMERUS_DOMINIORUM] = { 0 };
    for (int d = 0; d < NUMERUS_DOMINIORUM; d++) {
        const char *const *cl = claves_dominiorum[d];
        for (int i = 0; cl[i] != NULL; i++) {
            if (verbum_continet(inputus, cl[i]))
                scores[d]++;
        }
    }
    int maxidx = 0, maxval = scores[0];
    for (int d = 1; d < NUMERUS_DOMINIORUM; d++) {
        if (scores[d] > maxval) {
            maxval = scores[d];
            maxidx = d;
        }
    }
    if (maxval == 0)
        return DOMINIUM_CREATIO; /* defaltum: contemplatio creationis */
    return (DominiumSpirituale)maxidx;
}

/* Verba virentia augent viriditatem; verba arida minuunt */
static int
viriditas_efficacia_inputi(const char *inputus)
{
    static const char *virentia[] = {
        "viriditas", "virens", "viridis", "flos", "flores", "herba",
        "arbor", "fons", "aqua", "ros", "lumen", "sol", "vita",
        "vivens", "vivus", "caritas", "gratia", "amor", NULL
    };
    static const char *arida[] = {
        "ariditas", "aridus", "siccus", "mors", "pulvis", "tenebrae",
        "umbra", "peccatum", "dubium", "superbia", "ira", "odium",
        "languor", "morbus", NULL
    };
    int delta = 0;
    for (int i = 0; virentia[i] != NULL; i++)
        if (verbum_continet(inputus, virentia[i]))
            delta += 3;
    for (int i = 0; arida[i] != NULL; i++)
        if (verbum_continet(inputus, arida[i]))
            delta -= 3;
    return delta;
}

/* --------------------------------------------------------------------- */
/* Transitio modalis                                                      */
/* --------------------------------------------------------------------- */

static void
modus_advance(StatusHildegardis *restrict s)
{
    int pondera[NUMERUS_MODORUM];
    for (int i = 0; i < NUMERUS_MODORUM; i++)
        pondera[i] = matrix_transitionum[s->modus_currens][i];
    int electus = sors_pondere(&s->semen, pondera, NUMERUS_MODORUM);
    s->modus_currens = (ModusEcclesiasticus)electus;
}

/* --------------------------------------------------------------------- */
/* Selectio verbi mystici secundum modum currentem                        */
/* --------------------------------------------------------------------- */

static const char *
verbum_mysticum_elige(StatusHildegardis *restrict s)
{
    int pondera[64];
    int n = numerus_lexici;
    if (n > 64) n = 64;
    for (int i = 0; i < n; i++)
        pondera[i] = lexicon_mysticum[i].pondera[s->modus_currens];
    int idx = sors_pondere(&s->semen, pondera, n);
    return lexicon_mysticum[idx].verbum;
}

/* --------------------------------------------------------------------- */
/* Fragmentum visionis aleatorie electum                                  */
/* --------------------------------------------------------------------- */

static const char *
fragmentum_elige(StatusHildegardis *restrict s)
{
    int idx = sors_infra(&s->semen, numerus_fragmentorum);
    return fragmenta_visionum[idx];
}

/* Virtus aleatorie electa, sed non eadem duabus vicibus seriatim */
static VirtusOrdinis
virtus_elige(StatusHildegardis *restrict s)
{
    VirtusOrdinis v;
    int i;
    for (i = 0; i < 8; i++) {
        v = (VirtusOrdinis)sors_infra(&s->semen, VIRTUS_NUMERUS_TOTUS);
        if ((int)v != s->ultima_virtus_locuta)
            break;
    }
    s->ultima_virtus_locuta = (int)v;
    return v;
}

/* --------------------------------------------------------------------- */
/* Scriptura partium visionis                                             */
/* --------------------------------------------------------------------- */

typedef void (*FunctioPars)(StatusHildegardis *, FILE *, DominiumSpirituale);

static void
pars_invocatio_scribe(StatusHildegardis *s, FILE *f,
                      DominiumSpirituale d)
{
    (void)d;
    const char *invoc = invocationes[
        sors_infra(&s->semen, numerus_invocationum)];
    EFFUNDE(f, "%s ", invoc);
    /* adde inclinationem modalem */
    EFFUNDE(f, "in modo %s (%s), ",
            nomina_modorum[s->modus_currens],
            ingenia_modorum[s->modus_currens]);
    if (s->viriditas >= 60)
        EFFUNDE(f, "cum viriditas caelestis iterum effunderetur, ");
    else if (s->viriditas < 30)
        EFFUNDE(f, "cum ariditas terrae meae me contristaret, ");
}

static void
pars_descriptio_scribe(StatusHildegardis *s, FILE *f,
                       DominiumSpirituale d)
{
    (void)d;
    /* Scena VLA — longitudo electa aleatorie */
    int n_imaginum = 2 + sors_infra(&s->semen, 3); /* 2..4 */
    if (n_imaginum > LIMES_SCENAE) n_imaginum = LIMES_SCENAE;
    const char *imagines[LIMES_SCENAE];
    for (int i = 0; i < n_imaginum; i++)
        imagines[i] = fragmentum_elige(s);
    const char *verbum = verbum_mysticum_elige(s);
    EFFUNDE(f, "%s", imagines[0]);
    for (int i = 1; i < n_imaginum; i++) {
        const char *con = (i + 1 == n_imaginum) ? ", et " : ", ";
        EFFUNDE(f, "%s%s", con, imagines[i]);
    }
    EFFUNDE(f, "; et super omnia haec %s pendebat tamquam sigillum.",
            verbum);
}

static void
pars_interpretatio_scribe(StatusHildegardis *s, FILE *f,
                          DominiumSpirituale d)
{
    const char *praefatio = interpretationes_praefationes[
        sors_infra(&s->semen, numerus_interpretationum)];
    const char *verbum = verbum_mysticum_elige(s);
    EFFUNDE(f, "%s: %s est figura %s, quae in anima fideli %s.",
            praefatio,
            verbum,
            nomina_dominiorum[d],
            (s->viriditas >= 50)
                ? "floret sicut palma in Cades"
                : "gemit sicut turtur in solitudine");
}

static void
pars_virtus_loquitur(StatusHildegardis *s, FILE *f,
                     DominiumSpirituale d)
{
    (void)d;
    VirtusOrdinis v = virtus_elige(s);
    const char *nomen = nomina_virtutum[v];
    const char *epith = epitheta_virtutum[v];
    static const char *const verba_virtutum[] = {
        "ego sum quae ad portam vitae ducit",
        "ego sum quae animas lavat in fonte caritatis",
        "ego sum quae inimicum antiquum terit sub pede",
        "ego sum quae vestes sponsae candidas texit",
        "ego sum quae corda confracta consolor",
        "ego sum quae cum Agno in nuptiis saltat"
    };
    static const int n_verba = (int)(sizeof(verba_virtutum) /
                                     sizeof(verba_virtutum[0]));
    const char *ait = verba_virtutum[sors_infra(&s->semen, n_verba)];
    EFFUNDE(f, "Tunc exsurgens %s, %s, dixit: \"%s.\"", nomen, epith, ait);
}

static void
pars_exhortatio_scribe(StatusHildegardis *s, FILE *f,
                       DominiumSpirituale d)
{
    const char *const *arr = exhortationes_per_dominium[d];
    int n = numera_exhortationum[d];
    const char *ex = arr[sors_infra(&s->semen, n)];
    EFFUNDE(f, "%s", ex);
}

static void
pars_benedictio_scribe(StatusHildegardis *s, FILE *f,
                       DominiumSpirituale d)
{
    (void)d;
    const char *b = benedictiones[
        sors_infra(&s->semen, numerus_benedictionum)];
    EFFUNDE(f, "%s", b);
}

/* Vector functionum partium (dispatch per pointer) */
static const FunctioPars partes_visionis[] = {
    pars_invocatio_scribe,
    pars_descriptio_scribe,
    pars_interpretatio_scribe,
    pars_exhortatio_scribe
};
static const int numerus_partium =
    (int)(sizeof(partes_visionis) / sizeof(partes_visionis[0]));

/* --------------------------------------------------------------------- */
/* Compositio totius visionis                                             */
/* --------------------------------------------------------------------- */

static void
visio_compone(StatusHildegardis *restrict s, FILE *f, const char *inputus)
{
    DominiumSpirituale d = dominium_inveni(inputus);
    s->ultimum_dominium = d;

    /* Adiusta viriditatem ex inputo */
    viriditas_auge(s, viriditas_efficacia_inputi(inputus));

    /* Scribe marginem monastici codicis */
    EFFUNDE(f, "  [modus %s | viriditas %d | dominium %s]\n",
            nomina_modorum[s->modus_currens],
            s->viriditas,
            nomina_dominiorum[d]);

    /* Compositum literarum per compound literal */
    int ordo[4] = { 0, 1, 2, 3 };
    /* scriptio */
    for (int i = 0; i < numerus_partium; i++) {
        partes_visionis[ordo[i]](s, f, d);
        fputc(' ', f);
    }

    /* Virtus quoque loquitur cum probabilitate */
    if (sors_infra(&s->semen, 100) < 45) {
        fputc('\n', f);
        EFFUNDE(f, "  ");
        pars_virtus_loquitur(s, f, d);
    }

    /* Benedictio cum probabilitate (et si viriditas alta) */
    int prob_benedictionis = 20 + s->viriditas / 4;
    if (sors_infra(&s->semen, 100) < prob_benedictionis) {
        fputc('\n', f);
        EFFUNDE(f, "  ");
        pars_benedictio_scribe(s, f, d);
    }

    fputc('\n', f);
    EFFUNDE(f, "  — Hildegardis ancilla Domini, %s.\n",
            viriditas_descriptio(s->viriditas));

    /* Advance modum pro proximo turno */
    modus_advance(s);
    s->numerus_visionum++;
}

/* --------------------------------------------------------------------- */
/* Initializatio status                                                   */
/* --------------------------------------------------------------------- */

static void
status_initialize(StatusHildegardis *restrict s, uint64_t semen)
{
    /* Designated initializer in compound literal */
    *s = (StatusHildegardis){
        .semen = semen ? semen : UINT64_C(0xD1B54A32D192ED03),
        .modus_currens = MODUS_DORIUS,
        .viriditas = VIRIDITAS_INITIALIS,
        .numerus_turni = 0,
        .ultima_virtus_locuta = -1,
        .numerus_visionum = 0,
        .ultimum_dominium = DOMINIUM_CREATIO
    };

    /* Unio demonstrans bytes seminis (non adhibita sed valida) */
    VasculumSeminis vas = { .integrum = s->semen };
    (void)vas.octeta[0];
    (void)vas.partes.infimum;
}

/* --------------------------------------------------------------------- */
/* Colloquium scriptum: Hildegardis cum soror, episcopus, peregrinus      */
/* --------------------------------------------------------------------- */

typedef struct {
    const char *loquens;        /* nomen auditoris */
    const char *genus;          /* e.g. "soror", "episcopus", "peregrinus" */
    const char *dictum;         /* verba eius */
} ScriptumTurnus;

static const ScriptumTurnus turnus_scripta[] = {
    { "Soror Richardis", "soror",
      "Magistra, infirmitas corporis mei me graviter premit; dolor capitis "
      "non cessat et caro mea languet ut herba sicca." },
    { "Soror Richardis", "soror",
      "Sed quomodo, magistra, sciam utrum haec infirmitas sit castigatio "
      "an probatio? Dubium cor meum dilacerat." },
    { "Episcopus Magontinus", "episcopus",
      "Venerabilis Hildegardis, de peccato superbiae in clero nostro "
      "interrogo te; ego ipse timeo ne gloria mea me seducat." },
    { "Episcopus Magontinus", "episcopus",
      "Et de iustitia Dei, quid dicis? Multi iudices nostri iniquitatem "
      "pro iustitia vendunt et culpa populum opprimit." },
    { "Peregrinus ex Hispania", "peregrinus",
      "O virgo sapientissima, creatio tota mihi mysterium est; stellae, "
      "sol, luna, herba viridis — unde veniunt haec omnia?" },
    { "Peregrinus ex Hispania", "peregrinus",
      "Et gratiam Dei quaero in via mea; donum clementiae quaero, "
      "quia corpus meum fessum est ab itinere longissimo." },
    { "Soror Richardis", "soror",
      "Et de amore divino, magistra, quomodo sponsa Christi diligere "
      "debet, ut caritas non deficiat sed ardeat?" },
    { "Episcopus Magontinus", "episcopus",
      "Mater, rursus dubito: quid si visiones istae tuae phantasmata "
      "sunt? Ratio mea haesitat in hac quaestione." },
    { "Peregrinus ex Hispania", "peregrinus",
      "Corpus meum rursus dolet, aegritudo manuum et pedum; medicus in "
      "Hispania nil profuit mihi ad languorem hunc." },
    { "Soror Richardis", "soror",
      "Magistra, anima mea vult intellegere ordinem virtutum; quomodo "
      "humilitas contra superbiam stat in certamine interno?" },
    { "Episcopus Magontinus", "episcopus",
      "Et gratiam pro familia mea peto, ut benedictio Dei maneat super "
      "domum meam et servos meos." },
    { "Peregrinus ex Hispania", "peregrinus",
      "De iustitia Dei, venerabilis, quaero: cur iusti patiuntur et "
      "iniqui florent sicut arbor viridis in aestate?" },
    { "Soror Richardis", "soror",
      "Magistra, creatio haec mirabilis quomodo cantat Deo? Ego audire "
      "volo harmoniam viventium stellarum." },
    { "Omnes tres", "congregatio",
      "Benedic nobis, sancta mater, et caritatem divinam super nos "
      "effunde antequam recedamus a cella tua." }
};
static const int numerus_turnorum =
    (int)(sizeof(turnus_scripta) / sizeof(turnus_scripta[0]));

/* --------------------------------------------------------------------- */
/* Impressio auditoris (auditor structura cum flexibili membro)           */
/* --------------------------------------------------------------------- */

static void
auditor_imprime(FILE *f, const Auditor *a, const char *dictum)
{
    EFFUNDE(f, "%s", a->nomen);
    if (a->ingenium.est_ancilla)
        EFFUNDE(f, " (ancilla Domini)");
    else if (a->ingenium.est_episcopus)
        EFFUNDE(f, " (pontifex)");
    else if (a->ingenium.est_peregrinus)
        EFFUNDE(f, " (peregrinus)");
    EFFUNDE(f, ":\n  %s\n\n", dictum);
}

static Auditor *
auditor_crea(const char *nomen, const char *genus, const char *dictum)
{
    size_t ld = strlen(dictum) + 1;
    Auditor *a = (Auditor *)calloc(1, sizeof(Auditor) + ld);
    if (!a) return NULL;
    strncpy(a->nomen, nomen, sizeof(a->nomen) - 1);
    a->nomen[sizeof(a->nomen) - 1] = '\0';
    a->ingenium.est_ancilla = (strcmp(genus, "soror") == 0);
    a->ingenium.est_episcopus = (strcmp(genus, "episcopus") == 0);
    a->ingenium.est_peregrinus = (strcmp(genus, "peregrinus") == 0);
    a->ingenium.petit_benedictionem = 0;
    a->ingenium.monet_contra_haeresim = 0;
    a->turnus = 0;
    a->longitudo_dictorum = (int)strlen(dictum);
    memcpy(a->dicta, dictum, ld);
    return a;
}

/* --------------------------------------------------------------------- */
/* Colloquium scriptum                                                    */
/* --------------------------------------------------------------------- */

static void
colloquium_scriptum_ducere(StatusHildegardis *restrict s, FILE *f)
{
    EFFUNDE(f, "==============================================\n");
    EFFUNDE(f, "  COLLOQUIUM CUM HILDEGARDE BINGENSI\n");
    EFFUNDE(f, "  (visiones mysticae in cella Rupertsberga)\n");
    EFFUNDE(f, "==============================================\n\n");

    int turnus_minimus = COLLOQUIUM_MINIMUM;
    int n = numerus_turnorum;
    if (n < turnus_minimus)
        n = turnus_minimus;
    if (n > numerus_turnorum)
        n = numerus_turnorum;

    for (int i = 0; i < n; i++) {
        const ScriptumTurnus *t = &turnus_scripta[i];
        Auditor *a = auditor_crea(t->loquens, t->genus, t->dictum);
        if (!a) continue;
        auditor_imprime(f, a, t->dictum);

        /* Hildegardis respondet */
        EFFUNDE(f, "Hildegardis (turnus %d):\n", i + 1);
        visio_compone(s, f, t->dictum);
        EFFUNDE(f, "\n");

        s->numerus_turni++;
        free(a);
    }

    EFFUNDE(f, "==============================================\n");
    EFFUNDE(f, "  Finis colloquii. Visiones totales: %d.\n",
            s->numerus_visionum);
    EFFUNDE(f, "  Modus ultimus: %s. Viriditas ultima: %d.\n",
            nomina_modorum[s->modus_currens], s->viriditas);
    EFFUNDE(f, "==============================================\n");
}

/* --------------------------------------------------------------------- */
/* Modus interactivus                                                     */
/* --------------------------------------------------------------------- */

static void
purgare_lineam(char *linea)
{
    size_t l = strlen(linea);
    while (l > 0 && (linea[l-1] == '\n' || linea[l-1] == '\r' ||
                     linea[l-1] == ' ' || linea[l-1] == '\t')) {
        linea[l-1] = '\0';
        l--;
    }
}

static void
modus_interactivus_ducere(StatusHildegardis *restrict s)
{
    EFFUNDE(stdout, "==============================================\n");
    EFFUNDE(stdout, "  HILDEGARDIS BINGENSIS — modus interactivus\n");
    EFFUNDE(stdout, "  Scribe interrogationem tuam; linea vacua aut EOF finit.\n");
    EFFUNDE(stdout, "==============================================\n\n");

    char linea[LIMES_LINEAE];
    int turnus = 1;
    while (1) {
        EFFUNDE(stdout, "Tu > ");
        fflush(stdout);
        if (!fgets(linea, sizeof(linea), stdin))
            break;
        purgare_lineam(linea);
        if (linea[0] == '\0')
            break;
        EFFUNDE(stdout, "\nHildegardis (turnus %d):\n", turnus++);
        visio_compone(s, stdout, linea);
        EFFUNDE(stdout, "\n");
        s->numerus_turni++;
    }

    EFFUNDE(stdout, "\n  Vade in pace, peregrine. Viriditas tecum.\n");
}

/* --------------------------------------------------------------------- */
/* Usus ad stderr                                                         */
/* --------------------------------------------------------------------- */

static void
usus_imprime(const char *prog)
{
    EFFUNDE(stderr,
        "Usus: %s [-i] [-s SEMEN]\n"
        "  (nullum argumentum)  colloquium scriptum cum sororibus et ceteris\n"
        "  -i                   modus interactivus (lege ex stdin)\n"
        "  -s SEMEN             semen determinatum pro generatore (integer)\n"
        "  -h                   ostende hunc usum et exi\n"
        "Exempli gratia:\n"
        "  %s\n"
        "  %s -s 42\n"
        "  %s -i -s 1729\n",
        prog, prog, prog, prog);
}

/* --------------------------------------------------------------------- */
/* Argumenta legere                                                       */
/* --------------------------------------------------------------------- */

typedef struct {
    int interactivus;
    int habet_semen;
    uint64_t semen;
} ArgumentaElecta;

static int
argumenta_legere(int argc, char **argv, ArgumentaElecta *out)
{
    *out = (ArgumentaElecta){ .interactivus = 0, .habet_semen = 0, .semen = 0 };
    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        if (strcmp(a, "-i") == 0) {
            out->interactivus = 1;
        } else if (strcmp(a, "-s") == 0) {
            if (i + 1 >= argc) {
                QUERIMONIA("Flagulum -s requirit argumentum numericum.");
                return 2;
            }
            char *finis = NULL;
            unsigned long long v = strtoull(argv[++i], &finis, 10);
            if (!finis || *finis != '\0') {
                QUERIMONIA("Semen non est numerus validus: %s", argv[i]);
                return 2;
            }
            out->habet_semen = 1;
            out->semen = (uint64_t)v;
        } else if (strcmp(a, "-h") == 0 || strcmp(a, "--help") == 0) {
            usus_imprime(argv[0]);
            return 1; /* signal ad exitum 0 */
        } else {
            QUERIMONIA("Flagulum ignotum: %s", a);
            usus_imprime(argv[0]);
            return 2;
        }
    }
    return 0;
}

/* --------------------------------------------------------------------- */
/* Praefatio — caput colloquii                                            */
/* --------------------------------------------------------------------- */

static void
praefatio_imprime(FILE *f, const StatusHildegardis *s)
{
    EFFUNDE(f, "  Hildegardis Bingensis loquitur ex cella sua in monte\n");
    EFFUNDE(f, "  Ruperti, ubi visiones Domini accepit et scripsit.\n");
    EFFUNDE(f, "  Modus initialis: %s (%s). Viriditas: %d.\n",
            nomina_modorum[s->modus_currens],
            ingenia_modorum[s->modus_currens],
            s->viriditas);
    EFFUNDE(f, "  Semen sortis: %llu.\n\n",
            (unsigned long long)s->semen);
}

/* --------------------------------------------------------------------- */
/* Functio principalis                                                    */
/* --------------------------------------------------------------------- */

int
main(int argc, char **argv)
{
    ArgumentaElecta args;
    int status_arg = argumenta_legere(argc, argv, &args);
    if (status_arg == 1)
        return 0;
    if (status_arg == 2)
        return 2;

    uint64_t semen = args.habet_semen ? args.semen : 67;
    StatusHildegardis status;
    status_initialize(&status, semen);

    praefatio_imprime(stdout, &status);

    if (args.interactivus) {
        modus_interactivus_ducere(&status);
    } else {
        colloquium_scriptum_ducere(&status, stdout);
    }

    return 0;
}
