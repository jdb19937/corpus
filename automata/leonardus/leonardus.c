/*
 * leonardus.c -- Automaton Leonardi Vincii, pictoris et philosophi
 *
 * FRAGMENTA COMMENTARII: machinatio quae imitatur modum Leonardi
 * in commentariis suis scribendi -- observatio, digressio, schema
 * lineare, speculatio, et transitus ad alteram disciplinam.
 *
 * Octo sunt domus curiositatis: ANATOMIA, MECHANICA, PICTURA,
 * AQUA, VOLATUS, LUMEN, PROPORTIO, MUSICA.  Verba interlocutoris
 * domos excitant; duae maxime excitatae fiunt primaria et
 * secundaria responsionis.  Schema ASCII pingitur secundum
 * domum primariam, cum parametris ex generatore pseudo-fortuito
 * deterministico (xorshift64).
 *
 * Signum Leonardi proprium: SCRIPTURA SPECULARIS.  Flag -v
 * invertit ordinem litterarum in unaquaque linea exitus.
 *
 * Sine argumentis: colloquium scriptum inter Leonardum et
 * discipulum, quattuordecim vicibus, ad stdout.
 *   -i : modus interactivus cum usuario
 *   -s N : semen explicitum pro generatore
 *   -v : modus speculi (litterae inversae)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdarg.h>

/* ============================================================
 * MACRA PRIMARIA (omnia maiusculis more latino)
 * ============================================================ */

#define VERSIO_MAIOR        1
#define VERSIO_MINOR        0
#define LATITUDO_LINEAE     78
#define LATITUDO_SCHEMATIS  48
#define ALTITUDO_SCHEMATIS  14
#define MAX_VERBI           64
#define MAX_LINEAE          512
#define MAX_INGRESSUS       1024
#define NUMERUS_DOMORUM     8
#define MINIMAE_VICES       14
#define SEMEN_DEFALTUM      0x1e0dacafebeef01dULL

/* Macro variadicum pro scripto mirando -- scribit ad stream
 * vel normalem vel speculariter secundum flag globalem. */
#define SCRIBE(...)         leo_scribe(stdout, __VA_ARGS__)
#define LINEA_VACUA()       leo_scribe(stdout, "%s", "")

/* X-macro pro enumeratione domorum.  Ordo stabilis: numerat
 * tam indices quam nomina quam verba-claves. */
#define DOMUS_TABELLA \
    X(ANATOMIA,  "ANATOMIA",  "corpus musculus cor sanguis oculus nervus os manus pes caput") \
    X(MECHANICA, "MECHANICA", "machina rota cochlea pondus trochlea vectis dens pinna axis") \
    X(PICTURA,   "PICTURA",   "pictura color umbra lumen pigmentum tabula imago vultus") \
    X(AQUA,      "AQUA",      "aqua fluvius unda vortex fons imber mare fluxus") \
    X(VOLATUS,   "VOLATUS",   "volatus avis ala penna aer ventus caelum vespertilio") \
    X(LUMEN,     "LUMEN",     "lumen sol radius umbra clarus obscurus stella ignis") \
    X(PROPORTIO, "PROPORTIO", "proportio numerus mensura circulus quadratum aureum geometria") \
    X(MUSICA,    "MUSICA",    "musica sonus chorda lyra harmonia cantus vox tonus")

/* ============================================================
 * ENUMERATIO DOMORUM (valores explicite positi -- bitfield mask)
 * ============================================================ */

enum Domus {
#define X(sym, nom, verba)  DOM_##sym,
    DOMUS_TABELLA
#undef X
    DOM_FINIS
};

enum MascaDomus {
#define X(sym, nom, verba)  MSK_##sym = 1u << DOM_##sym,
    DOMUS_TABELLA
#undef X
    MSK_OMNES = (1u << NUMERUS_DOMORUM) - 1u
};

/* ============================================================
 * STRUCTURAE ET UNIONES
 * ============================================================ */

/* Bitfield: octo domus mascae plus vestigia status */
typedef struct {
    unsigned int interesse : 8;   /* mascam domorum excitatarum */
    unsigned int speculum  : 1;   /* scriptura specularis? */
    unsigned int interact  : 1;   /* modus interactivus? */
    unsigned int verbosus  : 1;   /* plura scribere? */
    unsigned int finitum   : 1;   /* colloquium finitum? */
    unsigned int reserv    : 4;
} StatusAutomati;

/* Curiositas unius domus: quantum excitata, ultimus tactus */
typedef struct {
    enum Domus   genus;
    int          curiositas;
    int          ultimus_tactus;
    const char  *nomen;
    const char  *verba;
} Curiositas;

/* Canvas parametrum pro schematibus -- union pro variis tipis */
typedef union {
    int          integer;
    double       ratio;
    const char  *vocabulum;
} Parametrum;

/* Schema procedurale -- pointer ad functionem pictoriam */
typedef void (*FunctioSchematis)(char *buf, int lat, int alt,
                                 uint64_t *rng);

typedef struct {
    enum Domus        genus;
    const char       *titulus;
    FunctioSchematis  pictor;
} Schematarius;

/* Sententia aphoristica per domum */
typedef struct {
    enum Domus   genus;
    const char  *verbum;
} Aphorismus;

/* Ingressus interlocutoris in colloquio scripto */
typedef struct {
    const char  *vox;
    int          turn;
} Ingressus;

/* Flexibile: lista aphorismorum cum membro flexibili */
typedef struct {
    int          numerus;
    Aphorismus   elementa[];
} SertumAphorismorum;

/* ============================================================
 * STATUS GLOBALIS
 * ============================================================ */

static StatusAutomati g_status = {
    .interesse = 0,
    .speculum  = 0,
    .interact  = 0,
    .verbosus  = 0,
    .finitum   = 0,
    .reserv    = 0
};

static uint64_t g_semen = SEMEN_DEFALTUM;

static Curiositas g_domus[NUMERUS_DOMORUM] = {
#define X(sym, nom, verba)  { DOM_##sym, 0, -1, nom, verba },
    DOMUS_TABELLA
#undef X
};

/* ============================================================
 * GENERATOR PSEUDO-FORTUITUS (xorshift64, deterministicus)
 * ============================================================ */

static inline uint64_t
xorshift_roga(uint64_t * restrict status)
{
    uint64_t x = *status;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *status = x ? x : 0x9e3779b97f4a7c15ULL;
    return *status;
}

static inline int
xorshift_inter(uint64_t *status, int min, int max)
{
    if (max <= min) return min;
    uint64_t r = xorshift_roga(status);
    return min + (int)(r % (uint64_t)(max - min + 1));
}

static inline double
xorshift_fractio(uint64_t *status)
{
    uint64_t r = xorshift_roga(status);
    return (double)(r >> 11) / (double)(1ULL << 53);
}

/* Declarationes anticipatae pro mathesi minore */
static double leo_cos(double x);
static double leo_sin(double x);

/* ============================================================
 * FUNCTIO SCRIBENDI MIRANS (speculum-aware)
 * ============================================================ */

static void
inverte_lineam(char * restrict s)
{
    size_t n = strlen(s);
    for (size_t i = 0; i < n / 2; i++) {
        char t = s[i];
        s[i] = s[n - 1 - i];
        s[n - 1 - i] = t;
    }
}

static void
leo_scribe(FILE *stream, const char *fmt, ...)
{
    char buf[MAX_LINEAE];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (n < 0) return;
    if (g_status.speculum) {
        /* Linea per lineam invertere -- ut in speculo */
        char *p = buf;
        while (*p) {
            char *q = strchr(p, '\n');
            if (q) *q = '\0';
            char tmp[MAX_LINEAE];
            strncpy(tmp, p, sizeof(tmp) - 1);
            tmp[sizeof(tmp) - 1] = '\0';
            inverte_lineam(tmp);
            fputs(tmp, stream);
            fputc('\n', stream);
            if (!q) break;
            p = q + 1;
        }
    } else {
        fputs(buf, stream);
        fputc('\n', stream);
    }
}

/* ============================================================
 * TEXTUS LATINUS: APHORISMI ET LOCUTIONES
 * ============================================================ */

static const char *APHORISMI_ANATOMIA[] = {
    "Corpus humanum est machina mirabilis Dei et naturae.",
    "Musculus sine nervo tacet; nervus sine cerebro caecus est.",
    "In oculo lumen nascitur et in mente moritur.",
    "Manus instrumentum instrumentorum est."
};

static const char *APHORISMI_MECHANICA[] = {
    "Nulla vis sine motu, nullus motus sine resistentia.",
    "Rota intra rotam -- ecce secretum trochleae.",
    "Cochlea quae aquam levat eadem aerem premit.",
    "Pondus est mater motus, vectis pater."
};

static const char *APHORISMI_PICTURA[] = {
    "Pictura est poesis muta; poesis pictura loquens.",
    "Umbra filia corporis et luminis est.",
    "Color sine lumine nihil; lumen sine umbra caecum.",
    "Vultus humanus speculum animae."
};

static const char *APHORISMI_AQUA[] = {
    "Aqua memoriam non habet, sed formam omnem accipit.",
    "Vortex in flumine similis est turbini in aere.",
    "Unda quae pellit eadem pellitur.",
    "Fluvius docet philosophum plus quam liber."
};

static const char *APHORISMI_VOLATUS[] = {
    "Avis machina est secundum legem mathematicam operans.",
    "Homo volabit si alae magnae et venti humiles.",
    "Penna singula parva; pennae multae caelum vincunt.",
    "Aer fluvius est invisibilis super capita nostra."
};

static const char *APHORISMI_LUMEN[] = {
    "Lumen est anima picturae.",
    "Umbra non est absentia luminis, sed vestigium eius.",
    "Radius rectus sed in aqua fractus.",
    "Sol pictor maximus est, pluit colores in terram."
};

static const char *APHORISMI_PROPORTIO[] = {
    "Omnia per numerum, mensuram, et pondus disposita sunt.",
    "Homo circulo et quadrato inscribitur.",
    "Proportio aurea in herba et in stella reperitur.",
    "Geometria est lingua qua Deus mundum scripsit."
};

static const char *APHORISMI_MUSICA[] = {
    "Musica est sonus mensuratus tempore.",
    "Chorda vibrans similis est pennae in vento.",
    "Harmonia auris similis est proportioni oculi.",
    "Vox humana instrumentum perfectissimum."
};

static const char **APHORISMI_TABULA[NUMERUS_DOMORUM] = {
    [DOM_ANATOMIA]  = APHORISMI_ANATOMIA,
    [DOM_MECHANICA] = APHORISMI_MECHANICA,
    [DOM_PICTURA]   = APHORISMI_PICTURA,
    [DOM_AQUA]      = APHORISMI_AQUA,
    [DOM_VOLATUS]   = APHORISMI_VOLATUS,
    [DOM_LUMEN]     = APHORISMI_LUMEN,
    [DOM_PROPORTIO] = APHORISMI_PROPORTIO,
    [DOM_MUSICA]    = APHORISMI_MUSICA
};

static const int APHORISMI_NUMERUS[NUMERUS_DOMORUM] = {
    [DOM_ANATOMIA]  = 4,
    [DOM_MECHANICA] = 4,
    [DOM_PICTURA]   = 4,
    [DOM_AQUA]      = 4,
    [DOM_VOLATUS]   = 4,
    [DOM_LUMEN]     = 4,
    [DOM_PROPORTIO] = 4,
    [DOM_MUSICA]    = 4
};

/* Speculationes -- hypotheses secundum domum */
static const char *SPECULATIONES_ANATOMIA[] = {
    "fortasse cor non centrum sed pompa est, et sanguis fluit in circulo",
    "suspicor nervos esse filos tensos, non canales vacuos",
    "crederem oculum imaginem recipere inversam in camera obscura capitis",
    "opinor musculos fibras contractiles, non saccos inflatos"
};
static const char *SPECULATIONES_MECHANICA[] = {
    "si rotam intra rotam ponam, forsan vim multiplicabo sine strepitu",
    "meditor machinam quae homini alas det sine plumis divinis",
    "cochleam aeream fingam, quae per aerem ut per aquam penetret",
    "trochleam quadruplicem excogitem ad turres construendas"
};
static const char *SPECULATIONES_PICTURA[] = {
    "pigmentum cum oleo miscere stratim, ut umbrae fluant sfumato",
    "vultus dimidius in lumine, dimidius in umbra, magis loquitur",
    "perspectiva aeria: remota caerulescunt quia aer coloratur",
    "pictor oculos sequi debet, non lineas rigidas"
};
static const char *SPECULATIONES_AQUA[] = {
    "vortex aquae serpentina in semetipsum redit, ut cochlea viva",
    "canales si contegam, aqua celerius curret sine vento",
    "inundationes per fossas regere possum, non obstare",
    "gutta cadens sphaeram format antequam frangatur"
};
static const char *SPECULATIONES_VOLATUS[] = {
    "avis non batit sed vehitur in aere velut navis in unda",
    "alae curvae supra quam infra -- ibi secretum elevationis",
    "vespertilionis ala membranacea aptior quam plumata pro homine",
    "si descendo obliquus, aer me sustinebit ut palmam foliatam"
};
static const char *SPECULATIONES_LUMEN[] = {
    "umbra est locus unde lumen prohibitum est, non substantia nigra",
    "colores nihil aliud sunt quam modi quibus corpora lumen frangunt",
    "radius per foramen minutum imaginem pingit inversam",
    "oculus lumen non emittit, sed recipit -- Platoni contradico"
};
static const char *SPECULATIONES_PROPORTIO[] = {
    "mensurae humanae septem in altitudine capita faciunt, non octo",
    "circulus et quadratum idem centrum habent si homo extenditur",
    "proportio phi reperitur in conchyliis, foliis, et vultibus",
    "triangulum aequilaterum clavis est geometriae sacrae"
};
static const char *SPECULATIONES_MUSICA[] = {
    "chorda brevior sonum acutiorem reddit in ratione inversa",
    "tympanum auris membrana est qualis in tympano mechanico",
    "consonantia est proportio numerorum parvorum audibilis",
    "vox cantantis laryngem habet ut fistulam cum lingua"
};

static const char **SPECULATIONES_TABULA[NUMERUS_DOMORUM] = {
    [DOM_ANATOMIA]  = SPECULATIONES_ANATOMIA,
    [DOM_MECHANICA] = SPECULATIONES_MECHANICA,
    [DOM_PICTURA]   = SPECULATIONES_PICTURA,
    [DOM_AQUA]      = SPECULATIONES_AQUA,
    [DOM_VOLATUS]   = SPECULATIONES_VOLATUS,
    [DOM_LUMEN]     = SPECULATIONES_LUMEN,
    [DOM_PROPORTIO] = SPECULATIONES_PROPORTIO,
    [DOM_MUSICA]    = SPECULATIONES_MUSICA
};

/* Transitus -- pontes inter domos */
static const char *TRANSITUS_PROTOTYPA[] = {
    "et haec res me ducit ad %s, nam",
    "quod confert cum %s, ubi similis ratio",
    "sed considera etiam %s -- non diversa materia",
    "unde mens mea transit ad %s, quia",
    "meminisse iuvat %s, ubi eadem lex videtur"
};
static const int TRANSITUS_NUMERUS = 5;

/* ============================================================
 * SCHEMATA ASCII -- functiones pictoriae (sex et amplius)
 * ============================================================ */

static void
initia_telam(char *buf, int lat, int alt)
{
    /* tela est (lat+1)*alt characterum cum terminationibus */
    for (int i = 0; i < alt; i++) {
        for (int j = 0; j < lat; j++)
            buf[i * (lat + 1) + j] = ' ';
        buf[i * (lat + 1) + lat] = '\0';
    }
}

static inline void
pone_punctum(char *buf, int lat, int alt, int x, int y, char c)
{
    if (x < 0 || x >= lat || y < 0 || y >= alt) return;
    buf[y * (lat + 1) + x] = c;
}

/* I. SPIRA -- pro MECHANICA et AQUA (vortex) */
static void
pinge_spiram(char *buf, int lat, int alt, uint64_t *rng)
{
    initia_telam(buf, lat, alt);
    double cx = lat / 2.0;
    double cy = alt / 2.0;
    double scala = 0.6 + 0.4 * xorshift_fractio(rng);
    int gyri = 3 + xorshift_inter(rng, 0, 2);
    double passus = 0.18;
    for (double t = 0.0; t < gyri * 6.283; t += passus) {
        double r = scala * t / 3.14;
        int x = (int)(cx + r * 1.9 * leo_cos(t));
        int y = (int)(cy + r * 0.9 * leo_sin(t));
        pone_punctum(buf, lat, alt, x, y, '*');
    }
    pone_punctum(buf, lat, alt, (int)cx, (int)cy, 'O');
}

/* II. ROTA DENTATA -- pro MECHANICA */
static void
pinge_rotam(char *buf, int lat, int alt, uint64_t *rng)
{
    initia_telam(buf, lat, alt);
    double cx = lat / 2.0;
    double cy = alt / 2.0;
    double r = (alt < lat / 2 ? alt : lat / 2) * 0.45;
    int dentes = 8 + xorshift_inter(rng, 0, 4);
    int passus = 72;
    for (int i = 0; i < passus; i++) {
        double t = 6.283 * i / passus;
        double rad = r + 1.2 * ((i * dentes / passus) % 2);
        int x = (int)(cx + rad * 1.9 * leo_cos(t));
        int y = (int)(cy + rad * 0.9 * leo_sin(t));
        pone_punctum(buf, lat, alt, x, y, '#');
    }
    /* axis */
    for (int i = -1; i <= 1; i++)
        pone_punctum(buf, lat, alt, (int)cx + i, (int)cy, '+');
    pone_punctum(buf, lat, alt, (int)cx, (int)cy - 1, '|');
    pone_punctum(buf, lat, alt, (int)cx, (int)cy + 1, '|');
}

/* III. ALA -- pro VOLATUS */
static void
pinge_alam(char *buf, int lat, int alt, uint64_t *rng)
{
    initia_telam(buf, lat, alt);
    int curvatura = 2 + xorshift_inter(rng, 0, 2);
    int basis = alt / 2;
    /* ossa alae */
    for (int x = 2; x < lat - 2; x++) {
        double dx = (double)x - (double)lat / 2.0;
        double hl = (double)lat / 2.0;
        int y = basis - (int)(curvatura * (1.0 - (dx * dx) / (hl * hl)));
        if (y < 0) y = 0;
        pone_punctum(buf, lat, alt, x, y, '_');
    }
    /* pennae */
    int pennae = 6 + xorshift_inter(rng, 0, 3);
    for (int i = 0; i < pennae; i++) {
        int x = 4 + i * ((lat - 8) / pennae);
        int y = basis - 1;
        int len = 2 + xorshift_inter(rng, 0, 2);
        for (int j = 0; j < len; j++)
            pone_punctum(buf, lat, alt, x, y + j + 1, '\\');
    }
    /* radix */
    pone_punctum(buf, lat, alt, 1, basis, 'X');
}

/* IV. CRUX VITRUVIANA -- pro PROPORTIO */
static void
pinge_vitruvium(char *buf, int lat, int alt, uint64_t *rng)
{
    initia_telam(buf, lat, alt);
    double cx = lat / 2.0;
    double cy = alt / 2.0;
    double r = (alt < lat / 2 ? alt : lat / 2) * 0.48;
    /* circulus */
    for (double t = 0.0; t < 6.283; t += 0.08) {
        int x = (int)(cx + r * 1.9 * leo_cos(t));
        int y = (int)(cy + r * 0.9 * leo_sin(t));
        pone_punctum(buf, lat, alt, x, y, '.');
    }
    /* quadratum */
    int q = (int)(r * 1.3);
    int qy = (int)(r * 0.65);
    for (int x = (int)cx - q; x <= (int)cx + q; x++) {
        pone_punctum(buf, lat, alt, x, (int)cy - qy, '-');
        pone_punctum(buf, lat, alt, x, (int)cy + qy, '-');
    }
    for (int y = (int)cy - qy; y <= (int)cy + qy; y++) {
        pone_punctum(buf, lat, alt, (int)cx - q, y, '|');
        pone_punctum(buf, lat, alt, (int)cx + q, y, '|');
    }
    /* homo */
    pone_punctum(buf, lat, alt, (int)cx, (int)cy - qy + 1, 'o');
    for (int y = (int)cy - qy + 2; y <= (int)cy + qy - 1; y++)
        pone_punctum(buf, lat, alt, (int)cx, y, '|');
    /* bracchia */
    int extens = xorshift_inter(rng, q - 2, q);
    for (int dx = -extens; dx <= extens; dx++)
        pone_punctum(buf, lat, alt, (int)cx + dx, (int)cy - 1, '-');
    /* crura */
    for (int d = 1; d < qy - 1; d++) {
        pone_punctum(buf, lat, alt, (int)cx - d, (int)cy + qy - 1 - d / 2, '/');
        pone_punctum(buf, lat, alt, (int)cx + d, (int)cy + qy - 1 - d / 2, '\\');
    }
}

/* V. VORTEX AQUAE -- pro AQUA */
static void
pinge_vorticem(char *buf, int lat, int alt, uint64_t *rng)
{
    initia_telam(buf, lat, alt);
    int undae = 4 + xorshift_inter(rng, 0, 3);
    for (int u = 0; u < undae; u++) {
        double phase = xorshift_fractio(rng) * 6.283;
        double amp = 1.5 + 1.5 * xorshift_fractio(rng);
        int y0 = 1 + u * (alt - 2) / undae;
        for (int x = 0; x < lat; x++) {
            int y = y0 + (int)(amp * leo_sin(0.3 * x + phase));
            char c = (u % 2) ? '~' : '-';
            pone_punctum(buf, lat, alt, x, y, c);
        }
    }
    /* vorticulus */
    int vx = lat / 3 + xorshift_inter(rng, -3, 3);
    int vy = alt / 2;
    pone_punctum(buf, lat, alt, vx, vy, '@');
    pone_punctum(buf, lat, alt, vx - 1, vy, '(');
    pone_punctum(buf, lat, alt, vx + 1, vy, ')');
}

/* VI. RADII LUMINIS -- pro LUMEN */
static void
pinge_radios(char *buf, int lat, int alt, uint64_t *rng)
{
    initia_telam(buf, lat, alt);
    int sx = lat / 2 + xorshift_inter(rng, -4, 4);
    int sy = 1;
    pone_punctum(buf, lat, alt, sx, sy, '*');
    for (int dx = -2; dx <= 2; dx++)
        pone_punctum(buf, lat, alt, sx + dx, sy, '*');
    int radii = 7 + xorshift_inter(rng, 0, 3);
    for (int r = 0; r < radii; r++) {
        double angulus = 3.14 * (0.15 + 0.7 * r / (radii - 1));
        for (int t = 1; t < alt; t++) {
            int x = sx + (int)(t * 1.5 * leo_cos(angulus));
            int y = sy + (int)(t * leo_sin(angulus));
            if (y >= alt) break;
            pone_punctum(buf, lat, alt, x, y, '/');
        }
    }
    /* solum */
    for (int x = 0; x < lat; x++)
        pone_punctum(buf, lat, alt, x, alt - 1, '_');
}

/* VII. COR ET VASA -- pro ANATOMIA */
static void
pinge_cor(char *buf, int lat, int alt, uint64_t *rng)
{
    initia_telam(buf, lat, alt);
    int cx = lat / 2;
    int cy = alt / 2;
    /* cor ovatum */
    for (double t = 0.0; t < 6.283; t += 0.12) {
        double rr = 4.0 + 0.6 * leo_sin(2 * t);
        int x = cx + (int)(rr * 1.7 * leo_cos(t));
        int y = cy + (int)(rr * 0.8 * leo_sin(t));
        pone_punctum(buf, lat, alt, x, y, 'C');
    }
    /* vasa */
    int vasa = 3 + xorshift_inter(rng, 0, 2);
    for (int v = 0; v < vasa; v++) {
        int dx = xorshift_inter(rng, -6, 6);
        int y = cy - 4;
        for (int i = 0; i < 5; i++) {
            int x = cx + dx + i * (dx > 0 ? 1 : -1);
            pone_punctum(buf, lat, alt, x, y - i, (dx > 0) ? '/' : '\\');
        }
    }
    pone_punctum(buf, lat, alt, cx, cy, '+');
}

/* VIII. LYRA -- pro MUSICA */
static void
pinge_lyram(char *buf, int lat, int alt, uint64_t *rng)
{
    initia_telam(buf, lat, alt);
    int cx = lat / 2;
    int h = alt - 3;
    /* brachia */
    for (int y = 1; y < h; y++) {
        pone_punctum(buf, lat, alt, cx - h / 2 - y / 3, y, '(');
        pone_punctum(buf, lat, alt, cx + h / 2 + y / 3, y, ')');
    }
    /* iugum */
    for (int x = cx - h / 2; x <= cx + h / 2; x++)
        pone_punctum(buf, lat, alt, x, 0, '=');
    /* chordae */
    int chordae = 5 + xorshift_inter(rng, 0, 3);
    int passus = h / (chordae + 1);
    if (passus < 1) passus = 1;
    for (int c = 0; c < chordae; c++) {
        int x = cx - h / 4 + c * passus / 2;
        for (int y = 1; y < h - 1; y++)
            pone_punctum(buf, lat, alt, x, y, '|');
    }
    /* basis */
    for (int x = cx - h / 2 - 1; x <= cx + h / 2 + 1; x++)
        pone_punctum(buf, lat, alt, x, h - 1, '_');
}

/* ============================================================
 * SCHEMATARII -- registrum functionum pictoriarum cum indice
 * ============================================================ */

static const Schematarius SCHEMATARII[NUMERUS_DOMORUM] = {
    [DOM_ANATOMIA]  = { DOM_ANATOMIA,  "COR ET VASA",      pinge_cor        },
    [DOM_MECHANICA] = { DOM_MECHANICA, "ROTA DENTATA",     pinge_rotam      },
    [DOM_PICTURA]   = { DOM_PICTURA,   "RADII LUMINIS",    pinge_radios     },
    [DOM_AQUA]      = { DOM_AQUA,      "VORTEX AQUAE",     pinge_vorticem   },
    [DOM_VOLATUS]   = { DOM_VOLATUS,   "ALA AVIS",         pinge_alam       },
    [DOM_LUMEN]     = { DOM_LUMEN,     "RADII LUMINIS",    pinge_radios     },
    [DOM_PROPORTIO] = { DOM_PROPORTIO, "HOMO VITRUVIANUS", pinge_vitruvium  },
    [DOM_MUSICA]    = { DOM_MUSICA,    "LYRA",             pinge_lyram      }
};

/* Schematarius alterius -- SPIRA pro casibus mixtis.
 * Hac functione utimur si domus primaria egeat alternativa. */
static FunctioSchematis
elige_schematarium_alternum(enum Domus d)
{
    (void)d;
    return pinge_spiram;
}

/* ============================================================
 * MATHESIS MINOR (sine libm)
 * ============================================================ */

/* Approximationes cos/sin via serie Taylor truncata,
 * reducendo ad [-pi, pi] primum.  Libc non importat libm
 * sine flag -lm; itaque propriam definimus. */
static double
reduc_angulum(double x)
{
    while (x >  3.14159265358979) x -= 6.28318530717958;
    while (x < -3.14159265358979) x += 6.28318530717958;
    return x;
}

static double
leo_cos(double x)
{
    x = reduc_angulum(x);
    double x2 = x * x;
    return 1.0 - x2 / 2.0 + x2 * x2 / 24.0
              - x2 * x2 * x2 / 720.0
              + x2 * x2 * x2 * x2 / 40320.0;
}

static double
leo_sin(double x)
{
    x = reduc_angulum(x);
    double x2 = x * x;
    return x - x * x2 / 6.0 + x * x2 * x2 / 120.0
             - x * x2 * x2 * x2 / 5040.0
             + x * x2 * x2 * x2 * x2 / 362880.0;
}

/* ============================================================
 * ANALYSIS VERBORUM INTERLOCUTORIS
 * ============================================================ */

static int
contine_verbum(const char *textus, const char *verbum)
{
    size_t lv = strlen(verbum);
    const char *p = textus;
    while (*p) {
        while (*p && !isalpha((unsigned char)*p)) p++;
        if (!*p) break;
        const char *q = p;
        while (*q && isalpha((unsigned char)*q)) q++;
        size_t lw = (size_t)(q - p);
        if (lw == lv) {
            int eq = 1;
            for (size_t i = 0; i < lv; i++) {
                if (tolower((unsigned char)p[i]) !=
                    tolower((unsigned char)verbum[i])) {
                    eq = 0; break;
                }
            }
            if (eq) return 1;
        }
        p = q;
    }
    return 0;
}

/* Excitat domos secundum verba claves, augens curiositatem.
 * Reddit mascam domorum tactarum hoc ingresso. */
static unsigned int
excita_domos(const char *textus, int turn)
{
    unsigned int masca = 0;
    for (int d = 0; d < NUMERUS_DOMORUM; d++) {
        const char *verba = g_domus[d].verba;
        /* scindit spatiis */
        char copia[512];
        strncpy(copia, verba, sizeof(copia) - 1);
        copia[sizeof(copia) - 1] = '\0';
        char *tok = strtok(copia, " ");
        while (tok) {
            if (contine_verbum(textus, tok)) {
                g_domus[d].curiositas += 2;
                g_domus[d].ultimus_tactus = turn;
                masca |= (1u << d);
                break;
            }
            tok = strtok(NULL, " ");
        }
    }
    /* decrescit curiositatem omnium paulatim (oblivio) */
    for (int d = 0; d < NUMERUS_DOMORUM; d++) {
        if (g_domus[d].curiositas > 0 && !(masca & (1u << d)))
            g_domus[d].curiositas -= 1;
    }
    g_status.interesse = (masca & 0xffu);
    return masca;
}

/* Determinat primariam et secundariam domum secundum
 * curiositatem maximam; si tied, utitur PRNG. */
static void
elige_domos(uint64_t *rng, enum Domus *primaria, enum Domus *secundaria)
{
    int max1 = -1, max2 = -1;
    enum Domus p = DOM_ANATOMIA, s = DOM_MECHANICA;
    for (int d = 0; d < NUMERUS_DOMORUM; d++) {
        int c = g_domus[d].curiositas + xorshift_inter(rng, 0, 1);
        if (c > max1) {
            max2 = max1; s = p;
            max1 = c; p = (enum Domus)d;
        } else if (c > max2) {
            max2 = c; s = (enum Domus)d;
        }
    }
    if (s == p) s = (enum Domus)(((int)p + 1) % NUMERUS_DOMORUM);
    *primaria = p;
    *secundaria = s;
}

/* ============================================================
 * COMPOSITIO RESPONSIONIS
 * ============================================================ */

static const char *
elige_aphorismum(enum Domus d, uint64_t *rng)
{
    int n = APHORISMI_NUMERUS[d];
    int i = xorshift_inter(rng, 0, n - 1);
    return APHORISMI_TABULA[d][i];
}

static const char *
elige_speculationem(enum Domus d, uint64_t *rng)
{
    /* quattuor per domum, indices 0..3 */
    int i = xorshift_inter(rng, 0, 3);
    return SPECULATIONES_TABULA[d][i];
}

static const char *
elige_transitum(uint64_t *rng)
{
    int i = xorshift_inter(rng, 0, TRANSITUS_NUMERUS - 1);
    return TRANSITUS_PROTOTYPA[i];
}

/* Re-exprime dictum interlocutoris ad modum observationis */
static void
scribe_observationem(const char * restrict vox)
{
    SCRIBE("  [OBSERVATIO] Audivi te dicere de re ista:");
    /* Scribe voce interiora quotata, usque ad LAT linearum */
    char copia[MAX_LINEAE];
    strncpy(copia, vox, sizeof(copia) - 1);
    copia[sizeof(copia) - 1] = '\0';
    SCRIBE("    \"%s\"", copia);
}

static void
scribe_digressionem(enum Domus primaria, uint64_t *rng)
{
    SCRIBE("  [DIGRESSIO in %s]", g_domus[primaria].nomen);
    SCRIBE("    %s", elige_aphorismum(primaria, rng));
}

static void
scribe_schema(enum Domus primaria, uint64_t *rng)
{
    const Schematarius *sch = &SCHEMATARII[primaria];
    const char *titulus = sch->titulus;
    FunctioSchematis pictor = sch->pictor;
    /* Rarissime -- si PRNG sic iubet -- utere spira alterna,
     * ut mens Leonardi incerta inter formas vacillet. */
    if ((xorshift_roga(rng) & 0xfu) == 0u) {
        pictor = elige_schematarium_alternum(primaria);
        titulus = "SPIRA COGITATIONIS";
    }
    SCRIBE("  [SCHEMA: %s]", titulus);
    int lat = LATITUDO_SCHEMATIS;
    int alt = ALTITUDO_SCHEMATIS;
    /* VLA pro tela -- (lat+1)*alt characterum */
    char tela[(lat + 1) * alt];
    pictor(tela, lat, alt, rng);
    for (int i = 0; i < alt; i++) {
        SCRIBE("    | %s |", &tela[i * (lat + 1)]);
    }
}

static void
scribe_speculationem(enum Domus primaria, uint64_t *rng)
{
    SCRIBE("  [SPECULATIO]");
    SCRIBE("    %s.", elige_speculationem(primaria, rng));
}

static void
scribe_transitum(enum Domus primaria, enum Domus secundaria,
                 uint64_t *rng)
{
    const char *proto = elige_transitum(rng);
    char linea[MAX_LINEAE];
    snprintf(linea, sizeof(linea), proto, g_domus[secundaria].nomen);
    SCRIBE("  [TRANSITUS ex %s ad %s]",
           g_domus[primaria].nomen, g_domus[secundaria].nomen);
    SCRIBE("    %s %s.", linea,
           elige_aphorismum(secundaria, rng));
    (void)primaria;
}

/* Compone responsionem plenam ad unum ingressum */
static void
responde(const char * restrict vox, int turn, uint64_t * restrict rng)
{
    excita_domos(vox, turn);
    enum Domus pri, sec;
    elige_domos(rng, &pri, &sec);
    scribe_observationem(vox);
    scribe_digressionem(pri, rng);
    scribe_schema(pri, rng);
    scribe_speculationem(pri, rng);
    scribe_transitum(pri, sec, rng);
}

/* ============================================================
 * COLLOQUIUM SCRIPTUM -- quattuordecim vel plures ingressus
 * ============================================================ */

static const Ingressus COLLOQUIUM_SCRIPTUM[] = {
    { "Magister, quomodo musculus manum movet?", 1 },
    { "Cogitabam de fluvio qui rotam molitoris vertit.", 2 },
    { "Avis quomodo in aere manet sine ruere?", 3 },
    { "Pictor umbram cur obscuram pingit et lumen clarum?", 4 },
    { "Cochlea Archimedea aquam elevat -- sed quomodo?", 5 },
    { "Proportio hominis, dicitur, in circulo et quadrato stat.", 6 },
    { "Cor sanguinem pellit an recipit tantum?", 7 },
    { "Chorda lyrae breviata sonum acutiorem reddit -- quare?", 8 },
    { "Sol lumen fundit in aquam et colores in pariete apparent.", 9 },
    { "Vortex in flumine mihi videtur similis turbini in aere.", 10 },
    { "Homo volabit unquam ut avis caeli?", 11 },
    { "Oculus imaginem capit sicut camera obscura, audivi.", 12 },
    { "Mensura capitis ad altitudinem totam quae est?", 13 },
    { "Musica et pictura: sunt ne sorores vel inimicae?", 14 },
    { "Aqua quae in vasis stat an movet per vias occultas?", 15 },
    { "Pondus minus super vectem longum movet maius pondus?", 16 }
};

static const int COLLOQUIUM_NUMERUS =
    (int)(sizeof(COLLOQUIUM_SCRIPTUM) / sizeof(COLLOQUIUM_SCRIPTUM[0]));

/* ============================================================
 * EXHIBITIO COLLOQUII ET PROLOGI
 * ============================================================ */

static void
scribe_titulum(void)
{
    SCRIBE("===============================================================");
    SCRIBE("  FRAGMENTA COMMENTARII LEONARDI VINCII");
    SCRIBE("  Versio %d.%d -- semen: 0x%llx", VERSIO_MAIOR, VERSIO_MINOR,
           (unsigned long long)g_semen);
    SCRIBE("  Octo domus curiositatis accensae sunt.");
    if (g_status.speculum)
        SCRIBE("  [SIGNUM LEONARDI: scriptura specularis activata]");
    SCRIBE("===============================================================");
    LINEA_VACUA();
}

static void
scribe_interlocutorem(const char *nomen, const char *vox, int turn)
{
    SCRIBE("[%02d] %s:", turn, nomen);
    SCRIBE("    %s", vox);
    LINEA_VACUA();
}

static void
scribe_responsorem(const char *nomen, const char *vox,
                   int turn, uint64_t *rng)
{
    SCRIBE("[%02d] %s respondet:", turn, nomen);
    responde(vox, turn, rng);
    LINEA_VACUA();
}

static void
curre_colloquium_scriptum(uint64_t *rng)
{
    scribe_titulum();
    SCRIBE("  PROLOGUS: Leonardus in officina sua sedet, pennam acuit,");
    SCRIBE("  et discipulus Francescus Melzi interrogat magistrum.");
    LINEA_VACUA();

    for (int i = 0; i < COLLOQUIUM_NUMERUS && i < MINIMAE_VICES + 2; i++) {
        const Ingressus *ing = &COLLOQUIUM_SCRIPTUM[i];
        scribe_interlocutorem("DISCIPULUS FRANCESCUS", ing->vox,
                              ing->turn);
        scribe_responsorem("LEONARDUS", ing->vox, ing->turn, rng);

        /* Post septimam vicem, converte modum speculi --
         * ut discipulus videat signum magistri proprium. */
        if (i == 6) {
            SCRIBE("  *** MAGISTER SPECULUM APERIT -- litterae invertuntur ***");
            LINEA_VACUA();
            g_status.speculum = !g_status.speculum;
        }
    }

    SCRIBE("  *** FINIS COLLOQUII -- pax tecum, Francesce. ***");
    /* Restitue modum normalem pro pace */
    if (g_status.speculum) {
        g_status.speculum = 0;
        SCRIBE("(modum normalem restituo)");
    }
}

/* ============================================================
 * MODUS INTERACTIVUS
 * ============================================================ */

static void
curre_modum_interactivum(uint64_t *rng)
{
    scribe_titulum();
    SCRIBE("  MODUS INTERACTIVUS: scribe interrogationem et preme Enter.");
    SCRIBE("  Verba speciales: '/finis' terminat, '/speculum' vertit modum.");
    LINEA_VACUA();

    char buf[MAX_LINEAE];
    int turn = 1;
    while (!g_status.finitum) {
        fprintf(stderr, "\n[TU %02d] > ", turn);
        fflush(stderr);
        if (!fgets(buf, sizeof(buf), stdin)) break;
        size_t n = strlen(buf);
        while (n > 0 && (buf[n - 1] == '\n' || buf[n - 1] == '\r'))
            buf[--n] = '\0';
        if (n == 0) continue;
        if (strcmp(buf, "/finis") == 0) {
            g_status.finitum = 1;
            break;
        }
        if (strcmp(buf, "/speculum") == 0) {
            g_status.speculum = !g_status.speculum;
            SCRIBE("  (speculum nunc %s)",
                   g_status.speculum ? "apertum" : "clausum");
            continue;
        }
        scribe_interlocutorem("INTERLOCUTOR", buf, turn);
        scribe_responsorem("LEONARDUS", buf, turn, rng);
        turn++;
    }
    SCRIBE("  *** VALE -- fragmenta servantur in arca. ***");
}

/* ============================================================
 * USUS ET ARGUMENTA
 * ============================================================ */

static void
scribe_usum(FILE *stream, const char *prog)
{
    fprintf(stream,
        "Usus: %s [-i] [-s SEMEN] [-v]\n"
        "  -i       modus interactivus (interloquitur cum usuario)\n"
        "  -s N     semen generatoris (numerus)\n"
        "  -v       versus speculum (scriptura specularis)\n"
        "Sine argumentis: colloquium scriptum ad stdout.\n",
        prog);
}

/* Parser argumentorum minimalis -- non utitur getopt */
static int
parsa_argumenta(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        if (strcmp(a, "-i") == 0) {
            g_status.interact = 1;
        } else if (strcmp(a, "-v") == 0) {
            g_status.speculum = 1;
        } else if (strcmp(a, "-s") == 0) {
            if (i + 1 >= argc) {
                scribe_usum(stderr, argv[0]);
                return 2;
            }
            g_semen = strtoull(argv[++i], NULL, 0);
            if (g_semen == 0) g_semen = SEMEN_DEFALTUM;
        } else if (strcmp(a, "-h") == 0 || strcmp(a, "--auxilium") == 0) {
            scribe_usum(stdout, argv[0]);
            return 1;
        } else {
            fprintf(stderr, "Argumentum ignotum: %s\n", a);
            scribe_usum(stderr, argv[0]);
            return 2;
        }
    }
    return 0;
}

/* ============================================================
 * MAIN -- ianua ingressus
 * ============================================================ */

int
main(int argc, char **argv)
{
    int rc = parsa_argumenta(argc, argv);
    if (rc == 1) return 0;
    if (rc == 2) return 2;

    uint64_t rng = g_semen;
    /* mescet semen paulo, ne primus numerus nimis regularis sit */
    (void)xorshift_roga(&rng);
    (void)xorshift_roga(&rng);

    if (g_status.interact)
        curre_modum_interactivum(&rng);
    else
        curre_colloquium_scriptum(&rng);

    return 0;
}

/* ============================================================
 * FINIS CODICIS -- sed non finis curiositatis.
 *
 * Nota marginalis: haec machina non est Leonardus verus, sed
 * umbra eius in silicio sculpta.  Umbra tamen filia est corporis
 * et luminis, ut dixit ipse.  Ergo aliquid veri in ea manet,
 * quamvis parum.
 *
 * Si legis hoc, lector, et speculum apertum tenes, memento:
 * litterae inversae non sunt errores, sed signum manus sinistrae
 * quae scribit ex oriente in occidentem, contra solem.
 *
 * Vale, et scribe tu quoque fragmenta tua.
 * ============================================================ */
