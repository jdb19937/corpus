/*
 * escherus.c — Tessellatio Cubicorum (Maurits Cornelis Escher)
 *
 * Generat imaginem 256x256 in stilo Escherii: tessellatio isometrica
 * rhomborum (cubi in visu perspectivo axonometrico). Tres facies
 * rhombicae (superior, sinistra, dextra) in tribus gradibus lucis
 * pictae, ita ut structura cubica emergat. Lineae atrae limites
 * inter facies monstrant.
 *
 * ASCII reddit in stdout cum gradibus lucis `" .:-=+*#%@"`.
 * PPM reddit cum -o.
 *
 * Compilatio: cc -std=c99 -Wall -Wextra -pedantic escherus.c -o escherus -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define LAT 256
#define ALT 256
#define PIX (LAT * ALT)
#define SEMEN_DEFALTUM 0xE5CEF0CA17298ULL

typedef struct { unsigned char r, g, b; } Color;

static unsigned char img[PIX * 3];
static uint64_t prng_state = SEMEN_DEFALTUM;

static uint64_t xs64(void) {
    uint64_t x = prng_state;
    x ^= x << 13; x ^= x >> 7; x ^= x << 17;
    prng_state = x;
    return x;
}
static double rnd_unit(void) { return (double)(xs64() >> 11) / (double)(1ULL << 53); }

static void setpx(int x, int y, Color c) {
    if (x < 0 || x >= LAT || y < 0 || y >= ALT) return;
    size_t i = (size_t)(y * LAT + x) * 3;
    img[i] = c.r; img[i+1] = c.g; img[i+2] = c.b;
}

static Color getpx(int x, int y) {
    Color c = {0,0,0};
    if (x < 0 || x >= LAT || y < 0 || y >= ALT) return c;
    size_t i = (size_t)(y * LAT + x) * 3;
    c.r = img[i]; c.g = img[i+1]; c.b = img[i+2];
    return c;
}

static void clear_img(Color c) {
    for (int y = 0; y < ALT; y++)
        for (int x = 0; x < LAT; x++) setpx(x, y, c);
}

/* ============================================================
 * GEOMETRIA: RHOMBUS ISOMETRICUS
 *
 * Rhombus habet 4 vertices. Impletio per scanline: iterare per y,
 * pro quaque linea invenire x limites rhombi, et ponere colorem.
 * ============================================================ */

typedef struct { double x, y; } Punctum;

/* Implere polygon convexum per scanline. Accipit array punctorum
 * ordine ciclico. */
static void impl_polygon(Punctum *pts, int n, Color c) {
    double ymin = pts[0].y, ymax = pts[0].y;
    for (int i = 1; i < n; i++) {
        if (pts[i].y < ymin) ymin = pts[i].y;
        if (pts[i].y > ymax) ymax = pts[i].y;
    }
    int iy0 = (int)floor(ymin);
    int iy1 = (int)ceil(ymax);
    if (iy0 < 0) iy0 = 0;
    if (iy1 > ALT) iy1 = ALT;
    for (int y = iy0; y < iy1; y++) {
        double yc = y + 0.5;
        double xs[16]; int nx = 0;
        for (int i = 0; i < n; i++) {
            Punctum a = pts[i], b = pts[(i+1) % n];
            if ((a.y <= yc && b.y > yc) || (b.y <= yc && a.y > yc)) {
                double t = (yc - a.y) / (b.y - a.y);
                if (nx < 16) xs[nx++] = a.x + t * (b.x - a.x);
            }
        }
        /* ordina */
        for (int i = 1; i < nx; i++) {
            double v = xs[i]; int j = i - 1;
            while (j >= 0 && xs[j] > v) { xs[j+1] = xs[j]; j--; }
            xs[j+1] = v;
        }
        for (int i = 0; i + 1 < nx; i += 2) {
            int x0 = (int)floor(xs[i]);
            int x1 = (int)ceil(xs[i+1]);
            if (x0 < 0) x0 = 0;
            if (x1 > LAT) x1 = LAT;
            for (int x = x0; x < x1; x++) setpx(x, y, c);
        }
    }
}

/* Convertere coordinata cubica (i, j, k) integra in punctum 2D.
 * Axes isometrici: e1 = (cos30, sin30), e2 = (-cos30, sin30),
 * e3 = (0, -1). Centrum imaginis = (128, 128). */
static Punctum iso_ad_2D(double i, double j, double k, double scala) {
    double c30 = 0.8660254037844387;
    double s30 = 0.5;
    Punctum p;
    p.x = 128.0 + scala * (i * c30 - j * c30);
    p.y = 128.0 + scala * (i * s30 + j * s30 - k);
    return p;
}

static void depingere_cubum(double i, double j, double k, double scala,
                            Color sup, Color sin_, Color dex) {
    /* Vertices cubi ab (i,j,k) ad (i+1,j+1,k+1). */
    Punctum v000 = iso_ad_2D(i,   j,   k,   scala);
    Punctum v100 = iso_ad_2D(i+1, j,   k,   scala);
    Punctum v010 = iso_ad_2D(i,   j+1, k,   scala);
    Punctum v001 = iso_ad_2D(i,   j,   k+1, scala);
    Punctum v101 = iso_ad_2D(i+1, j,   k+1, scala);
    Punctum v011 = iso_ad_2D(i,   j+1, k+1, scala);
    Punctum v111 = iso_ad_2D(i+1, j+1, k+1, scala);
    (void)v000;

    /* Facies superior (k+1): v001, v101, v111, v011 */
    Punctum sup_pts[4] = { v001, v101, v111, v011 };
    impl_polygon(sup_pts, 4, sup);

    /* Facies dextra (i+1): v100, v110..., v111, v101 */
    Punctum v110 = iso_ad_2D(i+1, j+1, k, scala);
    Punctum dex_pts[4] = { v100, v110, v111, v101 };
    impl_polygon(dex_pts, 4, dex);

    /* Facies sinistra (j+1): v010, v110, v111, v011 */
    Punctum sin_pts[4] = { v010, v110, v111, v011 };
    impl_polygon(sin_pts, 4, sin_);
}

/* ============================================================
 * TESSELLATIO
 * ============================================================ */

static void tessellare(void) {
    Color caelum = {60, 70, 110};
    clear_img(caelum);

    Color sup = {220, 210, 180};  /* lux plena */
    Color dex = {150, 135, 100};  /* lux lateralis */
    Color sin_ = {100, 90, 70};   /* umbra */

    double scala = 20.0;

    /* Retinaculum cubicum: iterare per (i, j) circa centrum. Pingere
     * a tergo ad frontem pro occlusione recta. */
    struct Ordo { double i, j, k; double depth; };
    struct Ordo ord[2048];
    int n = 0;

    for (int gi = -8; gi <= 8; gi++) {
        for (int gj = -8; gj <= 8; gj++) {
            /* altitudo variabilis: lenis undulatio */
            double h = sin(gi * 0.6) + cos(gj * 0.6) + 0.3 * sin((gi+gj) * 0.4);
            int kmax = (int)(h + 1.5);
            if (kmax < 0) kmax = 0;
            for (int k = 0; k <= kmax; k++) {
                if (n < 2048) {
                    ord[n].i = gi; ord[n].j = gj; ord[n].k = k;
                    /* profunditas = i + j - k (major = anterius in iso) */
                    ord[n].depth = gi + gj - k * 0.5;
                    n++;
                }
            }
        }
    }

    /* Ordina per depth ascendens (a tergo anterius). */
    for (int a = 1; a < n; a++) {
        struct Ordo t = ord[a]; int b = a - 1;
        while (b >= 0 && ord[b].depth > t.depth) { ord[b+1] = ord[b]; b--; }
        ord[b+1] = t;
    }

    for (int a = 0; a < n; a++) {
        depingere_cubum(ord[a].i, ord[a].j, ord[a].k, scala, sup, sin_, dex);
    }
}

/* ============================================================
 * LINEAE ATRAE ROBUSTAE (detectio limitum)
 * ============================================================ */

static void incidere_lineas(void) {
    /* Post rem depictam, deprehendere limites inter colores et
     * infuscare. */
    unsigned char copy[PIX * 3];
    memcpy(copy, img, sizeof copy);
    for (int y = 1; y < ALT - 1; y++) {
        for (int x = 1; x < LAT - 1; x++) {
            size_t i = (size_t)(y * LAT + x) * 3;
            size_t r = (size_t)(y * LAT + (x + 1)) * 3;
            size_t d = (size_t)((y + 1) * LAT + x) * 3;
            int dr = abs((int)copy[i] - (int)copy[r])
                   + abs((int)copy[i+1] - (int)copy[r+1])
                   + abs((int)copy[i+2] - (int)copy[r+2]);
            int dd = abs((int)copy[i] - (int)copy[d])
                   + abs((int)copy[i+1] - (int)copy[d+1])
                   + abs((int)copy[i+2] - (int)copy[d+2]);
            if (dr > 60 || dd > 60) {
                img[i] = 20; img[i+1] = 20; img[i+2] = 25;
            }
        }
    }
}

/* ============================================================
 * PPM ET ASCII
 * ============================================================ */

static int scribere_ppm(const char *nomen) {
    FILE *f = fopen(nomen, "wb");
    if (!f) { fprintf(stderr, "escherus: nequeo '%s'\n", nomen); return 1; }
    fprintf(f, "P6\n%d %d\n255\n", LAT, ALT);
    size_t n = fwrite(img, 1, (size_t)PIX * 3, f);
    fclose(f);
    return n == (size_t)PIX * 3 ? 0 : 1;
}

static void reddere_ascii(void) {
    const char *ramp = " .:-=+*#%@";
    int nramp = (int)strlen(ramp);
    int cols = 128, rows = 64;
    double sx = (double)LAT / cols;
    double sy = (double)ALT / rows;
    for (int cy = 0; cy < rows; cy++) {
        for (int cx = 0; cx < cols; cx++) {
            int x = (int)(cx * sx);
            int y = (int)(cy * sy);
            Color c = getpx(x, y);
            int lum = (c.r * 30 + c.g * 59 + c.b * 11) / 100;
            int k = lum * nramp / 256;
            if (k < 0) k = 0; if (k >= nramp) k = nramp - 1;
            putchar(ramp[k]);
        }
        putchar('\n');
    }
}

static void auxilium(const char *nomen) {
    printf("Usus: %s [-h] [-o imago.ppm] [-s semen]\n", nomen);
    printf("\nGenerat tessellationem isometricam cuborum in stilo\n");
    printf("Escherii (256x256).\n\n");
    printf("  -h         hoc auxilium\n");
    printf("  -o f.ppm   imago P6 PPM\n");
    printf("  -s N       semen generatoris\n");
}

int main(int argc, char **argv) {
    const char *out = NULL;
    uint64_t semen = SEMEN_DEFALTUM;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            auxilium(argv[0]); return 0;
        } else if (!strcmp(argv[i], "-o")) {
            if (i+1 >= argc) { fprintf(stderr, "escherus: -o?\n"); return 2; }
            out = argv[++i];
        } else if (!strcmp(argv[i], "-s")) {
            if (i+1 >= argc) { fprintf(stderr, "escherus: -s?\n"); return 2; }
            semen = strtoull(argv[++i], NULL, 0);
            if (!semen) semen = 1;
        } else {
            fprintf(stderr, "escherus: ignotum '%s'\n", argv[i]);
            return 2;
        }
    }
    prng_state = semen;
    (void)rnd_unit;
    tessellare();
    incidere_lineas();
    if (out) return scribere_ppm(out);
    reddere_ascii();
    return 0;
}
