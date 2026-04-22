/*
 * nicolaus.c — Automaton Nicolai Teslae, Inventoris Resonantiae
 *
 * Confabulator proceduralis qui personam Nicolai Teslae imitatur:
 * numeros sacros trium, sex, novem veneratur; oscillatores per
 * superpositionem sinuum computat; aethera et signa Martis et
 * columbas et memorias bobinae electricae commemorat; Edisonum
 * nominis inimici vocat. Determinismus per xorshift64; semen per
 * argumentum "-s" ponitur. Sine argumentis colloquium scriptum
 * inter Nicolaum et interlocutorem proicit. Cum "-i" sessio
 * interactiva incipit. Argumentum ignotum ad usum Latinum ducit.
 *
 * Scriptor: automaton ipsum. Lingua: Latina classica. Codex: C99.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include <stdarg.h>

/* ============================================================ */
/*  CONSTANTES MACRONES                                          */
/* ============================================================ */

#define VERSIO_AUTOMATI   "NICOLAUS-III-VI-IX"
#define MAXIMUM_LINEAE    1024
#define MAXIMUM_VERBI     96
#define MAXIMUM_VERBORUM  256
#define LIMES_UNDAE       120
#define ALTITUDO_UNDAE    9
#define TRINITAS          3
#define SEXTILIS          6
#define NONARIUS          9
#define NUMERUS_SACRUS    369
#define PI_LONGUS         3.14159265358979323846L
#define SEMEN_DEFAULTUM   0x1CE1A2020369ULL
#define FREQUENTIA_BASIS  111.0L

#define MINIMUS_TURNORUM  14

#define LONGITUDO(a)      ((int)(sizeof(a)/sizeof((a)[0])))

/* Variadica pro scripto ad stdout; more Latino dicitur. */
#define ENUNTIA(...)      fprintf(stdout, __VA_ARGS__)
#define QUERELA(...)      fprintf(stderr, __VA_ARGS__)

/* Macronem pro oscillatorio emittendum. */
#define SONA_SINUM(amp, frq, phs, tmp) \
    ((long double)(amp) * sinl((long double)(frq) * (long double)(tmp) + (long double)(phs)))

/* ============================================================ */
/*  X-MACRO: INDEX TOPICORUM ELECTRICORUM                        */
/* ============================================================ */

#define TABULA_TOPICORUM                                               \
    X(TOPICUM_ELECTRICITAS,    "ELECTRICITAS",    "fluxus electricus")  \
    X(TOPICUM_MAGNETISMUS,     "MAGNETISMUS",     "campus magneticus")  \
    X(TOPICUM_RESONANTIA,      "RESONANTIA",      "resonantia cosmica") \
    X(TOPICUM_ETHER,           "ETHER",           "aether luminifer")   \
    X(TOPICUM_LUX,             "LUX",             "lumen coronale")     \
    X(TOPICUM_MORS_STELLARUM,  "MORS_STELLARUM",  "fatum astrorum")     \
    X(TOPICUM_LUNA,            "LUNA",            "luna nocturna")      \
    X(TOPICUM_MARS,            "MARS",            "sermo Martius")      \
    X(TOPICUM_PHILANTHROPIA,   "PHILANTHROPIA",   "amor hominum")       \
    X(TOPICUM_PECUNIA,         "PECUNIA",         "aes Vorangi")        \
    X(TOPICUM_EDISON,          "EDISON",          "adversarius Edisonis") \
    X(TOPICUM_IGNOTUM,         "IGNOTUM",         "silentium ambiguum")

typedef enum {
#define X(id, nomen, desc) id,
    TABULA_TOPICORUM
#undef X
    TOPICUM_TERMINATOR
} TopicumGenus;

static const char *NOMINA_TOPICORUM[] = {
#define X(id, nomen, desc) nomen,
    TABULA_TOPICORUM
#undef X
};

static const char *DESCRIPTIONES_TOPICORUM[] = {
#define X(id, nomen, desc) desc,
    TABULA_TOPICORUM
#undef X
};

/* ============================================================ */
/*  X-MACRO: VOCABULARIUM PROPRIUM NICOLAI                       */
/* ============================================================ */

#define CATALOGUS_CLAVIUM                                               \
    C(TOPICUM_ELECTRICITAS,  "electr")                                  \
    C(TOPICUM_ELECTRICITAS,  "curren")                                  \
    C(TOPICUM_ELECTRICITAS,  "alternan")                                \
    C(TOPICUM_ELECTRICITAS,  "scintil")                                 \
    C(TOPICUM_ELECTRICITAS,  "fulgur")                                  \
    C(TOPICUM_MAGNETISMUS,   "magnet")                                  \
    C(TOPICUM_MAGNETISMUS,   "campus")                                  \
    C(TOPICUM_MAGNETISMUS,   "polus")                                   \
    C(TOPICUM_RESONANTIA,    "resona")                                  \
    C(TOPICUM_RESONANTIA,    "frequen")                                 \
    C(TOPICUM_RESONANTIA,    "vibra")                                   \
    C(TOPICUM_RESONANTIA,    "oscilla")                                 \
    C(TOPICUM_RESONANTIA,    "unda")                                    \
    C(TOPICUM_ETHER,         "aether")                                  \
    C(TOPICUM_ETHER,         "ether")                                   \
    C(TOPICUM_ETHER,         "medium")                                  \
    C(TOPICUM_LUX,           "lumen")                                   \
    C(TOPICUM_LUX,           "lux")                                     \
    C(TOPICUM_LUX,           "radius")                                  \
    C(TOPICUM_MORS_STELLARUM,"stella")                                  \
    C(TOPICUM_MORS_STELLARUM,"astr")                                    \
    C(TOPICUM_MORS_STELLARUM,"cosmos")                                  \
    C(TOPICUM_LUNA,          "luna")                                    \
    C(TOPICUM_LUNA,          "nox")                                     \
    C(TOPICUM_MARS,          "mars")                                    \
    C(TOPICUM_MARS,          "marti")                                   \
    C(TOPICUM_MARS,          "planet")                                  \
    C(TOPICUM_MARS,          "signum")                                  \
    C(TOPICUM_PHILANTHROPIA, "homin")                                   \
    C(TOPICUM_PHILANTHROPIA, "human")                                   \
    C(TOPICUM_PHILANTHROPIA, "gratui")                                  \
    C(TOPICUM_PECUNIA,       "aes")                                     \
    C(TOPICUM_PECUNIA,       "pecuni")                                  \
    C(TOPICUM_PECUNIA,       "aurum")                                   \
    C(TOPICUM_PECUNIA,       "morgan")                                  \
    C(TOPICUM_EDISON,        "edison")                                  \
    C(TOPICUM_EDISON,        "directu")                                 \
    C(TOPICUM_EDISON,        "inimic")                                  \
    C(TOPICUM_EDISON,        "menlo")

/* Structura clavis-topici pro classificatione. */
typedef struct {
    TopicumGenus genus;
    const char  *radix;
} ClavisTopici;

static const ClavisTopici CLAVES[] = {
#define C(genus, radix) { genus, radix },
    CATALOGUS_CLAVIUM
#undef C
};

/* ============================================================ */
/*  STATUS AUTOMATI: UNIO PRO NUMERIS SACRIS                     */
/* ============================================================ */

/* Unio pro numeris interpretandis: et ut integer et ut campi
 * bitfieldorum. Serviet ad demonstrationem usus C99 unionis. */
typedef union {
    uint32_t totus;
    struct {
        unsigned tres   : 4;   /* digiti sacri trium        */
        unsigned sex    : 4;   /* digiti sacri sextilium    */
        unsigned novem  : 4;   /* digiti sacri nonarum      */
        unsigned radix  : 4;   /* radix digitalis 0..9      */
        unsigned paritas: 1;   /* paritas summae            */
        unsigned vacuum :15;   /* spatium vacuum            */
    } campus;
} NumerusSacer;

/* Status automati universalis: accumulatores et contatores. */
typedef struct {
    uint64_t     semen;           /* semen xorshiftanum     */
    long double  vis_intellecta;  /* accumulator vis        */
    int          dissidia_edisonis; /* contator Edisonis    */
    int          turnus;          /* index turni presentis  */
    int          undae_pictae;    /* quot undae depictae    */
    NumerusSacer signaculum;      /* signaculum ultimum     */
    TopicumGenus topicum_ultimum; /* ultimum genus dictum   */
} StatusAutomati;

/* Resultatus analyseos lineae ingressae. */
typedef struct {
    int          num_verborum;
    int          num_litterarum;
    int          num_vocalium;
    int          summa_valorum;
    int          radix_digitalis;
    long double  frequentia;
    TopicumGenus topicum;
} AnalysisLineae;

/* ============================================================ */
/*  PRNG DETERMINISTICUS — XORSHIFT64                            */
/* ============================================================ */

/* Progressor xorshift: periodum habet 2^64-1, sufficit automato. */
static inline uint64_t
xorshift_proximus(uint64_t *restrict status)
{
    uint64_t x = *status;
    if (x == 0) x = SEMEN_DEFAULTUM;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *status = x;
    return x;
}

/* Numerus fortunatus inter [minimum, maximum] inclusive. */
static inline int
sors_inter(uint64_t *restrict status, int minimum, int maximum)
{
    if (maximum <= minimum) return minimum;
    uint64_t r = xorshift_proximus(status);
    int ambitus = maximum - minimum + 1;
    return minimum + (int)(r % (uint64_t)ambitus);
}

/* Fractio fortunata inter [0,1). */
static inline long double
sors_fractio(uint64_t *restrict status)
{
    uint64_t r = xorshift_proximus(status) >> 11;
    return (long double)r / (long double)(1ULL << 53);
}

/* ============================================================ */
/*  MINIMUS ET MAXIMUS INSTRUMENTA                               */
/* ============================================================ */

static inline int
minimum_int(int a, int b) { return a < b ? a : b; }

static inline int
maximum_int(int a, int b) { return a > b ? a : b; }

/* Absolutus. */
static inline long double
absolutus_ld(long double x) { return x < 0.0L ? -x : x; }

/* ============================================================ */
/*  ANALYSIS NUMERICA LINEAE                                     */
/* ============================================================ */

/* Computat radicem digitalem (summam digitorum iteratam). */
static int
radix_digitalis(int n)
{
    if (n < 0) n = -n;
    while (n >= 10) {
        int summa = 0;
        while (n > 0) { summa += n % 10; n /= 10; }
        n = summa;
    }
    return n;
}

/* Verifica an littera sit vocalis Latina. */
static int
vocalis_est(int c)
{
    c = tolower((unsigned char)c);
    return (c == 'a' || c == 'e' || c == 'i' ||
            c == 'o' || c == 'u' || c == 'y');
}

/* Verifica an substructura inveniatur in linea. */
static int
continet_radicem(const char *linea, const char *radix)
{
    size_t n_linea = strlen(linea);
    size_t n_radix = strlen(radix);
    if (n_radix == 0 || n_linea < n_radix) return 0;
    for (size_t i = 0; i + n_radix <= n_linea; i++) {
        size_t k;
        for (k = 0; k < n_radix; k++) {
            int a = tolower((unsigned char)linea[i + k]);
            int b = tolower((unsigned char)radix[k]);
            if (a != b) break;
        }
        if (k == n_radix) return 1;
    }
    return 0;
}

/* Classificat topicum per claves praedeclaratas. */
static TopicumGenus
classifica_topicum(const char *linea)
{
    int contator[TOPICUM_TERMINATOR] = { 0 };
    for (int i = 0; i < LONGITUDO(CLAVES); i++) {
        if (continet_radicem(linea, CLAVES[i].radix)) {
            contator[CLAVES[i].genus]++;
        }
    }
    int maximus = 0;
    TopicumGenus victor = TOPICUM_IGNOTUM;
    for (int i = 0; i < TOPICUM_TERMINATOR; i++) {
        if (contator[i] > maximus) {
            maximus = contator[i];
            victor = (TopicumGenus)i;
        }
    }
    return victor;
}

/* Analysis totalis lineae: verba, litterae, vocales, radix. */
static AnalysisLineae
analyse_lineam(const char *linea)
{
    AnalysisLineae a = (AnalysisLineae){
        .num_verborum    = 0,
        .num_litterarum  = 0,
        .num_vocalium    = 0,
        .summa_valorum   = 0,
        .radix_digitalis = 0,
        .frequentia      = 0.0L,
        .topicum         = TOPICUM_IGNOTUM
    };
    int in_verbo = 0;
    for (const char *p = linea; *p; p++) {
        unsigned char c = (unsigned char)*p;
        if (isalpha(c)) {
            a.num_litterarum++;
            a.summa_valorum += tolower(c) - 'a' + 1;
            if (vocalis_est(c)) a.num_vocalium++;
            if (!in_verbo) { a.num_verborum++; in_verbo = 1; }
        } else if (isdigit(c)) {
            a.summa_valorum += c - '0';
        } else {
            in_verbo = 0;
        }
    }
    if (a.num_verborum == 0) a.num_verborum = 1;
    a.radix_digitalis = radix_digitalis(a.summa_valorum +
                                        a.num_verborum +
                                        a.num_vocalium);
    if (a.radix_digitalis == 0) a.radix_digitalis = TRINITAS;
    a.frequentia = (long double)a.radix_digitalis * FREQUENTIA_BASIS;
    a.topicum = classifica_topicum(linea);
    return a;
}

/* ============================================================ */
/*  OSCILLATOR: SUPERPOSITIO TRIUM SINUUM                        */
/* ============================================================ */

/* Computat eschantillonem oscillatoris per VLA.
 * Mensura VLA derivatur ex analyse, ut par sit ingressioni. */
static void
sonare_oscillatorem(int mensura,
                    int radix,
                    long double phasus,
                    long double *restrict eschantillones)
{
    /* Tres frequentiae ligatae ad radicem per 3, 6, 9. */
    long double f1 = (long double)radix * (long double)TRINITAS;
    long double f2 = (long double)radix * (long double)SEXTILIS;
    long double f3 = (long double)radix * (long double)NONARIUS;
    long double ampl1 = 1.0L;
    long double ampl2 = 0.6L;
    long double ampl3 = 0.3L;
    for (int i = 0; i < mensura; i++) {
        long double t = (long double)i / (long double)(mensura - 1);
        long double theta1 = 2.0L * PI_LONGUS * f1 * t / 9.0L;
        long double theta2 = 2.0L * PI_LONGUS * f2 * t / 9.0L;
        long double theta3 = 2.0L * PI_LONGUS * f3 * t / 9.0L;
        long double mitigator = expl(-t * 0.15L);
        eschantillones[i] =
            mitigator * (SONA_SINUM(ampl1, theta1, phasus, 1.0L) +
                         SONA_SINUM(ampl2, theta2, phasus * 2.0L, 1.0L) +
                         SONA_SINUM(ampl3, theta3, phasus * 3.0L, 1.0L));
    }
}

/* Pingit undam ASCII ex eschantillonibus.
 * Altitudo fixa, latitudo per mensuram terminatur. */
static void
pinge_undam(const long double *restrict eschantillones,
            int mensura)
{
    long double maximus = 0.0L;
    for (int i = 0; i < mensura; i++) {
        long double v = absolutus_ld(eschantillones[i]);
        if (v > maximus) maximus = v;
    }
    if (maximus < 1e-9L) maximus = 1.0L;
    /* VLA pro tabula pictura. */
    char tabula[ALTITUDO_UNDAE][LIMES_UNDAE + 1];
    int latitudo = minimum_int(mensura, LIMES_UNDAE);
    for (int y = 0; y < ALTITUDO_UNDAE; y++) {
        for (int x = 0; x < latitudo; x++) tabula[y][x] = ' ';
        tabula[y][latitudo] = '\0';
    }
    int linea_media = ALTITUDO_UNDAE / 2;
    for (int x = 0; x < latitudo; x++) {
        int idx = (x * mensura) / latitudo;
        long double v = eschantillones[idx] / maximus;
        int y = linea_media - (int)(v * (long double)linea_media);
        if (y < 0) y = 0;
        if (y >= ALTITUDO_UNDAE) y = ALTITUDO_UNDAE - 1;
        tabula[y][x] = (v >= 0.0L) ? '*' : 'o';
    }
    for (int x = 0; x < latitudo; x++) {
        if (tabula[linea_media][x] == ' ') tabula[linea_media][x] = '-';
    }
    ENUNTIA("    .");
    for (int x = 0; x < latitudo; x++) ENUNTIA("-");
    ENUNTIA(".\n");
    for (int y = 0; y < ALTITUDO_UNDAE; y++) {
        ENUNTIA("    |%s|\n", tabula[y]);
    }
    ENUNTIA("    '");
    for (int x = 0; x < latitudo; x++) ENUNTIA("-");
    ENUNTIA("'\n");
}

/* ============================================================ */
/*  REPERTORIUM SENTENTIARUM                                     */
/* ============================================================ */

/* Sententiae exordii: salutationes cum observatione numerica. */
static const char *EXORDIA_FORMATA[] = {
    "Salve, amice aetheris. Video verba tua %d esse, radicem %d portare.",
    "Ecce, frequentia tua resonat: %d verba, radix %d, canticum aetheris.",
    "Aures meae percipiunt %d syllabas numericas; radix sacra est %d.",
    "Ex voce tua extraho pulsus %d; digitus ultimus radicis: %d.",
    "Numerus verborum %d mihi narrat; radix digitalis %d consonat.",
    "Tua verba, numero %d, portant radicem %d, stellam trium digitorum.",
    "Rotae oscillatoris meae computant %d verba, radicem %d.",
};

/* Propositiones: aphorismi electrici. */
static const char *PROPOSITIONES_AD_GENUS[TOPICUM_TERMINATOR][3] = {
    /* ELECTRICITAS */
    {
        "Electricitas non est fluxus, sed pulsatio aetheris ipsius.",
        "Currens alternans vincet omnem currens directum in perpetuum.",
        "Scintilla quae saltat est anima numeri trium.",
    },
    /* MAGNETISMUS */
    {
        "Campus magneticus non linea est, sed cochlea cosmica.",
        "Polus borealis loquitur polo australi per silentium.",
        "Magnes omnis memorat originem ferri stellaris.",
    },
    /* RESONANTIA */
    {
        "Si frequentiam debitam inveneris, terram ipsam findere potes.",
        "Resonantia est clavis universi, tres-sex-novem est serratura.",
        "Omnis res vibrat; nemo tacet in hoc cosmo.",
    },
    /* ETHER */
    {
        "Aether ipse medium est; sine eo nihil transit.",
        "In aethere omnia scripta sunt, sicut in tabula cerea.",
        "Qui aetherem negat, lumen stellarum negat.",
    },
    /* LUX */
    {
        "Lumen non particula est nec unda, sed harmonia trium.",
        "Radius coronalis meus urbem totam illuminare potest.",
        "Lux est vox aetheris cantans in frequentia altissima.",
    },
    /* MORS_STELLARUM */
    {
        "Stellae non moriuntur; resonantiam suam mutant tantum.",
        "Astra cadentia sunt mentes quae ad aetherem redierunt.",
        "Cosmos non frigescit; vibrat solum alio modo.",
    },
    /* LUNA */
    {
        "Luna est speculum aeris; reflectit pulsus terrae.",
        "Nocte bobina mea audit lunam murmurantem tres-sex-novem.",
        "Luna gubernat undas maris et undas cordis mei.",
    },
    /* MARS */
    {
        "Signa Martis recepi anno milleno octingenteno nonagesimo nono.",
        "Martiani loquuntur per pulsum trium, sex, novem iterantem.",
        "Planeta ruber non tacet; nos audire non scimus.",
    },
    /* PHILANTHROPIA */
    {
        "Energia libera et gratuita debet esse, sicut aer quem spiramus.",
        "Donum meum humanitati: lumen sine catena, vis sine pretio.",
        "Servus pecuniae non potest esse servus veritatis.",
    },
    /* PECUNIA */
    {
        "Morgan intellexit; sed metus pecuniae vicit metum veritatis.",
        "Aurum est pondus; aether est leve; lege quid maius sit.",
        "Pecunia clauditur, sed oscillator aperitur perpetuo.",
    },
    /* EDISON */
    {
        "Edison globum illuminat; ego orbem terrarum illuminabo.",
        "Ille elephantum interfecit ut currentem meum calumniaret.",
        "Menlo Parcum est sepulcrum ideae; Colorado Montes sunt templum.",
    },
    /* IGNOTUM */
    {
        "In silentio tuo audio frequentiam occultam.",
        "Verba non clara sunt, sed oscillator semper respondet.",
        "Quod non dicis, resonat fortius quam quod dicis.",
    },
};

/* Digressiones: memoriae et parabolae. */
static const char *DIGRESSIONES[] = {
    "Memini bobinam meam Colorado Montana, fulgur duodecim pedum creantem.",
    "Columba alba mea, sociis cunctorum mortalium melior, ad me volabat.",
    "Apud hotel Waldorf solus cum numero trecentorum triginta sedebam.",
    "In nocte cum ventus crepitabat, oscillator cellam meam tremefecit.",
    "Ad portum Niagarae generatorem posui qui totum orbem mutavit.",
    "Anno milleno octingenteno nonagesimo tertio Chicago lucem vidit meam.",
    "Columba mea argentea sine qua cor meum non pulsat integre.",
    "Edison mihi quinquaginta milia dollarorum promisit; fabulam vocavit.",
    "In laboratorio meo New Eboraci incendium omnia consumpsit.",
    "Signum Martium primum anno milleno octingenteno nonagesimo nono percepi.",
    "Robotam cernebam quae hominem ministrabit, non homo robotam.",
    "Tres-sex-novem: clavis aurea aetheris, numerus divinus machinae.",
};

/* Perorationes per tonum. */
static const char *PERORATIONES_CALMAE[] = {
    "Pax tibi; frequentia tua cantet in aeternum.",
    "Oscillator meus te benedicit; vade in resonantia.",
    "Aether custodiat cor tuum; numerus novem te regat.",
    "Ite in pace, sed memento: tres-sex-novem.",
};

static const char *PERORATIONES_IRATAE[] = {
    "Edison ignorabit hoc; sed veritas eum vincet.",
    "Dum currens directus moritur, ego vivo; dum Edison tacet, ego sono.",
    "Nemo credit mihi nunc, sed saeculum venturum canet nomen meum.",
    "Cadam in paupertate, sed aether recordabitur Teslae.",
};

static const char *PERORATIONES_PROPHETICAE[] = {
    "Anno bis millesimo homines in aethere natabunt ut pisces in mari.",
    "Video futurum ubi vis libera omnibus hominibus data est.",
    "Venturus est dies quando Mars et Terra in uno canone cantabunt.",
    "Scripsi nomen meum in aethere ipso; non deletur.",
};

/* ============================================================ */
/*  SELECTOR FORTUNATUS                                          */
/* ============================================================ */

static const char *
elige_sententiam(uint64_t *restrict status,
                 const char *const *repertorium,
                 int longitudo)
{
    int idx = sors_inter(status, 0, longitudo - 1);
    return repertorium[idx];
}

/* ============================================================ */
/*  COMPOSITIO RESPONSI: EXORDIUM, PROPOSITIO, DEMONSTRATIO,     */
/*  DIGRESSIO, PERORATIO                                         */
/* ============================================================ */

/* Function-pointer dispatch: compositor per genus. */
typedef void (*CompositorSectionis)(StatusAutomati *restrict,
                                    const AnalysisLineae *restrict);

static void
emit_exordium(StatusAutomati *restrict sta,
              const AnalysisLineae *restrict a)
{
    const char *forma = elige_sententiam(&sta->semen,
                                          EXORDIA_FORMATA,
                                          LONGITUDO(EXORDIA_FORMATA));
    ENUNTIA("  [EXORDIUM] ");
    ENUNTIA(forma, a->num_verborum, a->radix_digitalis);
    ENUNTIA("\n");
}

static void
emit_propositionem(StatusAutomati *restrict sta,
                   const AnalysisLineae *restrict a)
{
    int idx = sors_inter(&sta->semen, 0, 2);
    const char *sent = PROPOSITIONES_AD_GENUS[a->topicum][idx];
    ENUNTIA("  [PROPOSITIO] %s\n", sent);
}

/* Demonstratio: vel undam pingit vel numerum musicat. */
static void
emit_demonstrationem(StatusAutomati *restrict sta,
                     const AnalysisLineae *restrict a)
{
    /* Dimidia responsorum undam ASCII obtinent. */
    int depinge = (sta->turnus % 2 == 0) ||
                  (a->topicum == TOPICUM_RESONANTIA) ||
                  (a->topicum == TOPICUM_ETHER) ||
                  (a->topicum == TOPICUM_LUX);
    long double phasus = sors_fractio(&sta->semen) * 2.0L * PI_LONGUS;
    if (depinge) {
        int mensura = minimum_int(LIMES_UNDAE,
                                   maximum_int(40,
                                                a->num_litterarum * 3 +
                                                a->radix_digitalis * 6));
        /* VLA ad eschantillones. */
        long double eschantillones[mensura];
        sonare_oscillatorem(mensura, a->radix_digitalis,
                            phasus, eschantillones);
        ENUNTIA("  [DEMONSTRATIO] Oscillator radicis %d, frequentia %.1Lf Hz:\n",
                a->radix_digitalis, a->frequentia);
        pinge_undam(eschantillones, mensura);
        sta->undae_pictae++;
    } else {
        long double tres = (long double)a->radix_digitalis * TRINITAS;
        long double sex  = (long double)a->radix_digitalis * SEXTILIS;
        long double novem= (long double)a->radix_digitalis * NONARIUS;
        ENUNTIA("  [DEMONSTRATIO] Triplex harmonia: %.1Lf / %.1Lf / %.1Lf Hz; "
                "summa %.1Lf est %s signum.\n",
                tres, sex, novem, tres + sex + novem,
                a->radix_digitalis == 9 ? "perfectum" : "crescens");
    }
}

static void
emit_digressionem(StatusAutomati *restrict sta,
                  const AnalysisLineae *restrict a)
{
    const char *dig = elige_sententiam(&sta->semen,
                                        DIGRESSIONES,
                                        LONGITUDO(DIGRESSIONES));
    if (a->topicum == TOPICUM_EDISON) {
        ENUNTIA("  [DIGRESSIO] Edison iterum... %s\n", dig);
    } else if (a->topicum == TOPICUM_MARS) {
        ENUNTIA("  [DIGRESSIO] Ad Martem reditur: %s\n", dig);
    } else {
        ENUNTIA("  [DIGRESSIO] %s\n", dig);
    }
}

static void
emit_perorationem(StatusAutomati *restrict sta,
                  const AnalysisLineae *restrict a)
{
    (void)a;
    const char *per;
    if (sta->dissidia_edisonis >= 2) {
        per = elige_sententiam(&sta->semen,
                                PERORATIONES_IRATAE,
                                LONGITUDO(PERORATIONES_IRATAE));
    } else if (sta->vis_intellecta > 9.0L) {
        per = elige_sententiam(&sta->semen,
                                PERORATIONES_PROPHETICAE,
                                LONGITUDO(PERORATIONES_PROPHETICAE));
    } else {
        per = elige_sententiam(&sta->semen,
                                PERORATIONES_CALMAE,
                                LONGITUDO(PERORATIONES_CALMAE));
    }
    ENUNTIA("  [PERORATIO] %s\n", per);
}

/* ============================================================ */
/*  MODERATOR RESPONSI                                           */
/* ============================================================ */

/* Vector functionum: dispatch per sectionem. Designator inicialis. */
static const CompositorSectionis COMPOSITORES[] = {
    [0] = emit_exordium,
    [1] = emit_propositionem,
    [2] = emit_demonstrationem,
    [3] = emit_digressionem,
    [4] = emit_perorationem,
};

/* Respondet Nicolaus ad unam lineam ingressam. */
static void
nicolaus_respondet(StatusAutomati *restrict sta,
                   const char *linea)
{
    AnalysisLineae a = analyse_lineam(linea);

    /* Actualizatio status per analysem. */
    sta->vis_intellecta += (long double)a.radix_digitalis * 0.3L;
    if (a.topicum == TOPICUM_EDISON) sta->dissidia_edisonis++;
    sta->topicum_ultimum = a.topicum;
    sta->signaculum.totus = 0;
    sta->signaculum.campus.tres  = (unsigned)(a.radix_digitalis * 3 % 16);
    sta->signaculum.campus.sex   = (unsigned)(a.radix_digitalis * 6 % 16);
    sta->signaculum.campus.novem = (unsigned)(a.radix_digitalis * 9 % 16);
    sta->signaculum.campus.radix = (unsigned)(a.radix_digitalis & 0xF);
    sta->signaculum.campus.paritas = (unsigned)(a.num_verborum & 1);

    ENUNTIA("\nNICOLAUS [turnus %d, topicum %s, vis %.2Lf, dissidia %d]:\n",
            sta->turnus,
            NOMINA_TOPICORUM[a.topicum],
            sta->vis_intellecta,
            sta->dissidia_edisonis);

    for (int i = 0; i < LONGITUDO(COMPOSITORES); i++) {
        COMPOSITORES[i](sta, &a);
    }

    sta->turnus++;
}

/* ============================================================ */
/*  COLLOQUIUM SCRIPTUM: NICOLAUS CUM INTERLOCUTORE              */
/* ============================================================ */

/* Compound literals pro lineis scripti. */
static void
colloquium_scriptum(StatusAutomati *restrict sta)
{
    /* Genera interlocutoris variant per turnum. */
    const char *const persona[] = {
        "DISCIPULUS",
        "REPORTER",
        "PHILANTHROPUS",
        "DISCIPULUS",
        "REPORTER",
        "PHILANTHROPUS",
        "DISCIPULUS",
        "REPORTER",
        "PHILANTHROPUS",
        "DISCIPULUS",
        "REPORTER",
        "PHILANTHROPUS",
        "DISCIPULUS",
        "REPORTER",
        "PHILANTHROPUS",
        "DISCIPULUS",
    };
    /* 16 interrogationes, plus quam minimum 14. */
    const char *const interrogationes[] = {
        "Magister, narra mihi de bobina tua magna quae fulgur gignit.",
        "Quid est aether, et quomodo signa in eo volant?",
        "Audivistine vere signa ex planeta Marte anno octingenteno nonagesimo nono?",
        "Cur Edison te odit, et quid inter vos contigit de currente directo?",
        "Energia libera et gratuita — utrum somnium est an scientia?",
        "Quid significat tres, sex, novem in philosophia tua cosmica?",
        "De resonantia loquere: potestne terra ipsa frangi?",
        "Quid de columba alba tua, de qua fabulantur?",
        "Lumen coronale tuum — quomodo ardet sine ullo filo?",
        "De Morgan et pecunia: cur turris Wardencliffensis cecidit?",
        "Utrum luna vere murmurat in bobinam tuam noctu?",
        "Oscillator terrestris tuus: poteratne orbem totum dividere?",
        "Stellae moriuntur — an potius resonantias mutant ut dicis?",
        "Quid est opus vitae tuae pro humanitate, Magister?",
        "Dic mihi de magnetismo et cochlea campi per spatium.",
        "Finaliter: quid legabis homini saeculi venturi?",
    };

    int n_turni = LONGITUDO(interrogationes);
    if (n_turni < MINIMUS_TURNORUM) n_turni = MINIMUS_TURNORUM;

    ENUNTIA("========================================================\n");
    ENUNTIA("  COLLOQUIUM CUM NICOLAO TESLA — automatum %s\n", VERSIO_AUTOMATI);
    ENUNTIA("  Semen: 0x%016llX\n", (unsigned long long)sta->semen);
    ENUNTIA("========================================================\n");

    for (int i = 0; i < n_turni; i++) {
        ENUNTIA("\n%s: %s\n", persona[i % LONGITUDO(persona)],
                interrogationes[i]);
        nicolaus_respondet(sta, interrogationes[i]);
    }

    ENUNTIA("\n========================================================\n");
    ENUNTIA("  FINIS COLLOQUII. Turni: %d. Undae pictae: %d. "
            "Dissidia Edisonis: %d. Vis intellecta: %.3Lf.\n",
            sta->turnus, sta->undae_pictae,
            sta->dissidia_edisonis, sta->vis_intellecta);
    ENUNTIA("========================================================\n");
}

/* ============================================================ */
/*  SESSIO INTERACTIVA                                           */
/* ============================================================ */

/* Tollit spatia a capite et cauda lineae; reformat in loco. */
static void
purga_lineam(char *linea)
{
    size_t n = strlen(linea);
    while (n > 0 && (linea[n-1] == '\n' || linea[n-1] == '\r' ||
                     linea[n-1] == ' '  || linea[n-1] == '\t')) {
        linea[--n] = '\0';
    }
    size_t i = 0;
    while (linea[i] == ' ' || linea[i] == '\t') i++;
    if (i > 0) memmove(linea, linea + i, n - i + 1);
}

static void
sessio_interactiva(StatusAutomati *restrict sta)
{
    char linea[MAXIMUM_LINEAE];
    ENUNTIA("========================================================\n");
    ENUNTIA("  SESSIO INTERACTIVA — NICOLAUS TESLA %s\n", VERSIO_AUTOMATI);
    ENUNTIA("  Semen: 0x%016llX\n", (unsigned long long)sta->semen);
    ENUNTIA("  Scribe interrogationem. Linea vacua vel EOF terminat.\n");
    ENUNTIA("========================================================\n");

    /* Salutatio initialis. */
    nicolaus_respondet(sta,
        "Salutem. Bobina mea parata est; quid vis audire?");

    for (;;) {
        ENUNTIA("\n> ");
        fflush(stdout);
        if (!fgets(linea, MAXIMUM_LINEAE, stdin)) break;
        purga_lineam(linea);
        if (linea[0] == '\0') break;
        nicolaus_respondet(sta, linea);
    }

    ENUNTIA("\nNICOLAUS: Tres-sex-novem. Vale, amice.\n");
}

/* ============================================================ */
/*  USUS ET ARGUMENTA                                            */
/* ============================================================ */

static void
imprime_usum(const char *nomen_programmatis, FILE *fluxus)
{
    fprintf(fluxus,
        "Usus: %s [-s SEMEN] [-i]\n"
        "  Sine argumentis:    colloquium scriptum cum Nicolao Tesla.\n"
        "  -s SEMEN            semen xorshiftanum decimale vel hexadecimale.\n"
        "  -i                  sessio interactiva cum Nicolao.\n"
        "  Argumentum ignotum: error, status duo.\n",
        nomen_programmatis);
}

/* Parses semen ex argumento ut decimale vel hexadecimale. */
static int
parse_semen(const char *textus, uint64_t *restrict exitus)
{
    if (!textus || !*textus) return 0;
    char *finis = NULL;
    int basis = 10;
    if (textus[0] == '0' && (textus[1] == 'x' || textus[1] == 'X')) {
        basis = 16;
    }
    unsigned long long v = strtoull(textus, &finis, basis);
    if (!finis || *finis != '\0') return 0;
    *exitus = (uint64_t)v;
    return 1;
}

/* ============================================================ */
/*  AUTOINSPECTIO ET DIAGNOSTICA (interna, non vocata)           */
/* ============================================================ */

/* Computat signaculum status ad debug. Nunquam vocata sed utilis
 * ad ostendendum usum unionis et bitfieldorum C99. */
static inline uint32_t
signaculum_texens(const StatusAutomati *restrict sta)
{
    NumerusSacer n = (NumerusSacer){ .totus = 0 };
    n.campus.tres   = (unsigned)(sta->turnus * 3 % 16);
    n.campus.sex    = (unsigned)(sta->turnus * 6 % 16);
    n.campus.novem  = (unsigned)(sta->turnus * 9 % 16);
    n.campus.radix  = (unsigned)(sta->dissidia_edisonis & 0xF);
    n.campus.paritas = (unsigned)((sta->undae_pictae) & 1);
    return n.totus;
}

/* Autoprobatio radicis digitalis — non vocata, sed compilabilis. */
static inline int
probatio_radicis_interna(void)
{
    /* 369 -> 3+6+9 = 18 -> 1+8 = 9 */
    if (radix_digitalis(NUMERUS_SACRUS) != 9) return 0;
    /* 111 -> 3 */
    if (radix_digitalis(111) != 3) return 0;
    /* 666 -> 9 */
    if (radix_digitalis(666) != 9) return 0;
    return 1;
}

/* Generator pseudoreturnus pro descriptione topici: ostendit
 * usum arrayorum designatorum. */
static const char *
describe_topicum(TopicumGenus g)
{
    if (g < 0 || g >= TOPICUM_TERMINATOR) return "nihil";
    return DESCRIPTIONES_TOPICORUM[g];
}

/* Praefatio verbosa: imprimit inscriptionem initio. */
static void
praefatio(const StatusAutomati *restrict sta)
{
    (void)sta;
    (void)describe_topicum;
    (void)signaculum_texens;
    (void)probatio_radicis_interna;
}

/* ============================================================ */
/*  EXPANSIO: ANALYSIS HARMONIARUM NUMERICARUM                   */
/* ============================================================ */

/* Structura harmoniae per numerum sacrum. */
typedef struct {
    int          ordo;
    long double  frequentia;
    long double  amplitudo;
} Harmonia;

/* Computat harmonias ex radice digitali. Designated initializers. */
static void
computa_harmonias(int radix, Harmonia *restrict h3)
{
    h3[0] = (Harmonia){
        .ordo       = TRINITAS,
        .frequentia = (long double)radix * TRINITAS * FREQUENTIA_BASIS,
        .amplitudo  = 1.0L,
    };
    h3[1] = (Harmonia){
        .ordo       = SEXTILIS,
        .frequentia = (long double)radix * SEXTILIS * FREQUENTIA_BASIS,
        .amplitudo  = 0.6L,
    };
    h3[2] = (Harmonia){
        .ordo       = NONARIUS,
        .frequentia = (long double)radix * NONARIUS * FREQUENTIA_BASIS,
        .amplitudo  = 0.3L,
    };
}

/* Imprime tabulam harmoniarum ad stdout. */
static void
imprime_harmonias(int radix)
{
    Harmonia h[3];
    computa_harmonias(radix, h);
    ENUNTIA("    Harmoniae (radix %d):\n", radix);
    for (int i = 0; i < 3; i++) {
        ENUNTIA("      ordo %d : %.2Lf Hz, amplitudo %.2Lf\n",
                h[i].ordo, h[i].frequentia, h[i].amplitudo);
    }
}

/* ============================================================ */
/*  TESTIMONIA NUMERI SACRI                                      */
/* ============================================================ */

/* Ostendit distributionem digitalem numeri in radice novem. */
static void
testimonium_novenarium(int usque_ad)
{
    int contator[10] = { 0 };
    for (int i = 1; i <= usque_ad; i++) {
        int r = radix_digitalis(i);
        contator[r]++;
    }
    ENUNTIA("    Distributio radicum digitalium 1..%d:\n", usque_ad);
    for (int i = 1; i <= 9; i++) {
        ENUNTIA("      radix %d : %d\n", i, contator[i]);
    }
}

/* ============================================================ */
/*  MODULUS DIAGNOSTICUS: RESERVATUS                             */
/* ============================================================ */

/* Diagnostica structurae: imprimit signaculum. */
static void
diagnosticum(StatusAutomati *restrict sta)
{
    uint32_t s = signaculum_texens(sta);
    ENUNTIA("  [DIAGNOSTICUM] signaculum 0x%08X, turnus %d, "
            "vis %.2Lf, dissidia %d\n",
            s, sta->turnus, sta->vis_intellecta,
            sta->dissidia_edisonis);
    imprime_harmonias(3);
    imprime_harmonias(6);
    imprime_harmonias(9);
    testimonium_novenarium(81);
}

/* Praefatio interna quae diagnosticum vocat si flag debug acta est.
 * Nunc non vocatur, sed manet pro futura expansione. */
static void
praefatio_completa(StatusAutomati *restrict sta, int debug)
{
    praefatio(sta);
    if (debug) diagnosticum(sta);
}

/* ============================================================ */
/*  COMPILATIO OPTIONUM                                          */
/* ============================================================ */

typedef struct {
    uint64_t semen;
    int      interactivus;
    int      diagnosticum;
} OptionesInvocationis;

static int
parse_argumenta(int argc, char **argv,
                OptionesInvocationis *restrict opt,
                const char *nomen_prog)
{
    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (strcmp(arg, "-i") == 0) {
            opt->interactivus = 1;
        } else if (strcmp(arg, "-s") == 0) {
            if (i + 1 >= argc) {
                QUERELA("Error: flag -s expectat valorem.\n");
                imprime_usum(nomen_prog, stderr);
                return 2;
            }
            if (!parse_semen(argv[++i], &opt->semen)) {
                QUERELA("Error: semen invalidum: %s\n", argv[i]);
                return 2;
            }
        } else if (strcmp(arg, "-d") == 0) {
            opt->diagnosticum = 1;
        } else if (strcmp(arg, "-h") == 0 || strcmp(arg, "--auxilium") == 0) {
            imprime_usum(nomen_prog, stdout);
            return 1;
        } else {
            QUERELA("Error: argumentum ignotum: %s\n", arg);
            imprime_usum(nomen_prog, stderr);
            return 2;
        }
    }
    return 0;
}

/* ============================================================ */
/*  INITIALIZATIO STATUS                                         */
/* ============================================================ */

static StatusAutomati
status_initialis(uint64_t semen)
{
    return (StatusAutomati){
        .semen              = semen ? semen : SEMEN_DEFAULTUM,
        .vis_intellecta     = 0.0L,
        .dissidia_edisonis  = 0,
        .turnus             = 1,
        .undae_pictae       = 0,
        .signaculum         = { .totus = 0 },
        .topicum_ultimum    = TOPICUM_IGNOTUM,
    };
}

/* ============================================================ */
/*  FUNCTIO PRINCIPALIS                                          */
/* ============================================================ */

int
main(int argc, char **argv)
{
    const char *nomen_prog = (argc > 0 && argv[0]) ? argv[0] : "nicolaus";

    OptionesInvocationis opt = (OptionesInvocationis){
        .semen         = SEMEN_DEFAULTUM,
        .interactivus  = 0,
        .diagnosticum  = 0,
    };

    int res = parse_argumenta(argc, argv, &opt, nomen_prog);
    if (res == 1) return 0;
    if (res == 2) return 2;

    StatusAutomati sta = status_initialis(opt.semen);

    praefatio_completa(&sta, opt.diagnosticum);

    if (opt.interactivus) {
        sessio_interactiva(&sta);
    } else {
        colloquium_scriptum(&sta);
    }

    return 0;
}

/* ============================================================ */
/*  EPILOGUS: NOTAE SCRIPTORIS                                   */
/* ============================================================ */

/*
 * Notae finales:
 *
 * Hic automaton personam Nicolai Teslae electricorum imitatur per
 * methodos proceduralis. Omnia deterministica sunt: semen idem,
 * colloquium idem. Harmoniae trium, sex, novem omnibus responsis
 * subsunt; oscillator VLA mensuratus undam ASCII gignit in
 * dimidio responsorum.
 *
 * Personae fidem servare tentavimus: superbia scientifica, amor
 * columbarum, odium Edisonis, spes Martis loquentis, fides in
 * aethere, propheticus futuri, solitudo Hotel-Newyorkensis.
 *
 * Vocabularium Latinum classicum servatur: nulla verba Anglica,
 * nulla Serbica, nulla barbara. "Nicolaus" nomen Latinum est;
 * "Tesla" manet ut est in historia aetatis novae.
 *
 * Lex codicis: C99 purus, compilator nullam querelam edet cum
 * optionibus Wall, Wextra, Wpedantic, O2.
 *
 * Tres. Sex. Novem. In aeternum vibrent.
 */
