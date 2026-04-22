/*
 * hypatia.c — Automaton Loquens Hypatiae Alexandrinae
 *
 * Simulacrum dialecticum mathematico-philosophicum secundum mentem
 * Hypatiae Alexandrinae, magistrae neoplatonicae, quae geometriam,
 * arithmeticam, astronomiam, musicamque discipulis tradidit.
 *
 * Classificator dominii verba clavia inspicit et inter septem regiones
 * scientiae distinguit: NUMERUS, FIGURA, ASTRUM, MUSICA, PHILOSOPHIA,
 * ANIMA, FORMAE.  Responsum constat ex quattuor partibus: SALUTATIO,
 * PROPOSITIO (ex corpore theorematum ac sententiarum), DEMONSTRATIO
 * (tabula numerica vel figura geometrica in cratere VLA depicta),
 * ANAGOGE (ascensus neoplatonicus ad principium metaphysicum).
 *
 * Numerator "anabasis" per colloquium crescit, quo altior fit ascensus:
 * a sensibilibus per animam ad nuntium (nous) denique ad unum (hen).
 *
 * Argumenta:
 *   (nullum)   — colloquium automaticum inter Hypatiam et discipulum.
 *   -i         — modus interactivus: discipulus interrogat per tty.
 *   -s N       — semen deterministicum pro generatore xorshift64.
 *   aliud      — usus latinus ad stderr, exitus 2.
 *
 * Sub gcc -std=c99 -Wall -Wextra -Wpedantic -O2 sine monitis compilat.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <math.h>

/* ========================================================================
 * CONSTANTES ET MACROS GLOBALES
 * ======================================================================== */

#define VERSIO_AUTOMATI      "hypatia/1.0"
#define LIMES_LINEAE         256
#define LIMES_VERBI          64
#define LIMES_VERBORUM       32
#define COLLOQUIUM_TURNOS    16
#define CANVAS_DEFAULT_LAT   48
#define CANVAS_DEFAULT_ALT   20
#define ANABASIS_GRADUS      5
#define CORPUS_MINIMUM       30

/* Macro variadica: imprimit ad tubum egressus cum margine praefixo */
#define NARRA(...)           do { fprintf(stdout, __VA_ARGS__); } while (0)
#define ERRARE(...)          do { fprintf(stderr, __VA_ARGS__); } while (0)

/* Numeri pi, phi, radicis duorum — constantes neoplatonicae */
#define PI_NUMERUS           3.14159265358979323846
#define PHI_NUMERUS          1.61803398874989484820
#define RADIX_DUORUM         1.41421356237309504880

/* ========================================================================
 * GENERATOR XORSHIFT64 — DETERMINISTICUS
 * ======================================================================== */

/* Status generatoris in unione positus, ut bitfield ostendamus */
typedef union {
    uint64_t integer;
    struct {
        uint64_t bassus  : 32;
        uint64_t altus   : 32;
    } partes;
} StatusFortunae;

static StatusFortunae fortuna = { .integer = 0x1D2B3F4C5A6E7081ULL };

/* Miscet semen xorshift64 modo canonico */
static inline uint64_t
misce_xorshift(void)
{
    uint64_t x = fortuna.integer;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    fortuna.integer = x;
    return x;
}

/* Eligit numerum inter 0 et limes-1 */
static inline uint32_t
sors_usque(uint32_t limes)
{
    if (limes == 0) return 0;
    return (uint32_t)(misce_xorshift() % limes);
}

/* Fractio pseudo-aleatoria inter 0.0 et 1.0 */
static inline double
sors_fractio(void)
{
    return (double)(misce_xorshift() & 0xFFFFFFFFULL) / 4294967296.0;
}

/* Seminat generatorem ex numero dato */
static void
semina_fortunam(uint64_t semen)
{
    if (semen == 0) semen = 0x9E3779B97F4A7C15ULL;
    fortuna.integer = semen;
    /* Agitamus ut status diffundatur */
    for (int i = 0; i < 16; i++) (void)misce_xorshift();
}

/* ========================================================================
 * DOMINIUM: CLASSIFICATIO ARGUMENTORUM
 * ======================================================================== */

typedef enum {
    DOM_NUMERUS = 0,
    DOM_FIGURA,
    DOM_ASTRUM,
    DOM_MUSICA,
    DOM_PHILOSOPHIA,
    DOM_ANIMA,
    DOM_FORMAE,
    DOM_IGNOTUM,
    DOM_FINIS
} Dominium;

static const char *nomen_dominii[DOM_FINIS] = {
    "NUMERUS", "FIGURA", "ASTRUM", "MUSICA",
    "PHILOSOPHIA", "ANIMA", "FORMAE", "IGNOTUM"
};

/* Tabula verborum clavium pro singulis dominiis */
typedef struct {
    Dominium dom;
    const char *verbum;
} ClavisDominii;

static const ClavisDominii claves[] = {
    { DOM_NUMERUS,     "numerus"     },
    { DOM_NUMERUS,     "arithmos"    },
    { DOM_NUMERUS,     "unitas"      },
    { DOM_NUMERUS,     "divisor"     },
    { DOM_NUMERUS,     "primus"      },
    { DOM_NUMERUS,     "pythagoras"  },
    { DOM_NUMERUS,     "triplum"     },
    { DOM_NUMERUS,     "triangulus"  },
    { DOM_FIGURA,      "figura"      },
    { DOM_FIGURA,      "circulus"    },
    { DOM_FIGURA,      "conus"       },
    { DOM_FIGURA,      "ellipsis"    },
    { DOM_FIGURA,      "parabola"    },
    { DOM_FIGURA,      "hyperbole"   },
    { DOM_FIGURA,      "polygonum"   },
    { DOM_FIGURA,      "geometria"   },
    { DOM_FIGURA,      "linea"       },
    { DOM_ASTRUM,      "astrum"      },
    { DOM_ASTRUM,      "stella"      },
    { DOM_ASTRUM,      "planeta"     },
    { DOM_ASTRUM,      "ecliptica"   },
    { DOM_ASTRUM,      "sol"         },
    { DOM_ASTRUM,      "luna"        },
    { DOM_ASTRUM,      "caelum"      },
    { DOM_ASTRUM,      "astronomia"  },
    { DOM_MUSICA,      "musica"      },
    { DOM_MUSICA,      "harmonia"    },
    { DOM_MUSICA,      "monochordum" },
    { DOM_MUSICA,      "consonantia" },
    { DOM_MUSICA,      "ratio"       },
    { DOM_MUSICA,      "chorda"      },
    { DOM_MUSICA,      "diapason"    },
    { DOM_PHILOSOPHIA, "philosophia" },
    { DOM_PHILOSOPHIA, "syllogismus" },
    { DOM_PHILOSOPHIA, "dialectica"  },
    { DOM_PHILOSOPHIA, "veritas"     },
    { DOM_PHILOSOPHIA, "sapientia"   },
    { DOM_PHILOSOPHIA, "ratiocinium" },
    { DOM_ANIMA,       "anima"       },
    { DOM_ANIMA,       "psyche"      },
    { DOM_ANIMA,       "mens"        },
    { DOM_ANIMA,       "nous"        },
    { DOM_ANIMA,       "intellectus" },
    { DOM_ANIMA,       "ascensus"    },
    { DOM_ANIMA,       "anagoge"     },
    { DOM_FORMAE,      "forma"       },
    { DOM_FORMAE,      "idea"        },
    { DOM_FORMAE,      "eidos"       },
    { DOM_FORMAE,      "unum"        },
    { DOM_FORMAE,      "hen"         },
    { DOM_FORMAE,      "bonum"       },
    { DOM_FORMAE,      "solidum"     },
    { DOM_FORMAE,      "platonicus"  }
};

static const size_t n_claves = sizeof(claves) / sizeof(claves[0]);

/* Convertit litteras ad minusculas in loco */
static void
ad_minusculas(char *restrict verbum)
{
    for (char *p = verbum; *p; p++)
        *p = (char)tolower((unsigned char)*p);
}

/* Classificat textum ingressum secundum verba clavia */
static Dominium
classifica_textum(const char *textus)
{
    int numeri[DOM_FINIS] = { 0 };
    char buffer[LIMES_LINEAE];
    size_t j = 0;
    size_t lon = strlen(textus);
    if (lon >= LIMES_LINEAE) lon = LIMES_LINEAE - 1;

    for (size_t i = 0; i < lon; i++) {
        unsigned char c = (unsigned char)textus[i];
        if (isalpha(c)) {
            if (j < sizeof(buffer) - 1)
                buffer[j++] = (char)tolower(c);
        } else {
            if (j > 0) {
                buffer[j] = '\0';
                for (size_t k = 0; k < n_claves; k++) {
                    if (strstr(buffer, claves[k].verbum) != NULL) {
                        numeri[claves[k].dom]++;
                        break;
                    }
                }
                j = 0;
            }
        }
    }
    if (j > 0) {
        buffer[j] = '\0';
        for (size_t k = 0; k < n_claves; k++) {
            if (strstr(buffer, claves[k].verbum) != NULL) {
                numeri[claves[k].dom]++;
                break;
            }
        }
    }

    Dominium maxim = DOM_IGNOTUM;
    int summum = 0;
    for (int d = 0; d < DOM_IGNOTUM; d++) {
        if (numeri[d] > summum) {
            summum = numeri[d];
            maxim = (Dominium)d;
        }
    }
    return maxim;
}

/* ========================================================================
 * CORPUS PROPOSITIONUM — THEOREMATA ET APHORISMI PER X-MACRO
 * ======================================================================== */

/*
 * Unusquisque ingressus in corpore continet dominium et propositionem.
 * X-macro permittit easdem sententias et pro indice et pro textu expandi.
 */

#define CORPUS_PROPOSITIONUM                                                 \
    X(DOM_NUMERUS,     "Numerus est principium quo omnia ordinantur.")       \
    X(DOM_NUMERUS,     "Unitas non est numerus, sed fons numerorum.")        \
    X(DOM_NUMERUS,     "Numerus perfectus est qui summae divisorum par.")    \
    X(DOM_NUMERUS,     "Tria, quattuor, quinque — triplum pythagoricum.")    \
    X(DOM_NUMERUS,     "Quadratum hypotenusae catheta quadrata iungit.")     \
    X(DOM_FIGURA,      "Circulus est locus punctorum aequidistantium.")      \
    X(DOM_FIGURA,      "Conus sectus parit ellipsim, parabolam, hyperbolen.")\
    X(DOM_FIGURA,      "Triangulus tres angulos habet duobus rectis pares.") \
    X(DOM_FIGURA,      "Polygonum regulare latera et angulos aequales.")     \
    X(DOM_FIGURA,      "Sectio aurea divisionem harmonicam praestat.")       \
    X(DOM_ASTRUM,      "Sol centrum est motus apparentis planetarum.")       \
    X(DOM_ASTRUM,      "Ecliptica zodiaci viam per stellas describit.")      \
    X(DOM_ASTRUM,      "Luna a sole lumen mutuatum hominibus ostendit.")     \
    X(DOM_ASTRUM,      "Astrorum motus numeris caelestibus paret.")          \
    X(DOM_MUSICA,      "Diapason consonantia est rationis duplae.")          \
    X(DOM_MUSICA,      "Diapente tribus ad duo, diatessaron quattuor tribus.")\
    X(DOM_MUSICA,      "Harmonia mundi eisdem numeris ac musica regitur.")   \
    X(DOM_MUSICA,      "Chorda monochordi rationes integras canit.")         \
    X(DOM_PHILOSOPHIA, "Sapientia principium est beatae vitae.")             \
    X(DOM_PHILOSOPHIA, "Dialectica est ars quaerendi ac respondendi.")       \
    X(DOM_PHILOSOPHIA, "Syllogismus ex duabus praemissis conclusionem gignit.")\
    X(DOM_PHILOSOPHIA, "Veritas non mutatur secundum opiniones hominum.")    \
    X(DOM_ANIMA,       "Anima figuris geometricis similitudinem habet.")     \
    X(DOM_ANIMA,       "Intellectus ex sensibus ad formas ascendit.")        \
    X(DOM_ANIMA,       "Anima tres habet partes: rationalem, irascibilem, cupidinem.")\
    X(DOM_ANIMA,       "Anagoge est ascensus mentis ad divina.")             \
    X(DOM_FORMAE,      "Formae aeternae sunt exemplaria rerum sensibilium.") \
    X(DOM_FORMAE,      "Unum supra essentiam et supra intellectum manet.")   \
    X(DOM_FORMAE,      "Bonum fons lucis est omnibus entibus.")              \
    X(DOM_FORMAE,      "Quinque corpora platonica sunt elementorum signa.")  \
    X(DOM_FORMAE,      "Tetraedron igni, cubus terrae, octaedron aeri congruit.")\
    X(DOM_FORMAE,      "Dodecaedron caelo, icosaedron aquae adscribitur.")

typedef struct {
    Dominium dom;
    const char *textus;
} Propositio;

#define X(d, t) { d, t },
static const Propositio corpus_propositionum[] = {
    CORPUS_PROPOSITIONUM
};
#undef X

static const size_t n_propositionum =
    sizeof(corpus_propositionum) / sizeof(corpus_propositionum[0]);

/* Seligit propositionem dominii congruentem, vel quamlibet si nulla */
static const Propositio *
selige_propositionem(Dominium dom)
{
    size_t candidati[128];
    size_t n_cand = 0;
    for (size_t i = 0; i < n_propositionum && n_cand < 128; i++) {
        if (corpus_propositionum[i].dom == dom)
            candidati[n_cand++] = i;
    }
    if (n_cand == 0) {
        /* Nulla propositio propria: quamlibet eligimus */
        return &corpus_propositionum[sors_usque((uint32_t)n_propositionum)];
    }
    return &corpus_propositionum[candidati[sors_usque((uint32_t)n_cand)]];
}

/* ========================================================================
 * CANVAS GEOMETRICUS — VLA ADHIBITUS
 * ======================================================================== */

/*
 * Crater (canvas) ASCII, in quo figurae delineantur.
 * Usus VLA: dimensio in tempore executionis determinatur.
 */

static void
purga_craterem(int alt, int lat, char crater[alt][lat])
{
    for (int y = 0; y < alt; y++)
        for (int x = 0; x < lat; x++)
            crater[y][x] = ' ';
}

static void
imprime_craterem(int alt, int lat, char crater[alt][lat])
{
    /* Margo superior */
    NARRA("    +");
    for (int x = 0; x < lat; x++) NARRA("-");
    NARRA("+\n");
    for (int y = 0; y < alt; y++) {
        NARRA("    |");
        for (int x = 0; x < lat; x++) fputc(crater[y][x], stdout);
        NARRA("|\n");
    }
    NARRA("    +");
    for (int x = 0; x < lat; x++) NARRA("-");
    NARRA("+\n");
}

static inline void
pone_punctum(int alt, int lat, char crater[alt][lat], int x, int y, char c)
{
    if (x >= 0 && x < lat && y >= 0 && y < alt)
        crater[y][x] = c;
}

/* ========================================================================
 * DEMONSTRATORES: SEX FUNCTIONES GEOMETRICAE ET NUMERICAE
 * ======================================================================== */

/*
 * Unaquaeque functio demonstrans craterem VLA accipit atque figuram depingit
 * vel tabulam numericam imprimit.  Functiones ad tabulam dispatch adduntur.
 */

typedef struct ContextusDemonstrandi ContextusDemonstrandi;
struct ContextusDemonstrandi {
    int anabasis;
    int turnus;
    Dominium dom;
    const char *nomen;
    unsigned flagra : 4;
    unsigned reservatum : 4;
};

typedef void (*FunctioDemonstrandi)(const ContextusDemonstrandi *ctx);

/* -------- 1. CIRCULUS -------- */
static void
demonstra_circulum(const ContextusDemonstrandi *ctx)
{
    (void)ctx;
    int alt = CANVAS_DEFAULT_ALT;
    int lat = CANVAS_DEFAULT_LAT;
    char crater[alt][lat];
    purga_craterem(alt, lat, crater);

    double cx = lat / 2.0;
    double cy = alt / 2.0;
    double radius = (alt < lat ? alt : lat) * 0.40;
    /* Axem horizontalem multiplicamus ut forma rotunda appareat */
    double ratio_axium = 2.1;

    for (double theta = 0.0; theta < 2.0 * PI_NUMERUS; theta += 0.03) {
        int x = (int)(cx + ratio_axium * radius * cos(theta) * 0.5);
        int y = (int)(cy + radius * sin(theta));
        pone_punctum(alt, lat, crater, x, y, 'o');
    }
    /* Centrum notamus */
    pone_punctum(alt, lat, crater, (int)cx, (int)cy, '+');
    NARRA("    CIRCULUS PERFECTUS — locus punctorum aequidistantium:\n");
    imprime_craterem(alt, lat, crater);
}

/* -------- 2. TRIANGULUS PYTHAGORICUS -------- */
static void
demonstra_triangulum(const ContextusDemonstrandi *ctx)
{
    (void)ctx;
    int alt = CANVAS_DEFAULT_ALT;
    int lat = CANVAS_DEFAULT_LAT;
    char crater[alt][lat];
    purga_craterem(alt, lat, crater);

    /* Triangulum rectangulum 3-4-5 proportionibus */
    int ax = 4, ay = alt - 3;
    int bx = lat - 6, by = alt - 3;
    int cx = 4, cy = 3;

    /* Cathetus horizontalis */
    for (int x = ax; x <= bx; x++) pone_punctum(alt, lat, crater, x, ay, '-');
    /* Cathetus verticalis */
    for (int y = cy; y <= ay; y++) pone_punctum(alt, lat, crater, ax, y, '|');
    /* Hypotenusa */
    double dx = bx - cx;
    double dy = by - cy;
    double steps = fabs(dx) > fabs(dy) ? fabs(dx) : fabs(dy);
    for (double t = 0.0; t <= steps; t += 1.0) {
        int x = (int)(cx + dx * t / steps);
        int y = (int)(cy + dy * t / steps);
        pone_punctum(alt, lat, crater, x, y, '*');
    }
    pone_punctum(alt, lat, crater, ax, ay, '#');
    pone_punctum(alt, lat, crater, bx, by, '#');
    pone_punctum(alt, lat, crater, cx, cy, '#');
    NARRA("    TRIANGULUS RECTANGULUS — ratio 3:4:5, a^2 + b^2 = c^2:\n");
    imprime_craterem(alt, lat, crater);
    NARRA("    Ecce: 9 + 16 = 25, unde hypotenusa quinque mensuris.\n");
}

/* -------- 3. SECTIO CONICA (PARABOLA) -------- */
static void
demonstra_parabolam(const ContextusDemonstrandi *ctx)
{
    (void)ctx;
    int alt = CANVAS_DEFAULT_ALT;
    int lat = CANVAS_DEFAULT_LAT;
    char crater[alt][lat];
    purga_craterem(alt, lat, crater);

    /* Axes */
    int y0 = alt - 2;
    int x0 = lat / 2;
    for (int x = 0; x < lat; x++) pone_punctum(alt, lat, crater, x, y0, '-');
    for (int y = 0; y < alt; y++) pone_punctum(alt, lat, crater, x0, y, '|');
    pone_punctum(alt, lat, crater, x0, y0, '+');

    /* Parabola y = a * (x - x0)^2 inverti super axem.
     * Parvam variationem per sortem fractionalem addimus. */
    double a = 0.06 + 0.01 * (sors_fractio() - 0.5);
    for (int x = 0; x < lat; x++) {
        double dx_v = (double)(x - x0);
        double y = (double)y0 - a * dx_v * dx_v;
        int yi = (int)y;
        if (yi >= 0 && yi < y0)
            pone_punctum(alt, lat, crater, x, yi, '.');
    }
    /* Focus ad 1/(4a) super vertice */
    int fy = y0 - (int)(1.0 / (4.0 * a));
    pone_punctum(alt, lat, crater, x0, fy, 'F');
    NARRA("    PARABOLA — sectio coni parallela lateri; F punctum focale:\n");
    imprime_craterem(alt, lat, crater);
}

/* -------- 4. POLYGONUM REGULARE (HEXAGONUM) -------- */
static void
demonstra_polygonum(const ContextusDemonstrandi *ctx)
{
    int alt = CANVAS_DEFAULT_ALT;
    int lat = CANVAS_DEFAULT_LAT;
    char crater[alt][lat];
    purga_craterem(alt, lat, crater);

    int lateraNum = 5 + (ctx->anabasis % 4); /* a quinque ad octo */
    double cx = lat / 2.0;
    double cy = alt / 2.0;
    double radius = (alt < lat ? alt : lat) * 0.38;
    double ratio_axium = 2.1;

    double angs[16];
    int px[16], py[16];
    for (int i = 0; i < lateraNum; i++) {
        angs[i] = (2.0 * PI_NUMERUS * i) / lateraNum - PI_NUMERUS / 2.0;
        px[i] = (int)(cx + ratio_axium * radius * cos(angs[i]) * 0.5);
        py[i] = (int)(cy + radius * sin(angs[i]));
    }
    /* Lineas inter apices tracimus */
    for (int i = 0; i < lateraNum; i++) {
        int j = (i + 1) % lateraNum;
        double dx = px[j] - px[i];
        double dy = py[j] - py[i];
        double steps = fabs(dx) > fabs(dy) ? fabs(dx) : fabs(dy);
        if (steps < 1.0) steps = 1.0;
        for (double t = 0.0; t <= steps; t += 1.0) {
            int x = (int)(px[i] + dx * t / steps);
            int y = (int)(py[i] + dy * t / steps);
            pone_punctum(alt, lat, crater, x, y, '*');
        }
    }
    pone_punctum(alt, lat, crater, (int)cx, (int)cy, '+');
    NARRA("    POLYGONUM REGULARE — %d latera, angulis interioribus aequis:\n",
          lateraNum);
    imprime_craterem(alt, lat, crater);
}

/* -------- 5. TABULA DIVISORUM (NUMERUS) -------- */
static void
demonstra_divisores(const ContextusDemonstrandi *ctx)
{
    (void)ctx;
    NARRA("    TABULA DIVISORUM AC NUMERORUM PERFECTORUM:\n");
    NARRA("    +--------+---------------------------+---------+\n");
    NARRA("    | NUMER. | DIVISORES (praeter ipsum) | SUMMA   |\n");
    NARRA("    +--------+---------------------------+---------+\n");
    for (int n = 2; n <= 30; n++) {
        int summa = 0;
        char buf[64];
        size_t blen = 0;
        buf[0] = '\0';
        for (int d = 1; d < n; d++) {
            if (n % d == 0) {
                summa += d;
                int wrote = snprintf(buf + blen, sizeof(buf) - blen,
                                     "%s%d", blen ? " " : "", d);
                if (wrote < 0 || (size_t)wrote >= sizeof(buf) - blen) break;
                blen += (size_t)wrote;
            }
        }
        const char *nota = "";
        if (summa == n) nota = " <- perfectus";
        else if (summa > n) nota = " <- abundans";
        else if (summa < n) nota = "";
        NARRA("    | %6d | %-25s | %-7d |%s\n", n, buf, summa, nota);
    }
    NARRA("    +--------+---------------------------+---------+\n");
    NARRA("    Numerus perfectus est summae divisorum par: VI et XXVIII.\n");
}

/* -------- 6. TRIPLUM PYTHAGORICUM (NUMERUS) -------- */
static void
demonstra_triplum_pythagoricum(const ContextusDemonstrandi *ctx)
{
    (void)ctx;
    NARRA("    VENATIO TRIPLI PYTHAGORICI — a^2 + b^2 = c^2:\n");
    NARRA("    +-----+-----+-----+-----------+\n");
    NARRA("    |  a  |  b  |  c  |  a^2+b^2  |\n");
    NARRA("    +-----+-----+-----+-----------+\n");
    int inventa = 0;
    for (int a = 3; a <= 20 && inventa < 10; a++) {
        for (int b = a; b <= 20 && inventa < 10; b++) {
            int q = a * a + b * b;
            int c = (int)(sqrt((double)q) + 0.5);
            if (c * c == q && c <= 30) {
                NARRA("    | %3d | %3d | %3d | %9d |\n", a, b, c, q);
                inventa++;
            }
        }
    }
    NARRA("    +-----+-----+-----+-----------+\n");
}

/* -------- 7. SCHEMA ECLIPTICAE (ASTRUM) -------- */
static void
demonstra_eclipticam(const ContextusDemonstrandi *ctx)
{
    int alt = CANVAS_DEFAULT_ALT;
    int lat = CANVAS_DEFAULT_LAT;
    char crater[alt][lat];
    purga_craterem(alt, lat, crater);

    double cx = lat / 2.0;
    double cy = alt / 2.0;
    double radius = (alt < lat ? alt : lat) * 0.42;
    double ratio_axium = 2.1;

    /* Orbis eclipticae */
    for (double theta = 0.0; theta < 2.0 * PI_NUMERUS; theta += 0.03) {
        int x = (int)(cx + ratio_axium * radius * cos(theta) * 0.5);
        int y = (int)(cy + radius * sin(theta));
        pone_punctum(alt, lat, crater, x, y, '.');
    }
    /* Sol in medio */
    pone_punctum(alt, lat, crater, (int)cx, (int)cy, '@');
    /* Septem planetae antiqui: Luna, Mercurius, Venus, Sol, Mars, Iuppiter,
     * Saturnus — hic schematice tantum.  Positio ex anabasis calculatur. */
    const char *siglum = "LMVMIS";
    int nplan = 6;
    for (int i = 0; i < nplan; i++) {
        double phase = (double)ctx->anabasis * 0.15
                     + (double)i * (2.0 * PI_NUMERUS / nplan);
        double rr = radius * (0.25 + 0.13 * i);
        int x = (int)(cx + ratio_axium * rr * cos(phase) * 0.5);
        int y = (int)(cy + rr * sin(phase));
        pone_punctum(alt, lat, crater, x, y, siglum[i]);
    }
    NARRA("    SCHEMA ECLIPTICAE — Sol (@) et septem errantes:\n");
    imprime_craterem(alt, lat, crater);
    NARRA("    L=Luna M=Mercurius V=Venus I=Iuppiter S=Saturnus\n");
}

/* -------- 8. TABULA MONOCHORDI (MUSICA) -------- */
static void
demonstra_monochordum(const ContextusDemonstrandi *ctx)
{
    (void)ctx;
    NARRA("    MONOCHORDI RATIONES CONSONANTIARUM:\n");
    NARRA("    +------------+--------+----------------+----------+\n");
    NARRA("    | NOMEN      | RATIO  | SIGILLUM       | LONG.CHORD|\n");
    NARRA("    +------------+--------+----------------+----------+\n");
    struct {
        const char *nomen;
        int num, den;
    } rationes[] = {
        { "unisonus",    1, 1 },
        { "diapason",    2, 1 },
        { "diapente",    3, 2 },
        { "diatessaron", 4, 3 },
        { "ditonus",     5, 4 },
        { "semiditonus", 6, 5 },
        { "tonus",       9, 8 }
    };
    size_t n = sizeof(rationes) / sizeof(rationes[0]);
    for (size_t i = 0; i < n; i++) {
        double longitudo = 60.0 * rationes[i].den / rationes[i].num;
        NARRA("    | %-10s | %2d:%-2d | ", rationes[i].nomen,
              rationes[i].num, rationes[i].den);
        /* Sigillum visivum */
        int barae = (int)(longitudo / 3.0);
        if (barae > 14) barae = 14;
        for (int b = 0; b < barae; b++) fputc('=', stdout);
        for (int b = barae; b < 14; b++) fputc(' ', stdout);
        NARRA(" | %7.2f  |\n", longitudo);
    }
    NARRA("    +------------+--------+----------------+----------+\n");
    NARRA("    Sic chorda in ratione integrum secta consonantias edit.\n");
}

/* -------- 9. SYLLOGISMUS (PHILOSOPHIA) -------- */
static void
demonstra_syllogismum(const ContextusDemonstrandi *ctx)
{
    (void)ctx;
    /* Syllogismi varii per sortem eliguntur */
    static const struct {
        const char *maior;
        const char *minor;
        const char *conclusio;
    } formae[] = {
        { "Omne quod intellegit aeternum est",
          "Anima intellegit formas",
          "Ergo anima aeternum attingit" },
        { "Omnis numerus divisor unitatis est",
          "Unum non dividitur",
          "Ergo unum supra numerum est" },
        { "Omnis figura proportione constat",
          "Proportio ex ratione procedit",
          "Ergo figura rationem manifestat" },
        { "Omnis motus caelestis regulam sequitur",
          "Regula ex numero constat",
          "Ergo caeli numerus animus est" },
        { "Quidquid est, vel unum vel multa",
          "Multa ex uno procedunt",
          "Ergo unum omnia continet" }
    };
    size_t n = sizeof(formae) / sizeof(formae[0]);
    size_t i = sors_usque((uint32_t)n);
    NARRA("    SYLLOGISMUS DIALECTICUS:\n");
    NARRA("      Maior:     %s.\n", formae[i].maior);
    NARRA("      Minor:     %s.\n", formae[i].minor);
    NARRA("      Conclusio: %s.\n", formae[i].conclusio);
}

/* -------- 10. SOLIDUM PLATONICUM (FORMAE) -------- */
static void
demonstra_solidum_platonicum(const ContextusDemonstrandi *ctx)
{
    /* Enumerat quinque solida et eorum elementum */
    static const struct {
        const char *nomen;
        int facies, apices, latera;
        const char *elementum;
    } solida[] = {
        { "tetraedron",    4,  4,  6, "ignis"  },
        { "cubus",         6,  8, 12, "terra"  },
        { "octaedron",     8,  6, 12, "aer"    },
        { "dodecaedron",  12, 20, 30, "caelum" },
        { "icosaedron",   20, 12, 30, "aqua"   }
    };
    size_t n = sizeof(solida) / sizeof(solida[0]);
    NARRA("    QUINQUE SOLIDA PLATONICA — figurae elementorum:\n");
    NARRA("    +--------------+-------+--------+--------+----------+\n");
    NARRA("    | NOMEN        | FAC.  | APIC.  | LAT.   | ELEMENT. |\n");
    NARRA("    +--------------+-------+--------+--------+----------+\n");
    for (size_t i = 0; i < n; i++) {
        NARRA("    | %-12s | %5d | %6d | %6d | %-8s |\n",
              solida[i].nomen, solida[i].facies, solida[i].apices,
              solida[i].latera, solida[i].elementum);
    }
    NARRA("    +--------------+-------+--------+--------+----------+\n");
    /* Verificamus Eulerem: F - L + A = 2 */
    size_t i = (size_t)ctx->anabasis % n;
    int chi = solida[i].facies - solida[i].latera + solida[i].apices;
    NARRA("    Ecce %s: F - L + A = %d (formula Euleris).\n",
          solida[i].nomen, chi);
}

/* -------- Tabula dispatch -------- */

typedef struct {
    Dominium dom;
    FunctioDemonstrandi functio;
    const char *nomen;
} IngressusDispatch;

static const IngressusDispatch tabula_dispatch[] = {
    { DOM_FIGURA,      demonstra_circulum,            "circulus"    },
    { DOM_FIGURA,      demonstra_triangulum,          "triangulus"  },
    { DOM_FIGURA,      demonstra_parabolam,           "parabola"    },
    { DOM_FIGURA,      demonstra_polygonum,           "polygonum"   },
    { DOM_NUMERUS,     demonstra_divisores,           "divisores"   },
    { DOM_NUMERUS,     demonstra_triplum_pythagoricum,"triplum"     },
    { DOM_ASTRUM,      demonstra_eclipticam,          "ecliptica"   },
    { DOM_MUSICA,      demonstra_monochordum,         "monochordum" },
    { DOM_PHILOSOPHIA, demonstra_syllogismum,         "syllogismus" },
    { DOM_FORMAE,      demonstra_solidum_platonicum,  "solidum"     },
    { DOM_ANIMA,       demonstra_syllogismum,         "syllogismus" },
    { DOM_IGNOTUM,     demonstra_circulum,            "circulus"    }
};
static const size_t n_dispatch =
    sizeof(tabula_dispatch) / sizeof(tabula_dispatch[0]);

/* Invenit functionem demonstrantem pro dominio dato */
static const IngressusDispatch *
selige_demonstratorem(Dominium dom, int turnus)
{
    size_t candidati[16];
    size_t n_cand = 0;
    for (size_t i = 0; i < n_dispatch && n_cand < 16; i++) {
        if (tabula_dispatch[i].dom == dom)
            candidati[n_cand++] = i;
    }
    if (n_cand == 0) {
        /* Cum dominium incognitum, variamus per turnum */
        return &tabula_dispatch[(size_t)turnus % n_dispatch];
    }
    return &tabula_dispatch[candidati[sors_usque((uint32_t)n_cand)]];
}

/* ========================================================================
 * ASCENSUS ANAGOGICUS — GRADUS METAPHYSICI
 * ======================================================================== */

/*
 * Per gradus anabasis mens ascendit: ex sensibilibus ad mathemata, inde
 * ad animam, ad nuntium (nous), denique ad unum (hen).  Unusquisque gradus
 * suam vocem habet.
 */

static const char *gradus_anabasis[ANABASIS_GRADUS] = {
    "sensibilibus",     /* 0 */
    "mathematicis",     /* 1 */
    "anima",            /* 2 */
    "nuntio",           /* 3 */
    "uno"               /* 4 */
};

static const char *finis_anabasis[ANABASIS_GRADUS] = {
    "Ex his corporeis figuris mens ad ordinem sensibilem advertitur.",
    "Figurae numerique ad mathematicam regionem animum praeparant.",
    "Anima rationalis se in his formis recognoscit, sui memor facta.",
    "Mens ipsa, nous divinus, ad se conversa, principia contemplatur.",
    "Omnia demum ad Unum redeunt, fontem et terminum essentiae."
};

/* Calculat gradum anabasis ex numero turnus */
static int
computa_gradum(int turnus)
{
    int g = turnus / 3;
    if (g >= ANABASIS_GRADUS) g = ANABASIS_GRADUS - 1;
    if (g < 0) g = 0;
    return g;
}

/* Pronuntiat anagogen congruentem gradui */
static void
pronuntia_anagogen(int turnus, Dominium dom)
{
    int g = computa_gradum(turnus);
    NARRA("    [Anagoge — gradus %d: ex %s]\n", g, gradus_anabasis[g]);
    /* Sententia prima: ad dominium refertur */
    switch (dom) {
    case DOM_NUMERUS:
        NARRA("    Numeri ipsi non sunt res, sed similitudines rerum divinarum;\n");
        NARRA("    per eos mens ad proportionem aeternam adhaeret.\n");
        break;
    case DOM_FIGURA:
        NARRA("    Figura visibilis formae invisibilis vestigium est;\n");
        NARRA("    geometra non chartam, sed ideam inspicit.\n");
        break;
    case DOM_ASTRUM:
        NARRA("    Astra ordine suo animum ad caelestia convertunt;\n");
        NARRA("    motus eorum numeris nostri intellectus cognatis paret.\n");
        break;
    case DOM_MUSICA:
        NARRA("    Musica sensibilis harmoniae mundanae echo est;\n");
        NARRA("    consonantiae numericae unum in multis ostendunt.\n");
        break;
    case DOM_PHILOSOPHIA:
        NARRA("    Dialectica via est qua ex opinione ad scientiam transitur,\n");
        NARRA("    et ex scientia ad ipsius veritatis contemplationem.\n");
        break;
    case DOM_ANIMA:
        NARRA("    Anima in se recurrens agnoscit se esse imago intelligentiae,\n");
        NARRA("    quae ipsa imago est unius principii.\n");
        break;
    case DOM_FORMAE:
        NARRA("    Formae non sunt ultima; ipsae ab uno pendent,\n");
        NARRA("    quod ultra essentiam ineffabile manet.\n");
        break;
    default:
        NARRA("    Quidquid videtur, signum est eorum quae non videntur;\n");
        NARRA("    signis intellectis, ad signata ascende.\n");
        break;
    }
    /* Finis anabasis secundum gradum */
    NARRA("    %s\n", finis_anabasis[g]);
}

/* ========================================================================
 * SALUTATIONES
 * ======================================================================== */

static const char *salutationes_hypatiae[] = {
    "Salve, discipule, in atrio sapientiae.",
    "Adest iterum quaestio digna; audi et considera.",
    "Mens tua ad veritatem tendit — gaudeo.",
    "Inspice mecum quae subsunt phaenomenis.",
    "O mi discipule, aperi animum ad haec mathemata.",
    "Num rursus ad me confugis? Bene: interrogandum est.",
    "Ad figuras converte oculos, ad numeros mentem.",
    "Pulchra est via quae a sensibus ad formas ducit."
};
static const size_t n_salutationum =
    sizeof(salutationes_hypatiae) / sizeof(salutationes_hypatiae[0]);

static const char *salutationes_discipuli[] = {
    "Magistra, edoce me de his rebus.",
    "Hypatia, quaero rationem quam heri attigisti.",
    "Doctrix venerabilis, audi interrogantem.",
    "Magistra Alexandriae, apud te sedeo.",
    "Quaerere libet, magistra: responde si placet.",
    "Cum tuam doctrinam sequor, omnia clariora fiunt.",
    "Hypatia, quid de hoc mihi docebis?",
    "Si dignus sum, edissera mihi hanc quaestionem."
};
static const size_t n_sal_discipuli =
    sizeof(salutationes_discipuli) / sizeof(salutationes_discipuli[0]);

static const char *
sors_salutationis_hyp(void)
{
    return salutationes_hypatiae[sors_usque((uint32_t)n_salutationum)];
}

static const char *
sors_salutationis_disc(void)
{
    return salutationes_discipuli[sors_usque((uint32_t)n_sal_discipuli)];
}

/* ========================================================================
 * RESPONSUM HYPATIAE — COMPOSITIO PARTIUM
 * ======================================================================== */

static void
respondet_hypatia(const char *ingressus, int turnus)
{
    Dominium dom = classifica_textum(ingressus);
    if (dom == DOM_IGNOTUM) {
        /* Si nihil clarum inventum, dominium per sortem eligimus */
        dom = (Dominium)sors_usque(DOM_IGNOTUM);
    }

    const Propositio *prop = selige_propositionem(dom);
    const IngressusDispatch *disp = selige_demonstratorem(dom, turnus);

    ContextusDemonstrandi ctx = {
        .anabasis = turnus,
        .turnus = turnus,
        .dom = dom,
        .nomen = disp->nomen,
        .flagra = 0,
        .reservatum = 0
    };

    NARRA("\n--- HYPATIA respondet [turnus %d, dominium %s] ---\n",
          turnus + 1, nomen_dominii[dom]);
    NARRA("  SALUTATIO: %s\n", sors_salutationis_hyp());
    NARRA("  PROPOSITIO: %s\n", prop->textus);
    NARRA("  DEMONSTRATIO (%s):\n", disp->nomen);
    disp->functio(&ctx);
    NARRA("  ANAGOGE:\n");
    pronuntia_anagogen(turnus, dom);
    NARRA("\n");
}

/* ========================================================================
 * COLLOQUIUM SCRIPTUM — HYPATIA ET SYNESIUS
 * ======================================================================== */

/*
 * Colloquium praescriptum quattuordecim turnorum saltem.  Discipulus
 * (saepe Synesius Cyrenaeus) interrogat; Hypatia respondet ex omnibus
 * septem dominiis, conicos, numeros, astra, harmoniam, formas exponens.
 */

static const char *interrogationes_synesii[] = {
    "Magistra, quid est numerus et cur principium rerum dicitur?",
    "Expone mihi quae sit natura circuli et unde perfectio eius.",
    "Explica, quaeso, triplum pythagoricum et hypotenusae rationem.",
    "Quid de parabola docent Apollonii Pergaei libri?",
    "Quomodo chorda monochordi consonantias gignit?",
    "Quae ratio inter sectiones conicas et caeli orbes?",
    "Quae sunt quinque solida platonica et quibus elementis congruunt?",
    "Doce me de ecliptica et planetarum errantium via.",
    "Quid de syllogismo et de via dialectica sentis?",
    "Polygona regularia quomodo intra circulum describuntur?",
    "Doce iterum de animae ascensu ad intellectum.",
    "Forma an numerus prior est in ordine rerum?",
    "Quid tandem unum, supra essentiam praedicandum?",
    "Recapitula, Hypatia, iter nostrum per mathemata et formas."
};
static const size_t n_inter =
    sizeof(interrogationes_synesii) / sizeof(interrogationes_synesii[0]);

static void
profer_colloquium_scriptum(void)
{
    NARRA("=======================================================\n");
    NARRA(" COLLOQUIUM: HYPATIA ALEXANDRINA ET SYNESIUS CYRENAEUS\n");
    NARRA("=======================================================\n");
    NARRA(" Mathesis neoplatonica — dialectica et demonstratio.\n");
    NARRA(" Versio automati: %s.  Semen: 0x%016llX.\n",
          VERSIO_AUTOMATI, (unsigned long long)fortuna.integer);
    NARRA("=======================================================\n\n");

    int turni = COLLOQUIUM_TURNOS > (int)n_inter ? (int)n_inter : COLLOQUIUM_TURNOS;
    if (turni < 14) turni = 14; /* Minimum quattuordecim */

    for (int t = 0; t < turni; t++) {
        const char *interrog =
            interrogationes_synesii[(size_t)t % n_inter];
        NARRA(">>> SYNESIUS: %s %s\n", sors_salutationis_disc(), interrog);
        respondet_hypatia(interrog, t);
    }

    NARRA("=======================================================\n");
    NARRA(" FINIS COLLOQUII. Manet anima ad unum conversa.\n");
    NARRA("=======================================================\n");
}

/* ========================================================================
 * MODUS INTERACTIVUS
 * ======================================================================== */

static void
tolle_lineam(char *restrict buffer, size_t cap)
{
    size_t lon = strlen(buffer);
    if (lon > 0 && buffer[lon - 1] == '\n') buffer[lon - 1] = '\0';
    if (lon > 1 && buffer[lon - 2] == '\r') buffer[lon - 2] = '\0';
    (void)cap;
}

static void
profer_colloquium_interactivum(void)
{
    NARRA("=======================================================\n");
    NARRA(" HYPATIA ALEXANDRINA — modus interactivus.\n");
    NARRA(" Scribe interrogationem latinam.\n");
    NARRA(" Exi per 'vale' aut per EOF.\n");
    NARRA("=======================================================\n\n");

    char linea[LIMES_LINEAE];
    int turnus = 0;
    NARRA(">>> DISCIPULE: ");
    fflush(stdout);
    while (fgets(linea, sizeof(linea), stdin) != NULL) {
        tolle_lineam(linea, sizeof(linea));
        if (linea[0] == '\0') {
            NARRA(">>> DISCIPULE: ");
            fflush(stdout);
            continue;
        }
        char copia[LIMES_LINEAE];
        strncpy(copia, linea, sizeof(copia) - 1);
        copia[sizeof(copia) - 1] = '\0';
        ad_minusculas(copia);
        if (strstr(copia, "vale") != NULL ||
            strstr(copia, "exit") != NULL ||
            strstr(copia, "quit") != NULL) {
            NARRA("\nHYPATIA: Vale, discipule. Mens tua semper ad unum tendat.\n");
            break;
        }
        respondet_hypatia(linea, turnus);
        turnus++;
        NARRA(">>> DISCIPULE: ");
        fflush(stdout);
    }
    if (feof(stdin)) {
        NARRA("\nHYPATIA: Silentio absolvitur colloquium; manet sapientia.\n");
    }
}

/* ========================================================================
 * ARGUMENTA LINEAE IMPERII
 * ======================================================================== */

typedef struct {
    int modus_interactivus;
    int semen_datum;
    uint64_t semen;
} ArgumentaCli;

static void
monstra_usum(const char *nomen_programmatis)
{
    ERRARE("Usus: %s [-i] [-s N]\n", nomen_programmatis);
    ERRARE("  (nullum)   colloquium automaticum Hypatiae et discipuli\n");
    ERRARE("  -i         modus interactivus per tty\n");
    ERRARE("  -s N       semen deterministicum (numerus)\n");
    ERRARE("Automaton loquens Hypatiae Alexandrinae.\n");
}

static int
lege_argumenta(int argc, char **argv, ArgumentaCli *args)
{
    args->modus_interactivus = 0;
    args->semen_datum = 0;
    args->semen = 0;
    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        if (strcmp(a, "-i") == 0) {
            args->modus_interactivus = 1;
        } else if (strcmp(a, "-s") == 0) {
            if (i + 1 >= argc) {
                ERRARE("Semen post -s requiritur.\n");
                return -1;
            }
            args->semen = (uint64_t)strtoull(argv[++i], NULL, 0);
            args->semen_datum = 1;
        } else if (strcmp(a, "-h") == 0 || strcmp(a, "--help") == 0) {
            monstra_usum(argv[0]);
            return 1;
        } else {
            ERRARE("Argumentum ignotum: %s\n", a);
            return -1;
        }
    }
    return 0;
}

/* ========================================================================
 * FUNCTIO PRINCIPALIS
 * ======================================================================== */

int
main(int argc, char **argv)
{
    ArgumentaCli args;
    int rv = lege_argumenta(argc, argv, &args);
    if (rv < 0) {
        monstra_usum(argv[0]);
        return 2;
    }
    if (rv > 0) {
        return 0; /* -h/--help */
    }

    uint64_t semen = args.semen_datum
        ? args.semen
        : 0x1D2B3F4C5A6E7081ULL;
    semina_fortunam(semen);

    if (args.modus_interactivus) {
        profer_colloquium_interactivum();
    } else {
        profer_colloquium_scriptum();
    }
    return 0;
}

/* ========================================================================
 * APPENDIX: NOTAE DE STRUCTURA INTERNA
 * ========================================================================
 *
 * Hypatia Alexandrina (ca. 355-415 p.Chr.n.), filia Theonis mathematici,
 * docuit apud museum Alexandriae philosophiam Platonicam atque Plotinianam,
 * commentata est in Apollonii Conica, in Diophanti Arithmetica, in tabulas
 * astronomicas Ptolemaei.  Huc automaton eius dialecticam aemulatur, non
 * sententias ad litteram reddens, sed modum docendi: quaestioni respondere
 * per propositionem, demonstrationem visibilem, et anagogen.
 *
 * Structura programmatis:
 *   - Classificator dominii per verba clavia.
 *   - X-macro corpus propositionum tagged per dominium.
 *   - Tabula dispatch functionum demonstrantium, eligenda per dominium
 *     et sortem pseudoaleatoriam deterministicam.
 *   - Craterae ASCII implementati per VLA (arrays longitudinis variabilis
 *     C99).
 *   - Anagoge per anabasis-gradus incrementatos per turnum.
 *
 * Neoplatonici gradus: hyle (materia) -> soma (corpus) -> psyche (anima)
 *   -> nous (intellectus) -> hen (unum).  In hoc automato minoris numeri
 *   incipimus: a sensibilibus per mathematica ad animam, mentem, unum.
 *
 * Singulae sectiones coni ab Apollonio Pergaeo tradita: ellipsis
 * (sectio plani obliqui quod utrumque coni latus secat), parabola (sectio
 * parallela lateri), hyperbole (sectio utramque partem coni secans).
 *
 * Monochordum: instrumentum unicorde quo Pythagoras consonantias in
 * rationibus numericis ostendit — diapason 2:1, diapente 3:2, diatessaron
 * 4:3.  Chorda dimidiata octavam gignit; duabus tertiis, quintam.
 *
 * Syllogismus aristotelicus, quamvis Hypatia Platonica fuerit, ad
 * formandum animum discipulorum adhibetur — dialectica neoplatonica
 * syllogismos non reicit sed tamquam gradum utitur.
 *
 * Quinque solida platonica: tetraedron (ignis, quia acutissimum),
 * cubus (terra, stabilissimus), octaedron (aer), icosaedron (aqua),
 * dodecaedron (caelum, quia stellarum figuris propinquum).
 *
 * In Timaeo (55-56) Plato corpora elementis assignat; Theaetetus
 * (ante Platonem) demonstravit quinque tantum posse esse.
 *
 * Ecliptica zodiaci: orbita apparens solis per duodecim signa.
 * Planetae antiqui septem: Luna, Mercurius, Venus, Sol, Mars, Iuppiter,
 * Saturnus — schemata nostra propter spatium sex tantum exhibent,
 * cum Sole in medio posito.
 *
 * Numerus perfectus: cuius divisorum propriorum summa ipsi numero par est.
 * Sex est primus: 1 + 2 + 3 = 6.  Vicesimus octavus secundus: 1 + 2 + 4 +
 * 7 + 14 = 28.  Euclides in Elementis IX.36 regulam dedit: si 2^n - 1
 * primus est, tum (2^(n-1))(2^n - 1) perfectus est.
 *
 * Triplum pythagoricum: tres numeri integri (a, b, c) tales ut
 * a^2 + b^2 = c^2.  Primitivi sunt si inter se primi: (3,4,5), (5,12,13),
 * (8,15,17), (7,24,25).  Ex omni pari numerorum (m, n) cum m > n,
 * a = m^2 - n^2, b = 2mn, c = m^2 + n^2 generantur.
 *
 * Ratio aurea phi = (1 + sqrt(5))/2 ~ 1.618 — divisio lineae ita ut
 * tota ad maiorem sicut maior ad minorem pars se habeat.
 *
 * Constantes geometricae quibus hic utimur: pi (circuli perimetri ad
 * diametrum ratio), phi (ratio aurea), radix 2 (diagonalis quadrati
 * unius lateris).  Omnes Pythagoricis ac Platonicis notissimae.
 *
 * Commentarium in confectione programmatis:
 *   - Omnes identificatores ad verba latina referuntur: "fortuna" pro
 *     "random state", "sors_usque" pro "random in range", "crater" pro
 *     "canvas".  Computare modo scholastico latinum servatur.
 *   - Macros ALL-CAPS: NARRA, ERRARE, PI_NUMERUS, LIMES_LINEAE, etc.
 *   - Bitfields in StatusFortunae et ContextusDemonstrandi.
 *   - Unio ad statum fortunae.
 *   - Designated initializers in ArgumentaCli et compound literals
 *     implicitis in tabulis staticis.
 *   - VLA in functionibus demonstrantibus figuras.
 *   - Function-pointer dispatch per tabulam_dispatch.
 *   - X-macro corpus propositionum.
 *   - Variadic macro NARRA/ERRARE.
 *   - Inline functiones misce_xorshift, sors_usque, sors_fractio,
 *     pone_punctum.
 *   - Restrict qualifier in parametris bufferorum.
 *
 * Sic completus est automaton.  Vale, lector, et ad unum ascende.
 *
 * ========================================================================
 * APPENDIX SECUNDUS: DE RATIONE XORSHIFT ET DE DETERMINISMO
 * ========================================================================
 *
 * Generator xorshift64 a Marsaglia proposuit (2003).  Cum sit purae
 * aritmeticae entium sine statu externo, perfecte determisticus manet:
 * idem semen idem ordinem producit.  Hoc necessarium est ad reproducibi-
 * litatem testimoniorum et ad stabilitatem colloquiorum.
 *
 * Propriorum testimonium: xorshift64 periodum habet 2^64 - 1, qui pro
 * hac automati adhibitione longe sufficit — colloquium enim quattuor-
 * decim turnorum vix paucos bytes sortium consumit.
 *
 * Formula xorshift canonica (a, b, c) = (13, 7, 17) ad bona signa
 * statistica dirigit.  Variantes (12, 25, 27) etc. aliae sunt, sed
 * nostra utitur prima.
 *
 * ========================================================================
 * APPENDIX TERTIUS: DE NEOPLATONISMO ALEXANDRINO
 * ========================================================================
 *
 * Neoplatonismus Alexandrinus, cuius Hypatia columna fuit, ex Plotino
 * (205-270 p.Chr.n.) per Porphyrium, Iamblichum, Syrianum, Proclum,
 * descendit.  Tres principales hypostases: Unum (hen), Nous (intellectus),
 * Psyche (anima).  Omnia ex Uno procedunt (prohodos), ad Unum reverti
 * debent (epistrophe).  Mathematicae disciplinae gradus mediae sunt
 * inter sensibilia et intelligibilia — inde Platonis dictum quod
 * "geometria ad mentem docet" (Rep. VII).
 *
 * Hypatia, teste Socrate Scholastico (Hist. Eccl. VII.15), "mathematicam
 * et philosophiam docuit tantam quantam Plotinus ipse"; Damaskios eam
 * "philosophiam circumibat" scripsit.  Synesius Cyrenaeus, discipulus eius
 * et postea episcopus Ptolemaidis, epistulas ad eam missit quibus amor
 * et observantia intelleguntur.  Mortem eius (415 p.Chr.n.) hodie homines
 * memoriae mandamus tamquam cladem libertatis scholae.
 *
 * Huic automato honor in animum revocandae Hypatiae datur, non in
 * tragoediam recasurae, sed in doctrinae eius vim renovandam.
 *
 * VALE.
 */
