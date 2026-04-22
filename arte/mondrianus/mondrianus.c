/*
 * mondrianus.c — Compositio Neoplastica (Piet Mondrian)
 *
 * Generat imaginem 256x256 in stilo "De Stijl": divisio recursiva
 * plani in rectangula per lineas horizontales et verticales,
 * ubi rectangula eliguntur ad colorem primarium (rubrum, caeruleum,
 * flavum) aut album/cineritium accipiendum. Lineae atrae fortes
 * separant regiones.
 *
 * PRNG: xorshift64*, semine -s mutabili, defaltum constans.
 *
 * Invocatio:
 *   mondrianus            -> reddit ANSI 24-bit in stdout
 *   mondrianus -o f.ppm   -> scribit P6 PPM in f.ppm
 *   mondrianus -s N       -> semen generatoris mutat
 *   mondrianus -h         -> auxilium
 *
 * Compilatio: cc -std=c99 -Wall -Wextra -pedantic mondrianus.c -o mondrianus
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define LATITUDO 256
#define ALTITUDO 256
#define SEMEN_DEFALTUM 0xDE57194LLU
#define PIXELLI (LATITUDO * ALTITUDO)
#define MAX_RECT 512
#define CRASSITUDO_LINEAE 4
#define PROFUNDITAS_MAX 6
#define MIN_LATUS 24

typedef struct {
    unsigned char r, g, b;
} Color;

typedef struct {
    int x0, y0, x1, y1;
    Color color;
} Rectangulum;

static uint64_t state_prng = SEMEN_DEFALTUM;

static uint64_t xorshift64(void) {
    uint64_t x = state_prng;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    state_prng = x;
    return x * 2685821657736338717ULL;
}

static int rand_range(int lo, int hi) {
    if (hi <= lo) return lo;
    return lo + (int)(xorshift64() % (uint64_t)(hi - lo));
}

static double rand_unit(void) {
    return (double)(xorshift64() >> 11) / (double)(1ULL << 53);
}

/* Paletta Mondriana: album dominans, rubrum, caeruleum, flavum,
 * parvum cineritium, rarissime atrum. */
static const Color PALETTA[] = {
    {245, 240, 230},   /* album osseum */
    {245, 240, 230},
    {245, 240, 230},
    {245, 240, 230},
    {245, 240, 230},
    {200, 30, 30},     /* rubrum */
    {30, 55, 170},     /* caeruleum */
    {240, 200, 35},    /* flavum */
    {220, 215, 205},   /* cinericium lene */
    {25, 25, 25},      /* atrum rarum */
};
#define N_PALETTA ((int)(sizeof(PALETTA)/sizeof(PALETTA[0])))

static Color eligere_colorem(int profunditas) {
    /* Minora rectangula saepius colorata; maxima saepe alba. */
    if (profunditas < 2) {
        /* Primae divisiones albae sunt. */
        return PALETTA[0];
    }
    return PALETTA[rand_range(0, N_PALETTA)];
}

/* =============================================================
 * DIVISIO RECURSIVA
 * ============================================================= */

static Rectangulum rectangula[MAX_RECT];
static int n_rect = 0;

static void dividere(int x0, int y0, int x1, int y1, int profunditas) {
    int latitudo = x1 - x0;
    int altitudo = y1 - y0;

    /* Si parvum, aut profunde divisum, accipere colorem. */
    int terminare = 0;
    if (profunditas >= PROFUNDITAS_MAX) terminare = 1;
    else if (latitudo < MIN_LATUS * 2 && altitudo < MIN_LATUS * 2) terminare = 1;
    else if (rand_unit() < 0.18 + profunditas * 0.08) terminare = 1;

    if (terminare) {
        if (n_rect < MAX_RECT) {
            Rectangulum r = { x0, y0, x1, y1, eligere_colorem(profunditas) };
            rectangula[n_rect++] = r;
        }
        return;
    }

    /* Eligere axem divisionis: praeferentia ad maiorem dimensionem. */
    int horizontaliter;
    if (latitudo > altitudo * 3 / 2) horizontaliter = 0;     /* sectio verticalis */
    else if (altitudo > latitudo * 3 / 2) horizontaliter = 1;
    else horizontaliter = (xorshift64() & 1) ? 1 : 0;

    if (horizontaliter) {
        int lo = y0 + MIN_LATUS;
        int hi = y1 - MIN_LATUS;
        if (hi <= lo) {
            if (n_rect < MAX_RECT) {
                Rectangulum r = { x0, y0, x1, y1, eligere_colorem(profunditas) };
                rectangula[n_rect++] = r;
            }
            return;
        }
        int cut = rand_range(lo, hi);
        dividere(x0, y0, x1, cut, profunditas + 1);
        dividere(x0, cut, x1, y1, profunditas + 1);
    } else {
        int lo = x0 + MIN_LATUS;
        int hi = x1 - MIN_LATUS;
        if (hi <= lo) {
            if (n_rect < MAX_RECT) {
                Rectangulum r = { x0, y0, x1, y1, eligere_colorem(profunditas) };
                rectangula[n_rect++] = r;
            }
            return;
        }
        int cut = rand_range(lo, hi);
        dividere(x0, y0, cut, y1, profunditas + 1);
        dividere(cut, y0, x1, y1, profunditas + 1);
    }
}

/* =============================================================
 * PICTURA
 * ============================================================= */

static unsigned char pixellorum[PIXELLI * 3];

static void ponere_pixel(int x, int y, Color c) {
    if (x < 0 || x >= LATITUDO || y < 0 || y >= ALTITUDO) return;
    size_t idx = (size_t)(y * LATITUDO + x) * 3;
    pixellorum[idx + 0] = c.r;
    pixellorum[idx + 1] = c.g;
    pixellorum[idx + 2] = c.b;
}

static void implere_rectangulum(int x0, int y0, int x1, int y1, Color c) {
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > LATITUDO) x1 = LATITUDO;
    if (y1 > ALTITUDO) y1 = ALTITUDO;
    for (int y = y0; y < y1; y++) {
        for (int x = x0; x < x1; x++) {
            ponere_pixel(x, y, c);
        }
    }
}

static void depingere(void) {
    /* Fundum album. */
    Color album = {245, 240, 230};
    Color atrum = {20, 20, 20};
    implere_rectangulum(0, 0, LATITUDO, ALTITUDO, album);

    /* Implere rectangula. */
    for (int i = 0; i < n_rect; i++) {
        Rectangulum *r = &rectangula[i];
        implere_rectangulum(r->x0, r->y0, r->x1, r->y1, r->color);
    }

    /* Depingere lineas atras inter rectangula et ad marginem. */
    for (int i = 0; i < n_rect; i++) {
        Rectangulum *r = &rectangula[i];
        /* Linea superior */
        implere_rectangulum(r->x0, r->y0, r->x1, r->y0 + CRASSITUDO_LINEAE, atrum);
        /* Linea inferior */
        implere_rectangulum(r->x0, r->y1 - CRASSITUDO_LINEAE, r->x1, r->y1, atrum);
        /* Sinistra */
        implere_rectangulum(r->x0, r->y0, r->x0 + CRASSITUDO_LINEAE, r->y1, atrum);
        /* Dextra */
        implere_rectangulum(r->x1 - CRASSITUDO_LINEAE, r->y0, r->x1, r->y1, atrum);
    }
}

/* =============================================================
 * SCRIPTURA PPM
 * ============================================================= */

static int scribere_ppm(const char *nomen_fil) {
    FILE *f = fopen(nomen_fil, "wb");
    if (!f) {
        fprintf(stderr, "mondrianus: nequeo aperire '%s'\n", nomen_fil);
        return 1;
    }
    fprintf(f, "P6\n%d %d\n255\n", LATITUDO, ALTITUDO);
    size_t n = fwrite(pixellorum, 1, (size_t)PIXELLI * 3, f);
    fclose(f);
    if (n != (size_t)PIXELLI * 3) {
        fprintf(stderr, "mondrianus: error scripturae\n");
        return 1;
    }
    return 0;
}

/* =============================================================
 * REDDITIO ANSI
 * ============================================================= */

/* Reddit cum 24-bit ANSI colore in duobus semiquadris per cellam
 * (▀ = U+2580). Descendit imaginem per factorem 4: 64 columnae x
 * 32 lineae (64 x 64 pixelli post duplicationem verticalem). */

static void pixel_medius(int px, int py, int width, Color *out) {
    /* 4x4 pixellorum medius. */
    int sx = px * 4, sy = py * 4;
    unsigned long r = 0, g = 0, bl = 0;
    int count = 0;
    for (int dy = 0; dy < 4; dy++) {
        for (int dx = 0; dx < 4; dx++) {
            int x = sx + dx;
            int y = sy + dy;
            if (x >= width || y >= ALTITUDO) continue;
            size_t idx = (size_t)(y * LATITUDO + x) * 3;
            r += pixellorum[idx + 0];
            g += pixellorum[idx + 1];
            bl += pixellorum[idx + 2];
            count++;
        }
    }
    if (count == 0) count = 1;
    out->r = (unsigned char)(r / (unsigned long)count);
    out->g = (unsigned char)(g / (unsigned long)count);
    out->b = (unsigned char)(bl / (unsigned long)count);
}

static int rgb_ad_256(unsigned long r, unsigned long g, unsigned long b) {
    unsigned long mx = r>g?(r>b?r:b):(g>b?g:b), mn = r<g?(r<b?r:b):(g<b?g:b);
    if (mx - mn < 10) {
        unsigned long gr = (r+g+b)/3;
        if (gr < 8) return 16;
        if (gr > 248) return 231;
        return 232 + (int)((gr - 8) * 24 / 240);
    }
    return 16 + 36*(int)(r*5/255) + 6*(int)(g*5/255) + (int)(b*5/255);
}

static void reddere_ansi(void) {
    int cols = 64;
    int rows = 32;
    for (int cy = 0; cy < rows; cy++) {
        for (int cx = 0; cx < cols; cx++) {
            Color top, bot;
            /* Cella unusquisque = 2 half-rows, quaeque 4 pixelli verticales. */
            /* Simplificamus: sampla 4x4 sed duobus: superius et inferius. */
            int sx = cx * 4;
            int sy_top = cy * 8;
            int sy_bot = cy * 8 + 4;
            unsigned long tr=0,tg=0,tb=0,br=0,bg=0,bb=0;
            int ct=0, cb=0;
            for (int dy = 0; dy < 4; dy++) {
                for (int dx = 0; dx < 4; dx++) {
                    int x = sx + dx;
                    int yt = sy_top + dy;
                    int yb = sy_bot + dy;
                    if (x < LATITUDO && yt < ALTITUDO) {
                        size_t idx = (size_t)(yt * LATITUDO + x) * 3;
                        tr += pixellorum[idx]; tg += pixellorum[idx+1]; tb += pixellorum[idx+2];
                        ct++;
                    }
                    if (x < LATITUDO && yb < ALTITUDO) {
                        size_t idx = (size_t)(yb * LATITUDO + x) * 3;
                        br += pixellorum[idx]; bg += pixellorum[idx+1]; bb += pixellorum[idx+2];
                        cb++;
                    }
                }
            }
            if (ct==0) ct=1; if (cb==0) cb=1;
            top.r = (unsigned char)(tr/(unsigned long)ct);
            top.g = (unsigned char)(tg/(unsigned long)ct);
            top.b = (unsigned char)(tb/(unsigned long)ct);
            bot.r = (unsigned char)(br/(unsigned long)cb);
            bot.g = (unsigned char)(bg/(unsigned long)cb);
            bot.b = (unsigned char)(bb/(unsigned long)cb);
            printf("\x1b[38;5;%dm\x1b[48;5;%dm\xe2\x96\x80",
                   rgb_ad_256(top.r, top.g, top.b),
                   rgb_ad_256(bot.r, bot.g, bot.b));
            (void)pixel_medius;
        }
        printf("\x1b[0m\n");
    }
}

/* =============================================================
 * AUXILIUM ET ARGUMENTA
 * ============================================================= */

static void auxilium(const char *nomen) {
    printf("Usus: %s [-h] [-o imago.ppm] [-s semen]\n", nomen);
    printf("\n");
    printf("Generat imaginem 256x256 in stilo compositionis\n");
    printf("neoplasticae Petri Mondriani.\n");
    printf("\n");
    printf("  -h            hoc auxilium ostendere\n");
    printf("  -o f.ppm      imaginem PPM in 'f.ppm' scribere\n");
    printf("  -s N          semen generatoris fortuiti (numerus integer)\n");
    printf("\n");
    printf("Sine argumentis, reddit in terminali per codices ANSI.\n");
}

int main(int argc, char **argv) {
    const char *nomen_fil = NULL;
    uint64_t semen = SEMEN_DEFALTUM;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            auxilium(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "mondrianus: -o requirit argumentum\n");
                return 2;
            }
            nomen_fil = argv[++i];
        } else if (strcmp(argv[i], "-s") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "mondrianus: -s requirit argumentum\n");
                return 2;
            }
            semen = strtoull(argv[++i], NULL, 0);
            if (semen == 0) semen = 1;
        } else {
            fprintf(stderr, "mondrianus: ignotum argumentum '%s'\n", argv[i]);
            fprintf(stderr, "pro auxilio: %s -h\n", argv[0]);
            return 2;
        }
    }

    state_prng = semen;

    /* Divisio recursiva. */
    dividere(0, 0, LATITUDO, ALTITUDO, 0);
    depingere();

    if (nomen_fil) {
        return scribere_ppm(nomen_fil);
    } else {
        reddere_ansi();
    }
    return 0;
}
