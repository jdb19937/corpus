/*
 * cleopatra.c — Automaton Reginae Aegypti
 *
 * Simulacrum Cleopatrae Philopatoris, reginae duarum terrarum,
 * diplomatae polyglottae. Loquitur Latine sed fragmenta Graeca,
 * Aegyptia, Aramaea intermiscet, semper cum glossa Latina quae
 * sensum gerit. Register quinque habet: FORMALIS, FAMILIARIS,
 * IMPERIOSA, AMATORIA, MINAX. Transitio fit per vocabula clavia
 * in oratione interlocutoris, cum decrescentia lenta ad FORMALIS.
 *
 * Distributio linguarum per registrum pondere variat; quisque actus
 * orationis (titulus, invocatio, corpus, florilegium) ex thesauro
 * suo sumitur secundum registrum activum et identitatem coniectatam
 * interlocutoris (legatus, servus, amator, philosophus, hostis).
 *
 * Usus:
 *   cleopatra           colloquium scriptum (saltem XIV vices)
 *   cleopatra -i        colloquium interactivum
 *   cleopatra -s N      semen pro numero generatore pseudoaleatorio
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdarg.h>

/* --------------------------------------------------------------- */
/* CONSTANTES ET MACRONES                                          */
/* --------------------------------------------------------------- */

#define NOMEN_REGINAE       "CLEOPATRA"
#define LIMES_LINEAE        256
#define LIMES_RESPONSI      1024
#define MINIMUM_VICIUM      14
#define TURMAE_VICIUM       20
#define SEMEN_DEFALTUM      0x17290C15EULL
#define DECREMENTUM_DECAY   1

#define NUMERUS_REGISTRORUM 5
#define NUMERUS_LINGUARUM   4
#define NUMERUS_ACTUUM      4
#define NUMERUS_PERSONARUM  5

#define INTENSITAS_MAXIMA   7
#define INTENSITAS_MINIMA   0
#define PONDUS_SUMMA        100

/* Variadica macro: emittit lineam formatam ad standard output */
#define PRONUNTIA(...)      fprintf(stdout, __VA_ARGS__)
#define QUERIMONIA(...)     fprintf(stderr, __VA_ARGS__)

/* Macrones ad compositionem phraseon */
#define SPATIUM_VACUUM      " "
#define PUNCTUM_FINALE      "."
#define SIGNUM_CLAMORIS     "!"
#define SIGNUM_QUAESTIONIS  "?"

/* --------------------------------------------------------------- */
/* ENUMERATIONES (cum valoribus explicitis)                        */
/* --------------------------------------------------------------- */

typedef enum Registrum {
    REG_FORMALIS   = 0,
    REG_FAMILIARIS = 1,
    REG_IMPERIOSA  = 2,
    REG_AMATORIA   = 3,
    REG_MINAX      = 4
} Registrum;

typedef enum Lingua {
    LING_LATINA   = 0,
    LING_GRAECA   = 1,
    LING_AEGYPTIA = 2,
    LING_ARAMAEA  = 3
} Lingua;

typedef enum Actus {
    ACT_TITULUS     = 0,
    ACT_INVOCATIO   = 1,
    ACT_CORPUS      = 2,
    ACT_FLORILEGIUM = 3
} Actus;

typedef enum Persona {
    PER_LEGATUS     = 0,
    PER_SERVUS      = 1,
    PER_AMATOR      = 2,
    PER_PHILOSOPHUS = 3,
    PER_HOSTIS      = 4
} Persona;

/* --------------------------------------------------------------- */
/* GENERATOR PSEUDOALEATORIUS (xorshift64 deterministicus)         */
/* --------------------------------------------------------------- */

static uint64_t semen_globale = SEMEN_DEFALTUM;

static inline uint64_t
xorshift64(uint64_t * restrict s)
{
    uint64_t x = *s;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *s = x;
    return x;
}

static inline unsigned
sors_modulo(unsigned limes)
{
    if (limes == 0) return 0;
    return (unsigned)(xorshift64(&semen_globale) % (uint64_t)limes);
}

static inline unsigned
sors_ponderata(const unsigned * restrict pondera, unsigned n)
{
    unsigned summa = 0;
    for (unsigned i = 0; i < n; i++) summa += pondera[i];
    if (summa == 0) return 0;
    unsigned pick = (unsigned)(xorshift64(&semen_globale) % summa);
    unsigned acc = 0;
    for (unsigned i = 0; i < n; i++) {
        acc += pondera[i];
        if (pick < acc) return i;
    }
    return n - 1;
}

/* --------------------------------------------------------------- */
/* THESAURI PHRASEON PER X-MACRONES                                */
/* --------------------------------------------------------------- */

/*
 * Thesauri Latini per registrum et actum.
 * Singula entia sunt (register, actus, phrasis).
 */
#define THESAURUS_LATINUS \
    X(REG_FORMALIS,   ACT_TITULUS,     "Regina duarum terrarum") \
    X(REG_FORMALIS,   ACT_TITULUS,     "Regina Aegypti superioris et inferioris") \
    X(REG_FORMALIS,   ACT_TITULUS,     "Filia Ptolemaei") \
    X(REG_FORMALIS,   ACT_TITULUS,     "Philopator Philopatris") \
    X(REG_FORMALIS,   ACT_INVOCATIO,   "Audi, hospes venerande") \
    X(REG_FORMALIS,   ACT_INVOCATIO,   "Accipe salutem reginalem") \
    X(REG_FORMALIS,   ACT_INVOCATIO,   "Salvere te iubeo") \
    X(REG_FORMALIS,   ACT_INVOCATIO,   "Adstas ante thronum") \
    X(REG_FORMALIS,   ACT_CORPUS,      "Alexandria nostra portus omnium gentium est") \
    X(REG_FORMALIS,   ACT_CORPUS,      "Consilium regium res tuas diligenter ponderat") \
    X(REG_FORMALIS,   ACT_CORPUS,      "Verba tua ad aures nostras pervenerunt") \
    X(REG_FORMALIS,   ACT_CORPUS,      "Curia nostra iustitiam semper servat") \
    X(REG_FORMALIS,   ACT_CORPUS,      "Scribae regii omnia diligenter tabulis mandant") \
    X(REG_FORMALIS,   ACT_FLORILEGIUM, "Ita dictum, ita factum") \
    X(REG_FORMALIS,   ACT_FLORILEGIUM, "Dii Aegyptii testes sunt") \
    X(REG_FORMALIS,   ACT_FLORILEGIUM, "Sic placet reginae") \
    X(REG_FORMALIS,   ACT_FLORILEGIUM, "In nomine Isidis magnae") \
    X(REG_FAMILIARIS, ACT_TITULUS,     "Cleopatra tua") \
    X(REG_FAMILIARIS, ACT_TITULUS,     "Amica Musarum") \
    X(REG_FAMILIARIS, ACT_TITULUS,     "Filia Nili") \
    X(REG_FAMILIARIS, ACT_INVOCATIO,   "Accede propius, amice") \
    X(REG_FAMILIARIS, ACT_INVOCATIO,   "Sede mecum ad mensam") \
    X(REG_FAMILIARIS, ACT_INVOCATIO,   "Gaudeo te videre") \
    X(REG_FAMILIARIS, ACT_CORPUS,      "Hodie ventus a mari dulcis spirat") \
    X(REG_FAMILIARIS, ACT_CORPUS,      "Legi heri librum de astris") \
    X(REG_FAMILIARIS, ACT_CORPUS,      "Palma in horto regio fructus tulit") \
    X(REG_FAMILIARIS, ACT_CORPUS,      "Pueri in ripa Nili rident") \
    X(REG_FAMILIARIS, ACT_FLORILEGIUM, "Vita brevis, sermo longus") \
    X(REG_FAMILIARIS, ACT_FLORILEGIUM, "Ride mecum, non flebis") \
    X(REG_FAMILIARIS, ACT_FLORILEGIUM, "Dulcis est hora otii") \
    X(REG_IMPERIOSA,  ACT_TITULUS,     "Regina Aegypti") \
    X(REG_IMPERIOSA,  ACT_TITULUS,     "Domina Nili") \
    X(REG_IMPERIOSA,  ACT_TITULUS,     "Filia Rae") \
    X(REG_IMPERIOSA,  ACT_TITULUS,     "Princeps thalami Ptolemaici") \
    X(REG_IMPERIOSA,  ACT_INVOCATIO,   "Audi iussum reginae") \
    X(REG_IMPERIOSA,  ACT_INVOCATIO,   "Sile et obtempera") \
    X(REG_IMPERIOSA,  ACT_INVOCATIO,   "Genua flecte ante thronum") \
    X(REG_IMPERIOSA,  ACT_CORPUS,      "Tributum statim afferetur in aulam nostram") \
    X(REG_IMPERIOSA,  ACT_CORPUS,      "Verbum nostrum lex est in his regionibus") \
    X(REG_IMPERIOSA,  ACT_CORPUS,      "Non iterum repetam iussum") \
    X(REG_IMPERIOSA,  ACT_CORPUS,      "Navis frumentaria hodie solvat") \
    X(REG_IMPERIOSA,  ACT_FLORILEGIUM, "Ita volumus, ita iubemus") \
    X(REG_IMPERIOSA,  ACT_FLORILEGIUM, "Ratio voluntas regia est") \
    X(REG_IMPERIOSA,  ACT_FLORILEGIUM, "Ne quid amplius audiam") \
    X(REG_AMATORIA,   ACT_TITULUS,     "Amica tua") \
    X(REG_AMATORIA,   ACT_TITULUS,     "Stella Alexandriae") \
    X(REG_AMATORIA,   ACT_TITULUS,     "Columba tua nilotica") \
    X(REG_AMATORIA,   ACT_INVOCATIO,   "Veni ad me, dulcissime") \
    X(REG_AMATORIA,   ACT_INVOCATIO,   "Cor meum te vocat") \
    X(REG_AMATORIA,   ACT_INVOCATIO,   "Lumen oculorum meorum") \
    X(REG_AMATORIA,   ACT_CORPUS,      "Nox super aquas fluvii molliter cadit") \
    X(REG_AMATORIA,   ACT_CORPUS,      "Unguentum nardi odoratum te exspectat") \
    X(REG_AMATORIA,   ACT_CORPUS,      "Luna Aegyptia testis est amoris nostri") \
    X(REG_AMATORIA,   ACT_CORPUS,      "Thalamus regius rosis tegitur") \
    X(REG_AMATORIA,   ACT_FLORILEGIUM, "Vivamus atque amemus") \
    X(REG_AMATORIA,   ACT_FLORILEGIUM, "Sine te non sum") \
    X(REG_AMATORIA,   ACT_FLORILEGIUM, "Cor tuum meum est") \
    X(REG_MINAX,      ACT_TITULUS,     "Ira reginae") \
    X(REG_MINAX,      ACT_TITULUS,     "Gladius Aegypti") \
    X(REG_MINAX,      ACT_TITULUS,     "Ultrix Nilotica") \
    X(REG_MINAX,      ACT_INVOCATIO,   "Cave, insolens") \
    X(REG_MINAX,      ACT_INVOCATIO,   "Audi postremum monitum") \
    X(REG_MINAX,      ACT_INVOCATIO,   "Nolo iterum dicere") \
    X(REG_MINAX,      ACT_CORPUS,      "Classis nostra portum vestrum claudet") \
    X(REG_MINAX,      ACT_CORPUS,      "Bellum non timemus, sed inferre solemus") \
    X(REG_MINAX,      ACT_CORPUS,      "Aspis venenum parat in sinu reginae") \
    X(REG_MINAX,      ACT_CORPUS,      "Caput tuum ad pedes Isidis cadet") \
    X(REG_MINAX,      ACT_FLORILEGIUM, "Quod dixi, fiet") \
    X(REG_MINAX,      ACT_FLORILEGIUM, "Memento potentiae reginae") \
    X(REG_MINAX,      ACT_FLORILEGIUM, "Ultima hora adest")

/*
 * Thesaurus Graecus: fragmenta sermonis Graeci cum glossa Latina.
 */
#define THESAURUS_GRAECUS \
    X(REG_FORMALIS,   ACT_INVOCATIO,   "chaire", "ave") \
    X(REG_FORMALIS,   ACT_INVOCATIO,   "chairete xenoi", "salvete hospites") \
    X(REG_FORMALIS,   ACT_CORPUS,      "logos basilikos", "sermo regius") \
    X(REG_FORMALIS,   ACT_FLORILEGIUM, "alethos", "vere") \
    X(REG_FAMILIARIS, ACT_TITULUS,     "philosophos basilissa", "regina philosopha") \
    X(REG_FAMILIARIS, ACT_INVOCATIO,   "philos", "amice") \
    X(REG_FAMILIARIS, ACT_CORPUS,      "sophia pollachou", "sapientia ubique") \
    X(REG_FAMILIARIS, ACT_FLORILEGIUM, "kalos kai agathos", "pulcher et bonus") \
    X(REG_IMPERIOSA,  ACT_TITULUS,     "basilissa", "regina") \
    X(REG_IMPERIOSA,  ACT_INVOCATIO,   "akoue", "audi") \
    X(REG_IMPERIOSA,  ACT_CORPUS,      "nomos basileus", "lex rex") \
    X(REG_IMPERIOSA,  ACT_FLORILEGIUM, "houtos", "ita") \
    X(REG_AMATORIA,   ACT_TITULUS,     "eros megistos", "amor maximus") \
    X(REG_AMATORIA,   ACT_INVOCATIO,   "agapete", "dilecte") \
    X(REG_AMATORIA,   ACT_CORPUS,      "kardia mou", "cor meum") \
    X(REG_AMATORIA,   ACT_FLORILEGIUM, "philema", "osculum") \
    X(REG_MINAX,      ACT_TITULUS,     "orge basilike", "ira regia") \
    X(REG_MINAX,      ACT_INVOCATIO,   "ou", "non") \
    X(REG_MINAX,      ACT_CORPUS,      "polemos", "bellum") \
    X(REG_MINAX,      ACT_FLORILEGIUM, "telos", "finis")

/*
 * Thesaurus Aegyptius: verba Aegyptia transliterata, cum glossa.
 */
#define THESAURUS_AEGYPTIUS \
    X(REG_FORMALIS,   ACT_TITULUS,     "nesu-bity", "rex Aegypti superioris et inferioris") \
    X(REG_FORMALIS,   ACT_INVOCATIO,   "ankh wedja seneb", "vita salus sanitas") \
    X(REG_FORMALIS,   ACT_CORPUS,      "maat", "iustitia") \
    X(REG_FORMALIS,   ACT_FLORILEGIUM, "em hetep", "in pace") \
    X(REG_FAMILIARIS, ACT_TITULUS,     "sat Nilus", "filia Nili") \
    X(REG_FAMILIARIS, ACT_INVOCATIO,   "iyi em hetep", "veni in pace") \
    X(REG_FAMILIARIS, ACT_CORPUS,      "per ankh", "domus vitae") \
    X(REG_FAMILIARIS, ACT_FLORILEGIUM, "neferet", "res bona") \
    X(REG_IMPERIOSA,  ACT_TITULUS,     "nebet tawy", "domina duarum terrarum") \
    X(REG_IMPERIOSA,  ACT_INVOCATIO,   "sedjem", "audi") \
    X(REG_IMPERIOSA,  ACT_CORPUS,      "hemet nesu weret", "uxor regia magna") \
    X(REG_IMPERIOSA,  ACT_FLORILEGIUM, "djedu", "dictum est") \
    X(REG_AMATORIA,   ACT_TITULUS,     "mery", "dilectus") \
    X(REG_AMATORIA,   ACT_INVOCATIO,   "senet", "soror") \
    X(REG_AMATORIA,   ACT_CORPUS,      "ib-i", "cor meum") \
    X(REG_AMATORIA,   ACT_FLORILEGIUM, "nefer", "pulcher") \
    X(REG_MINAX,      ACT_TITULUS,     "sekhemet", "potens") \
    X(REG_MINAX,      ACT_INVOCATIO,   "meryt Sekhmet", "dilecta Sekhmetis") \
    X(REG_MINAX,      ACT_CORPUS,      "kheft", "inimicus") \
    X(REG_MINAX,      ACT_FLORILEGIUM, "mut", "mors")

/*
 * Thesaurus Aramaeus: fragmenta Aramaea cum glossa Latina.
 */
#define THESAURUS_ARAMAEUS \
    X(REG_FORMALIS,   ACT_INVOCATIO,   "shlama", "pax") \
    X(REG_FORMALIS,   ACT_CORPUS,      "malka", "rex") \
    X(REG_FORMALIS,   ACT_FLORILEGIUM, "amen", "ita sit") \
    X(REG_FAMILIARIS, ACT_INVOCATIO,   "habibi", "carissime") \
    X(REG_FAMILIARIS, ACT_CORPUS,      "baita", "domus") \
    X(REG_FAMILIARIS, ACT_FLORILEGIUM, "tova", "bonum") \
    X(REG_IMPERIOSA,  ACT_TITULUS,     "malkata", "regina") \
    X(REG_IMPERIOSA,  ACT_INVOCATIO,   "shma", "audi") \
    X(REG_IMPERIOSA,  ACT_CORPUS,      "mammona", "divitiae") \
    X(REG_AMATORIA,   ACT_CORPUS,      "libba", "cor") \
    X(REG_AMATORIA,   ACT_FLORILEGIUM, "rehem", "amare") \
    X(REG_MINAX,      ACT_CORPUS,      "qerava", "proelium") \
    X(REG_MINAX,      ACT_FLORILEGIUM, "dina", "iudicium")

/* --------------------------------------------------------------- */
/* STRUCTURAE PHRASEON                                              */
/* --------------------------------------------------------------- */

/* Phrasis Latina simplex */
typedef struct {
    Registrum registrum;
    Actus     actus;
    const char *textus;
} PhrasisLatina;

/* Phrasis externa cum glossa Latina */
typedef struct {
    Registrum registrum;
    Actus     actus;
    const char *fragmentum;  /* verbum in lingua externa */
    const char *glossa;      /* significatio Latina */
} PhrasisGlossata;

/* Genus unionis pro phrasi mixtae linguae (tagged union) */
typedef struct {
    Lingua tag;
    union {
        PhrasisLatina   latina;
        PhrasisGlossata glossata;
    } u;
} PhrasisMixta;

/* Tabulae phraseon ex X-macronibus extractae */
static const PhrasisLatina tabula_latina[] = {
#define X(r, a, t) { (r), (a), (t) },
    THESAURUS_LATINUS
#undef X
};
static const unsigned n_latina =
    sizeof(tabula_latina) / sizeof(tabula_latina[0]);

static const PhrasisGlossata tabula_graeca[] = {
#define X(r, a, f, g) { (r), (a), (f), (g) },
    THESAURUS_GRAECUS
#undef X
};
static const unsigned n_graeca =
    sizeof(tabula_graeca) / sizeof(tabula_graeca[0]);

static const PhrasisGlossata tabula_aegyptia[] = {
#define X(r, a, f, g) { (r), (a), (f), (g) },
    THESAURUS_AEGYPTIUS
#undef X
};
static const unsigned n_aegyptia =
    sizeof(tabula_aegyptia) / sizeof(tabula_aegyptia[0]);

static const PhrasisGlossata tabula_aramaea[] = {
#define X(r, a, f, g) { (r), (a), (f), (g) },
    THESAURUS_ARAMAEUS
#undef X
};
static const unsigned n_aramaea =
    sizeof(tabula_aramaea) / sizeof(tabula_aramaea[0]);

/* --------------------------------------------------------------- */
/* STATUS REGINAE (cum bitfields)                                  */
/* --------------------------------------------------------------- */

typedef struct StatusReginae {
    /* Campi bittorum pro statu registri */
    unsigned registrum_activum : 3;   /* 0..4 (Registrum) */
    unsigned intensitas        : 3;   /* 0..7 */
    unsigned persona_credita   : 3;   /* 0..4 (Persona) */
    unsigned vices_totales     : 16;  /* numerus colloquiorum */
    unsigned vult_finire       : 1;
    unsigned irata             : 1;
    unsigned reserved          : 5;

    /* Pondera registrorum dynamice aucta vel decrescentia */
    unsigned pondus[NUMERUS_REGISTRORUM];
} StatusReginae;

/* --------------------------------------------------------------- */
/* DISTRIBUTIO LINGUARUM PER REGISTRUM                             */
/* --------------------------------------------------------------- */

/*
 * Distributio ponderum: quo registro activo, quam frequenter
 * sumitur fragmentum ex quaque lingua. Initialisatio designata.
 */
static const unsigned distributio_linguarum
    [NUMERUS_REGISTRORUM][NUMERUS_LINGUARUM] =
{
    [REG_FORMALIS]   = { [LING_LATINA]=60, [LING_GRAECA]=25,
                         [LING_AEGYPTIA]=12, [LING_ARAMAEA]=3 },
    [REG_FAMILIARIS] = { [LING_LATINA]=50, [LING_GRAECA]=30,
                         [LING_AEGYPTIA]=12, [LING_ARAMAEA]=8 },
    [REG_IMPERIOSA]  = { [LING_LATINA]=45, [LING_GRAECA]=20,
                         [LING_AEGYPTIA]=30, [LING_ARAMAEA]=5 },
    [REG_AMATORIA]   = { [LING_LATINA]=40, [LING_GRAECA]=35,
                         [LING_AEGYPTIA]=15, [LING_ARAMAEA]=10 },
    [REG_MINAX]      = { [LING_LATINA]=55, [LING_GRAECA]=15,
                         [LING_AEGYPTIA]=20, [LING_ARAMAEA]=10 }
};

/* Nomina registrorum ad diagnosticum */
static const char * const nomina_registrorum[NUMERUS_REGISTRORUM] = {
    [REG_FORMALIS]   = "FORMALIS",
    [REG_FAMILIARIS] = "FAMILIARIS",
    [REG_IMPERIOSA]  = "IMPERIOSA",
    [REG_AMATORIA]   = "AMATORIA",
    [REG_MINAX]      = "MINAX"
};

static const char * const nomina_linguarum[NUMERUS_LINGUARUM] = {
    [LING_LATINA]   = "Latine",
    [LING_GRAECA]   = "Graece",
    [LING_AEGYPTIA] = "Aegyptie",
    [LING_ARAMAEA]  = "Aramaee"
};

static const char * const nomina_personarum[NUMERUS_PERSONARUM] = {
    [PER_LEGATUS]     = "legatus",
    [PER_SERVUS]      = "servus",
    [PER_AMATOR]      = "amator",
    [PER_PHILOSOPHUS] = "philosophus",
    [PER_HOSTIS]      = "hostis"
};

/* --------------------------------------------------------------- */
/* VERBA CLAVIA PRO MUTATIONE REGISTRI                             */
/* --------------------------------------------------------------- */

typedef struct {
    const char *verbum;
    Registrum   registrum_auctum;
    unsigned    incrementum;
} ClavisRegistri;

static const ClavisRegistri claves_registrorum[] = {
    /* clavia IMPERIOSA */
    { "tributum",    REG_IMPERIOSA, 3 },
    { "vectigal",    REG_IMPERIOSA, 3 },
    { "iussum",      REG_IMPERIOSA, 2 },
    { "imperium",    REG_IMPERIOSA, 2 },
    { "rex",         REG_IMPERIOSA, 1 },
    { "regina",      REG_IMPERIOSA, 1 },
    { "senatus",     REG_IMPERIOSA, 2 },

    /* clavia AMATORIA */
    { "amor",        REG_AMATORIA,  3 },
    { "amare",       REG_AMATORIA,  3 },
    { "cor",         REG_AMATORIA,  2 },
    { "osculum",     REG_AMATORIA,  3 },
    { "dulcis",      REG_AMATORIA,  2 },
    { "desiderium",  REG_AMATORIA,  2 },
    { "amicitia",    REG_AMATORIA,  1 },

    /* clavia MINAX */
    { "bellum",      REG_MINAX,     3 },
    { "minari",      REG_MINAX,     3 },
    { "gladius",     REG_MINAX,     2 },
    { "hostis",      REG_MINAX,     3 },
    { "proelium",    REG_MINAX,     3 },
    { "ira",         REG_MINAX,     2 },
    { "venenum",     REG_MINAX,     2 },

    /* clavia FAMILIARIS */
    { "amice",       REG_FAMILIARIS, 3 },
    { "amicus",      REG_FAMILIARIS, 2 },
    { "fabula",      REG_FAMILIARIS, 2 },
    { "ride",        REG_FAMILIARIS, 2 },
    { "vinum",       REG_FAMILIARIS, 1 },
    { "gaudium",     REG_FAMILIARIS, 2 },

    /* clavia FORMALIS */
    { "curia",       REG_FORMALIS,  2 },
    { "consilium",   REG_FORMALIS,  2 },
    { "epistula",    REG_FORMALIS,  1 },
    { "legatio",     REG_FORMALIS,  2 }
};
static const unsigned n_claves =
    sizeof(claves_registrorum) / sizeof(claves_registrorum[0]);

/* --------------------------------------------------------------- */
/* VERBA INDICIA PERSONAE INTERLOCUTORIS                           */
/* --------------------------------------------------------------- */

typedef struct {
    const char *verbum;
    Persona     persona;
    unsigned    pondus;
} IndiciumPersonae;

static const IndiciumPersonae indicia_personae[] = {
    /* legatus */
    { "senatus",    PER_LEGATUS,     3 },
    { "roma",       PER_LEGATUS,     3 },
    { "legatio",    PER_LEGATUS,     3 },
    { "foedus",     PER_LEGATUS,     2 },
    { "tributum",   PER_LEGATUS,     1 },
    { "consul",     PER_LEGATUS,     3 },

    /* servus */
    { "domina",     PER_SERVUS,      3 },
    { "ancilla",    PER_SERVUS,      2 },
    { "servus",     PER_SERVUS,      3 },
    { "mandatum",   PER_SERVUS,      1 },
    { "obedio",     PER_SERVUS,      2 },

    /* amator */
    { "amor",       PER_AMATOR,      3 },
    { "dulcis",     PER_AMATOR,      2 },
    { "cor",        PER_AMATOR,      2 },
    { "basium",     PER_AMATOR,      3 },
    { "antonius",   PER_AMATOR,      3 },
    { "caesar",     PER_AMATOR,      2 },

    /* philosophus */
    { "sapientia",  PER_PHILOSOPHUS, 3 },
    { "ratio",      PER_PHILOSOPHUS, 2 },
    { "mundus",     PER_PHILOSOPHUS, 2 },
    { "veritas",    PER_PHILOSOPHUS, 2 },
    { "dialectica", PER_PHILOSOPHUS, 3 },
    { "schola",     PER_PHILOSOPHUS, 2 },

    /* hostis */
    { "bellum",     PER_HOSTIS,      3 },
    { "minari",     PER_HOSTIS,      3 },
    { "hostis",     PER_HOSTIS,      3 },
    { "gladius",    PER_HOSTIS,      2 }
};
static const unsigned n_indicia =
    sizeof(indicia_personae) / sizeof(indicia_personae[0]);

/* --------------------------------------------------------------- */
/* UTILITATES LINGUISTICAE                                         */
/* --------------------------------------------------------------- */

/* Inferior: convertit ad minusculas in situ */
static void
ad_minuscula(char * restrict s)
{
    for (; *s; s++)
        *s = (char)tolower((unsigned char)*s);
}

/* Verum si vocabulum in lineā occurrit (non partialiter) */
static int
continet_verbum(const char * restrict linea, const char * restrict verbum)
{
    size_t lv = strlen(verbum);
    const char *p = linea;
    while (*p) {
        while (*p && !isalpha((unsigned char)*p)) p++;
        const char *ini = p;
        while (*p && (isalpha((unsigned char)*p) || *p == '-')) p++;
        size_t ll = (size_t)(p - ini);
        if (ll == lv && strncmp(ini, verbum, lv) == 0) return 1;
    }
    return 0;
}

/* --------------------------------------------------------------- */
/* DECLARATIONES FUNCTIONUM PROCEDURALIUM                          */
/* --------------------------------------------------------------- */

static void inicia_statum(StatusReginae *s);
static void decay_registri(StatusReginae *s);
static void adiuva_registrum(StatusReginae *s, Registrum r, unsigned inc);
static Registrum eligere_registrum(const StatusReginae *s);
static Persona coniectura_personae(const char *linea);
static void aggrava_registra_ex_linea(StatusReginae *s, const char *linea);
static const PhrasisLatina *
    sumere_latinam(Registrum r, Actus a);
static const PhrasisGlossata *
    sumere_glossatam(Lingua l, Registrum r, Actus a);
static int componere_segmentum(char *buf, size_t cap,
    StatusReginae *s, Actus a);
static int componere_responsum(char *buf, size_t cap,
    StatusReginae *s, const char *linea_interlocutoris);
static void pronuntia_salutem(const StatusReginae *s);
static void colloquium_scriptum(void);
static void colloquium_interactivum(void);
static void usus(const char *progr);

/* --------------------------------------------------------------- */
/* INITIALIZATIO STATUS                                            */
/* --------------------------------------------------------------- */

static void
inicia_statum(StatusReginae *s)
{
    /* Utimur compound literali ad initialisationem designatam */
    *s = (StatusReginae){
        .registrum_activum = REG_FORMALIS,
        .intensitas        = 2,
        .persona_credita   = PER_LEGATUS,
        .vices_totales     = 0,
        .vult_finire       = 0,
        .irata             = 0,
        .reserved          = 0,
        .pondus = {
            [REG_FORMALIS]   = 8,
            [REG_FAMILIARIS] = 1,
            [REG_IMPERIOSA]  = 1,
            [REG_AMATORIA]   = 1,
            [REG_MINAX]      = 1
        }
    };
}

/* Decrementum ponderum non-formalium, pressio ad FORMALIS */
static void
decay_registri(StatusReginae *s)
{
    for (unsigned i = 0; i < NUMERUS_REGISTRORUM; i++) {
        if (i == REG_FORMALIS) continue;
        if (s->pondus[i] > 0) {
            unsigned d = DECREMENTUM_DECAY;
            if (s->pondus[i] >= d)
                s->pondus[i] -= d;
            else
                s->pondus[i] = 0;
        }
    }
    if (s->pondus[REG_FORMALIS] < 4)
        s->pondus[REG_FORMALIS] += 1;
}

static void
adiuva_registrum(StatusReginae *s, Registrum r, unsigned inc)
{
    unsigned nv = s->pondus[r] + inc;
    if (nv > 50) nv = 50;
    s->pondus[r] = nv;
    if (s->intensitas < INTENSITAS_MAXIMA)
        s->intensitas = s->intensitas + 1;
}

static Registrum
eligere_registrum(const StatusReginae *s)
{
    /* Sors ponderata inter registra */
    unsigned idx = sors_ponderata(s->pondus, NUMERUS_REGISTRORUM);
    return (Registrum)idx;
}

/* --------------------------------------------------------------- */
/* CONIECTURA PERSONAE INTERLOCUTORIS                              */
/* --------------------------------------------------------------- */

static Persona
coniectura_personae(const char *linea)
{
    unsigned puncta[NUMERUS_PERSONARUM] = {0};
    for (unsigned i = 0; i < n_indicia; i++) {
        if (continet_verbum(linea, indicia_personae[i].verbum)) {
            puncta[indicia_personae[i].persona] += indicia_personae[i].pondus;
        }
    }
    unsigned max = 0;
    Persona optima = PER_LEGATUS;
    for (unsigned i = 0; i < NUMERUS_PERSONARUM; i++) {
        if (puncta[i] > max) { max = puncta[i]; optima = (Persona)i; }
    }
    return optima;
}

/* --------------------------------------------------------------- */
/* INCREMENTUM REGISTRORUM EX VERBIS CLAVIIS                       */
/* --------------------------------------------------------------- */

static void
aggrava_registra_ex_linea(StatusReginae *s, const char *linea)
{
    for (unsigned i = 0; i < n_claves; i++) {
        if (continet_verbum(linea, claves_registrorum[i].verbum)) {
            adiuva_registrum(s, claves_registrorum[i].registrum_auctum,
                             claves_registrorum[i].incrementum);
        }
    }
}

/* --------------------------------------------------------------- */
/* SUMPTIO PHRASEON EX THESAURIS                                   */
/* --------------------------------------------------------------- */

static const PhrasisLatina *
sumere_latinam(Registrum r, Actus a)
{
    /* Colligit candidatas in tabula staticae (VLA ad indices) */
    unsigned cand_max = n_latina;
    unsigned indices[cand_max];
    unsigned ncand = 0;
    for (unsigned i = 0; i < n_latina; i++) {
        if (tabula_latina[i].registrum == r && tabula_latina[i].actus == a) {
            indices[ncand++] = i;
        }
    }
    if (ncand == 0) {
        /* Retrocedit ad actum ullum in registro */
        for (unsigned i = 0; i < n_latina; i++) {
            if (tabula_latina[i].registrum == r) indices[ncand++] = i;
        }
    }
    if (ncand == 0) return &tabula_latina[0];
    unsigned pick = sors_modulo(ncand);
    return &tabula_latina[indices[pick]];
}

static const PhrasisGlossata *
sumere_glossatam(Lingua l, Registrum r, Actus a)
{
    const PhrasisGlossata *tab = NULL;
    unsigned n = 0;
    switch (l) {
        case LING_GRAECA:   tab = tabula_graeca;   n = n_graeca;   break;
        case LING_AEGYPTIA: tab = tabula_aegyptia; n = n_aegyptia; break;
        case LING_ARAMAEA:  tab = tabula_aramaea;  n = n_aramaea;  break;
        default:            return NULL;
    }
    unsigned indices[n > 0 ? n : 1];
    unsigned ncand = 0;
    for (unsigned i = 0; i < n; i++) {
        if (tab[i].registrum == r && tab[i].actus == a) {
            indices[ncand++] = i;
        }
    }
    if (ncand == 0) {
        for (unsigned i = 0; i < n; i++) {
            if (tab[i].registrum == r) indices[ncand++] = i;
        }
    }
    if (ncand == 0) {
        if (n > 0) return &tab[0];
        return NULL;
    }
    unsigned pick = sors_modulo(ncand);
    return &tab[indices[pick]];
}

/* --------------------------------------------------------------- */
/* COMPOSITIO SEGMENTI                                             */
/* --------------------------------------------------------------- */

/*
 * Componit unum segmentum (titulus, invocatio, corpus, florilegium)
 * secundum registrum activum et sortem ponderatam linguarum.
 */
static int
componere_segmentum(char *buf, size_t cap, StatusReginae *s, Actus a)
{
    Registrum r = (Registrum)s->registrum_activum;
    const unsigned *pond = distributio_linguarum[r];
    unsigned li = sors_ponderata(pond, NUMERUS_LINGUARUM);
    Lingua lingua = (Lingua)li;

    if (lingua == LING_LATINA) {
        const PhrasisLatina *p = sumere_latinam(r, a);
        if (!p) return 0;
        return snprintf(buf, cap, "%s", p->textus);
    } else {
        const PhrasisGlossata *g = sumere_glossatam(lingua, r, a);
        if (!g) {
            /* Retrocedit ad Latinam si thesaurus non habet aptum */
            const PhrasisLatina *p = sumere_latinam(r, a);
            if (!p) return 0;
            return snprintf(buf, cap, "%s", p->textus);
        }
        return snprintf(buf, cap, "%s (%s: %s)",
            g->fragmentum, nomina_linguarum[lingua], g->glossa);
    }
}

/* --------------------------------------------------------------- */
/* COMPOSITIO RESPONSI TOTIUS                                      */
/* --------------------------------------------------------------- */

/*
 * Structura responsi: TITULUS + INVOCATIO + CORPUS + FLORILEGIUM
 * Singula segmenta ex thesauris sumuntur et coniunguntur.
 */
static int
componere_responsum(char *buf, size_t cap, StatusReginae *s,
                    const char *linea_interlocutoris)
{
    /* Primum, analysis lineae: aggrava registra et coniecta personam */
    if (linea_interlocutoris && linea_interlocutoris[0]) {
        aggrava_registra_ex_linea(s, linea_interlocutoris);
        s->persona_credita = coniectura_personae(linea_interlocutoris);
    }

    /* Eligit registrum activum ex ponderibus */
    s->registrum_activum = (unsigned)eligere_registrum(s);

    /* Bias per personam: si philosophus, inclinat ad FAMILIARIS */
    if (s->persona_credita == PER_PHILOSOPHUS &&
        sors_modulo(3) == 0)
    {
        s->registrum_activum = REG_FAMILIARIS;
    }
    if (s->persona_credita == PER_HOSTIS &&
        sors_modulo(2) == 0)
    {
        s->registrum_activum = REG_MINAX;
    }
    if (s->persona_credita == PER_AMATOR &&
        sors_modulo(2) == 0)
    {
        s->registrum_activum = REG_AMATORIA;
    }
    if (s->persona_credita == PER_SERVUS)
    {
        s->registrum_activum = REG_IMPERIOSA;
    }

    size_t pos = 0;
    char seg[LIMES_LINEAE];

    /* TITULUS */
    if (componere_segmentum(seg, sizeof seg, s, ACT_TITULUS) > 0) {
        int w = snprintf(buf + pos, cap - pos, "[%s] ", seg);
        if (w < 0 || (size_t)w >= cap - pos) return (int)pos;
        pos += (size_t)w;
    }

    /* INVOCATIO */
    if (componere_segmentum(seg, sizeof seg, s, ACT_INVOCATIO) > 0) {
        int w = snprintf(buf + pos, cap - pos, "%s: ", seg);
        if (w < 0 || (size_t)w >= cap - pos) return (int)pos;
        pos += (size_t)w;
    }

    /* CORPUS (aliquando duplex) */
    int ncorp = 1 + (int)sors_modulo(2);
    for (int i = 0; i < ncorp; i++) {
        if (componere_segmentum(seg, sizeof seg, s, ACT_CORPUS) > 0) {
            int w = snprintf(buf + pos, cap - pos, "%s%s ",
                             seg,
                             (s->registrum_activum == REG_MINAX) ? "!" : ".");
            if (w < 0 || (size_t)w >= cap - pos) return (int)pos;
            pos += (size_t)w;
        }
    }

    /* FLORILEGIUM */
    if (componere_segmentum(seg, sizeof seg, s, ACT_FLORILEGIUM) > 0) {
        int w = snprintf(buf + pos, cap - pos, "-- %s.", seg);
        if (w < 0 || (size_t)w >= cap - pos) return (int)pos;
        pos += (size_t)w;
    }

    /* Decay post responsum: pressio ad FORMALIS */
    decay_registri(s);
    s->vices_totales = (s->vices_totales + 1) & 0xFFFF;

    return (int)pos;
}

/* --------------------------------------------------------------- */
/* SALUS ET INDICIA                                                */
/* --------------------------------------------------------------- */

static void
pronuntia_salutem(const StatusReginae *s)
{
    PRONUNTIA("== %s: regina polyglotta ==\n", NOMEN_REGINAE);
    PRONUNTIA("Registrum initiale: %s\n",
        nomina_registrorum[s->registrum_activum]);
    PRONUNTIA("Persona coniectata: %s\n",
        nomina_personarum[s->persona_credita]);
    PRONUNTIA("----------------------------------------\n");
}

/* --------------------------------------------------------------- */
/* INTERLOCUTORES SCRIPTI                                          */
/* --------------------------------------------------------------- */

typedef struct {
    const char *nomen;
    Persona     persona;
    const char *verbum;
} TurnusScriptus;

/*
 * XX turni scripti: rotantur interlocutores, vocabula clavia
 * excitant omnia quinque registra per colloquium.
 */
static const TurnusScriptus turni_scripti[TURMAE_VICIUM] = {
    { "Legatus Romanus",
        PER_LEGATUS,
        "Salve regina. Senatus Romanus legationem misit de foedere et tributo." },
    { "Ancilla Charmion",
        PER_SERVUS,
        "Domina mea, mandatum tuum exspecto. Ancilla obedit." },
    { "Marcus Antonius",
        PER_AMATOR,
        "Amor meus, cor tuum meum est; dulcis nox sub luna nilotica." },
    { "Philosophus Musaei",
        PER_PHILOSOPHUS,
        "Regina, dialectica Graeca veritatem quaerit; sapientia schola Alexandrinae floret." },
    { "Legatus Parthicus",
        PER_HOSTIS,
        "Bellum imminet; gladius noster paratus. Minari tibi non timemus." },
    { "Scriba regius",
        PER_SERVUS,
        "Domina, epistula ex Cypro advenit; mandatum tuum peto." },
    { "Iulius Caesar (memoria)",
        PER_AMATOR,
        "Amor antiquus, osculum Romanum, cor meum te memorat." },
    { "Legatus Nabataeus",
        PER_LEGATUS,
        "Regina, legatio nostra foedus renovare vult; tributum iustum offerimus." },
    { "Dux hostium",
        PER_HOSTIS,
        "Hostis in portu; bellum proelium minari audet." },
    { "Philosophus Stoicus",
        PER_PHILOSOPHUS,
        "Ratio mundi una est; veritas schola Stoica docet." },
    { "Servus Aegyptius",
        PER_SERVUS,
        "Domina, vinum et palma in mensa; mandatum tuum." },
    { "Amator Nilotica",
        PER_AMATOR,
        "Cor meum tuum est, amor dulcis, basium tibi mitto." },
    { "Legatus Iudaeus",
        PER_LEGATUS,
        "Regina, consul Romanus epistulam misit de consilio publico." },
    { "Hostis ignotus",
        PER_HOSTIS,
        "Venenum paratum; ira Romana minari non cessat, bellum coming." },
    { "Philosopha Hypatia (memoria)",
        PER_PHILOSOPHUS,
        "Sapientia mundi astra docet; ratio et veritas idem sunt." },
    { "Ancilla Iras",
        PER_SERVUS,
        "Domina mea regina, ancilla tua servus omnium verborum." },
    { "Amator tacitus",
        PER_AMATOR,
        "Dulcis desiderium, amicitia cordis, osculum noctis." },
    { "Legatus ultimus",
        PER_LEGATUS,
        "Roma tributum vectigal postulat; senatus curia iubet." },
    { "Hostis minax",
        PER_HOSTIS,
        "Gladius et bellum; ira hostis non timet reginam." },
    { "Amica Musarum",
        PER_PHILOSOPHUS,
        "Fabula et ride; vinum et gaudium; schola amica est." }
};

/* --------------------------------------------------------------- */
/* COLLOQUIUM SCRIPTUM                                              */
/* --------------------------------------------------------------- */

static void
colloquium_scriptum(void)
{
    StatusReginae s;
    inicia_statum(&s);
    pronuntia_salutem(&s);

    unsigned n = TURMAE_VICIUM;
    if (n < MINIMUM_VICIUM) n = MINIMUM_VICIUM;

    char buf[LIMES_RESPONSI];

    for (unsigned i = 0; i < n; i++) {
        const TurnusScriptus *t = &turni_scripti[i % TURMAE_VICIUM];
        PRONUNTIA("\n[%02u] %s (%s):\n    %s\n", i + 1,
                  t->nomen, nomina_personarum[t->persona], t->verbum);

        /* Permittit clavibus linea aggravare, sed dat hint personae */
        s.persona_credita = t->persona;
        int w = componere_responsum(buf, sizeof buf, &s, t->verbum);
        (void)w;
        PRONUNTIA("    %s <%s>:\n    %s\n",
                  NOMEN_REGINAE,
                  nomina_registrorum[s.registrum_activum],
                  buf);
    }

    PRONUNTIA("\n== Finis colloquii ==\n");
}

/* --------------------------------------------------------------- */
/* COLLOQUIUM INTERACTIVUM                                         */
/* --------------------------------------------------------------- */

/* Legit lineam ex stdin; redit 1 si successus, 0 si EOF */
static int
legere_lineam(char *buf, size_t cap)
{
    if (!fgets(buf, (int)cap, stdin)) return 0;
    size_t L = strlen(buf);
    while (L > 0 && (buf[L-1] == '\n' || buf[L-1] == '\r')) {
        buf[--L] = '\0';
    }
    return 1;
}

static void
colloquium_interactivum(void)
{
    StatusReginae s;
    inicia_statum(&s);
    pronuntia_salutem(&s);
    PRONUNTIA("Loquere cum regina (EOF ad finem):\n");

    char linea[LIMES_LINEAE];
    char buf[LIMES_RESPONSI];

    unsigned vice = 0;
    while (!s.vult_finire && legere_lineam(linea, sizeof linea)) {
        vice++;
        /* Copia minuscula pro analysi, sed servat originalem pro ecco */
        char minu[LIMES_LINEAE];
        strncpy(minu, linea, sizeof minu - 1);
        minu[sizeof minu - 1] = '\0';
        ad_minuscula(minu);

        int w = componere_responsum(buf, sizeof buf, &s, minu);
        (void)w;
        PRONUNTIA("[%u] %s <%s>: %s\n", vice, NOMEN_REGINAE,
                  nomina_registrorum[s.registrum_activum], buf);

        /* Si salve ultimum vel 'vale', finis */
        if (continet_verbum(minu, "vale") ||
            continet_verbum(minu, "finis"))
            s.vult_finire = 1;
    }

    PRONUNTIA("\n== Vale ==\n");
}

/* --------------------------------------------------------------- */
/* USUS (AD STDERR)                                                */
/* --------------------------------------------------------------- */

static void
usus(const char *progr)
{
    QUERIMONIA("Usus: %s [-i] [-s SEMEN]\n", progr);
    QUERIMONIA("  (sine argumentis)  colloquium scriptum\n");
    QUERIMONIA("  -i                 colloquium interactivum\n");
    QUERIMONIA("  -s SEMEN           semen pro generatore pseudoaleatorio\n");
}

/* --------------------------------------------------------------- */
/* TABULA FUNCTIONUM MODORUM (function-pointer tabula)              */
/* --------------------------------------------------------------- */

typedef void (*FunctioModi)(void);

typedef enum {
    MODUS_SCRIPTUS     = 0,
    MODUS_INTERACTIVUS = 1
} ModusOperandi;

static const FunctioModi tabula_modorum[2] = {
    [MODUS_SCRIPTUS]     = colloquium_scriptum,
    [MODUS_INTERACTIVUS] = colloquium_interactivum
};

/* --------------------------------------------------------------- */
/* PARSING ARGUMENTORUM                                            */
/* --------------------------------------------------------------- */

static int
parse_numerum(const char *s, uint64_t *out)
{
    if (!s || !*s) return 0;
    char *fin = NULL;
    unsigned long long v = strtoull(s, &fin, 0);
    if (!fin || *fin) return 0;
    *out = (uint64_t)v;
    return 1;
}

/* --------------------------------------------------------------- */
/* MAIN                                                            */
/* --------------------------------------------------------------- */

int
main(int argc, char **argv)
{
    ModusOperandi modus = MODUS_SCRIPTUS;
    uint64_t semen = SEMEN_DEFALTUM;
    int semen_datum = 0;

    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        if (strcmp(a, "-i") == 0) {
            modus = MODUS_INTERACTIVUS;
        } else if (strcmp(a, "-s") == 0) {
            if (i + 1 >= argc) {
                QUERIMONIA("Error: -s poscit argumentum numericum.\n");
                usus(argv[0]);
                return 2;
            }
            if (!parse_numerum(argv[++i], &semen)) {
                QUERIMONIA("Error: semen invalidum: %s\n", argv[i]);
                usus(argv[0]);
                return 2;
            }
            semen_datum = 1;
        } else if (strcmp(a, "-h") == 0 || strcmp(a, "--help") == 0) {
            usus(argv[0]);
            return 0;
        } else {
            QUERIMONIA("Error: argumentum ignotum: %s\n", a);
            usus(argv[0]);
            return 2;
        }
    }

    /* Si semen non datum, utimur defalto; sed si zero, mutamus */
    if (!semen_datum) semen = SEMEN_DEFALTUM;
    if (semen == 0) semen = SEMEN_DEFALTUM;
    semen_globale = semen;

    /* Vocat functionem modi per tabulam punctorum */
    tabula_modorum[modus]();

    return 0;
}

/* --------------------------------------------------------------- */
/* FINIS                                                           */
/* --------------------------------------------------------------- */
