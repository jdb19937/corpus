/*
 * pollockius.c — Actionis Pictura (Jackson Pollock)
 *
 * Generat imaginem 256x256 in stilo "action painting" Pollockii:
 * guttae et lineae iactae super linteum, per iterationes motus
 * brachii simulati. Multiplex stratum colorum.
 *
 * Linea simulatur traiectoria quasi ballistica: positio et
 * velocitas mutantur, et inde guttae sparguntur.
 *
 * cc -std=c99 -Wall -Wextra -pedantic pollockius.c -o pollockius -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define LAT 256
#define ALT 256
#define PIX (LAT*ALT)
#define SEMEN_DEFALTUM 0xF077AC0ULL

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct { double r, g, b; } Colorf;

static unsigned char img[PIX*3];
static uint64_t rng;
static uint64_t xs(void){ uint64_t x=rng; x^=x<<13; x^=x>>7; x^=x<<17; rng=x; return x; }
static double runit(void){ return (double)(xs()>>11)/(double)(1ULL<<53); }
static int rrange(int a, int b){ if(b<=a) return a; return a + (int)(xs()%(uint64_t)(b-a)); }
static double rnorm(void){
    /* Box-Muller */
    double u1 = runit(); if (u1 < 1e-9) u1 = 1e-9;
    double u2 = runit();
    return sqrt(-2.0 * log(u1)) * cos(2 * M_PI * u2);
}

static double clampd(double v, double a, double b){ return v<a?a:(v>b?b:v); }
static Colorf mix(Colorf a, Colorf b, double t){ Colorf c={a.r+(b.r-a.r)*t, a.g+(b.g-a.g)*t, a.b+(b.b-a.b)*t}; return c; }
static void setpx_alpha(int x, int y, Colorf c, double alpha) {
    if (x < 0 || x >= LAT || y < 0 || y >= ALT) return;
    size_t i = (size_t)(y*LAT+x)*3;
    Colorf o = { img[i]/255.0, img[i+1]/255.0, img[i+2]/255.0 };
    Colorf f = mix(o, c, alpha);
    img[i]   = (unsigned char)(clampd(f.r,0,1)*255);
    img[i+1] = (unsigned char)(clampd(f.g,0,1)*255);
    img[i+2] = (unsigned char)(clampd(f.b,0,1)*255);
}
static void setpx(int x, int y, Colorf c) { setpx_alpha(x, y, c, 1.0); }

/* Paletta Pollockiana: atrum, album, brunum, subalbum, rubrum,
 * aurantium, viride, caeruleum. */
static const Colorf PAL[] = {
    {0.08, 0.08, 0.10},   /* atrum */
    {0.95, 0.93, 0.87},   /* album */
    {0.45, 0.30, 0.18},   /* brunum */
    {0.82, 0.72, 0.55},   /* subalbum */
    {0.72, 0.18, 0.15},   /* rubrum */
    {0.90, 0.55, 0.15},   /* aurantium */
    {0.30, 0.50, 0.25},   /* viride */
    {0.20, 0.30, 0.55},   /* caeruleum */
    {0.95, 0.85, 0.25},   /* flavum */
};
#define NPAL ((int)(sizeof(PAL)/sizeof(PAL[0])))

/* ============================================================
 * LINTEUM: textura lini sub gradiente
 * ============================================================ */
static void linteum(void) {
    Colorf a = {0.90, 0.85, 0.70};
    Colorf b = {0.75, 0.70, 0.55};
    for (int y = 0; y < ALT; y++) {
        for (int x = 0; x < LAT; x++) {
            /* Textura linearis lini */
            double n = 0.5 + 0.5 * sin(x * 0.9 + y * 0.1) * sin(y * 0.8 + x * 0.1);
            n += 0.05 * (runit() - 0.5);
            Colorf c = mix(a, b, clampd(n, 0, 1));
            setpx(x, y, c);
        }
    }
}

/* ============================================================
 * GUTTA (sparsio punctorum fortuitorum circum locum datum)
 * ============================================================ */
static void gutta(double cx, double cy, double radius, Colorf c, double densitas) {
    int n = (int)(radius * radius * densitas);
    for (int i = 0; i < n; i++) {
        double r = sqrt(runit()) * radius;
        double theta = runit() * 2 * M_PI;
        int x = (int)(cx + r * cos(theta));
        int y = (int)(cy + r * sin(theta));
        double alpha = 0.5 + 0.5 * runit();
        setpx_alpha(x, y, c, alpha);
    }
}

/* ============================================================
 * TRAIECTIO: linea fluida cum velocitate et gravitate
 * ============================================================ */
static void traiectio(Colorf c) {
    double x = runit() * LAT;
    double y = runit() * ALT;
    double angle = runit() * 2 * M_PI;
    double speed = 1.5 + runit() * 3.5;
    double vx = cos(angle) * speed;
    double vy = sin(angle) * speed;
    int steps = 200 + rrange(0, 500);
    double pen_w = 0.8 + runit() * 2.5;
    for (int s = 0; s < steps; s++) {
        /* Penicillus depingit ellipsoidem secundum directionem motus. */
        double dir = atan2(vy, vx);
        double maj = pen_w * (1.0 + speed * 0.15);
        double min = pen_w;
        int n = (int)(maj * 4);
        for (int i = 0; i < n; i++) {
            double t = (double)i / n - 0.5;
            double rr = runit() * min;
            double theta = runit() * 2 * M_PI;
            double lx = t * maj * 2;
            double ex = cos(dir) * lx - sin(dir) * rr * cos(theta);
            double ey = sin(dir) * lx + cos(dir) * rr * sin(theta);
            int px = (int)(x + ex);
            int py = (int)(y + ey);
            setpx_alpha(px, py, c, 0.7);
        }
        /* Aliquando guttae maiores. */
        if (runit() < 0.015) {
            gutta(x, y, 3 + runit() * 5, c, 6.0);
        }
        /* Sparsus parvus circum traiectoriam. */
        if (runit() < 0.2) {
            double sx = x + rnorm() * 3;
            double sy = y + rnorm() * 3;
            setpx_alpha((int)sx, (int)sy, c, 0.4);
        }
        /* Mutatio directionis. */
        double dax = rnorm() * 0.18;
        double day = rnorm() * 0.18;
        vx += dax;
        vy += day;
        /* Limes velocitatis. */
        double sp2 = vx*vx + vy*vy;
        if (sp2 > 30) { double k = sqrt(30/sp2); vx *= k; vy *= k; }
        x += vx;
        y += vy;
        /* Extra marginem: desinere. */
        if (x < -20 || x > LAT + 20 || y < -20 || y > ALT + 20) break;
    }
}

/* ============================================================
 * SPATTER (gutta latior cum densitate)
 * ============================================================ */
static void spatter_plane(Colorf c, int n, double size_mean) {
    for (int i = 0; i < n; i++) {
        double x = runit() * LAT;
        double y = runit() * ALT;
        double s = size_mean * (0.3 + runit() * 1.5);
        /* Forma parva ovalis */
        int np = (int)(s * s * 2);
        double dir = runit() * 2 * M_PI;
        double asp = 1.0 + runit() * 3.0;
        for (int k = 0; k < np; k++) {
            double r = sqrt(runit()) * s;
            double theta = runit() * 2 * M_PI;
            double lx = r * cos(theta) * asp;
            double ly = r * sin(theta);
            double ex = cos(dir) * lx - sin(dir) * ly;
            double ey = sin(dir) * lx + cos(dir) * ly;
            setpx_alpha((int)(x + ex), (int)(y + ey), c, 0.8);
        }
        /* Pilus exiliens a gutta. */
        for (int k = 0; k < (int)s; k++) {
            double rr = s + runit() * s * 3;
            double tt = dir + (runit() - 0.5) * 0.6;
            int px = (int)(x + rr * cos(tt));
            int py = (int)(y + rr * sin(tt));
            setpx_alpha(px, py, c, 0.5);
        }
    }
}

/* ============================================================
 * SETS OF TRAIECTIONES
 * ============================================================ */
static void depingere(void) {
    linteum();

    /* Ordo strata: brunum, deinde caeruleum/viride, deinde rubrum,
     * deinde album, deinde atrum, deinde aurantium. */
    const int ordo[] = {2, 7, 6, 4, 8, 1, 0, 5};
    int no = sizeof(ordo)/sizeof(ordo[0]);

    for (int si = 0; si < no; si++) {
        Colorf c = PAL[ordo[si]];
        int iters = 3 + rrange(0, 5);
        for (int i = 0; i < iters; i++) {
            traiectio(c);
        }
        spatter_plane(c, 12 + rrange(0, 20), 1.5);
    }
    /* Guttae finales magnae. */
    for (int i = 0; i < 8; i++) {
        Colorf c = PAL[rrange(0, NPAL)];
        gutta(runit() * LAT, runit() * ALT, 3 + runit() * 6, c, 12.0);
    }
}

/* ============================================================
 * PPM + ANSI
 * ============================================================ */
static int scribere_ppm(const char *n) {
    FILE *f = fopen(n, "wb");
    if (!f) { fprintf(stderr, "pollockius: nequeo '%s'\n", n); return 1; }
    fprintf(f, "P6\n%d %d\n255\n", LAT, ALT);
    size_t m = fwrite(img, 1, (size_t)PIX*3, f);
    fclose(f);
    return m == (size_t)PIX*3 ? 0 : 1;
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
    int cols = 64, rows = 32;
    for (int cy = 0; cy < rows; cy++) {
        for (int cx = 0; cx < cols; cx++) {
            unsigned long tr=0,tg=0,tb=0,br=0,bg=0,bb=0; int ct=0,cb=0;
            int bx = cx*4, by = cy*8;
            for (int dy = 0; dy < 4; dy++)
                for (int dx = 0; dx < 4; dx++) {
                    int x = bx+dx, y = by+dy;
                    if (x<LAT && y<ALT) { size_t i=(size_t)(y*LAT+x)*3; tr+=img[i]; tg+=img[i+1]; tb+=img[i+2]; ct++; }
                    y = by+4+dy;
                    if (x<LAT && y<ALT) { size_t i=(size_t)(y*LAT+x)*3; br+=img[i]; bg+=img[i+1]; bb+=img[i+2]; cb++; }
                }
            if (!ct) ct=1; if (!cb) cb=1;
            int fg = rgb_ad_256(tr/ct, tg/ct, tb/ct);
            int bg_ = rgb_ad_256(br/cb, bg/cb, bb/cb);
            printf("\x1b[38;5;%dm\x1b[48;5;%dm\xe2\x96\x80", fg, bg_);
        }
        printf("\x1b[0m\n");
    }
}

static void auxilium(const char *n) {
    printf("Usus: %s [-h] [-o imago.ppm] [-s semen]\n", n);
    printf("\nActionis Pictura Pollockiana (256x256): guttae et\n");
    printf("traiectiones multicolores super linteum.\n\n");
    printf("  -h         auxilium\n");
    printf("  -o f.ppm   scriptio PPM\n");
    printf("  -s N       semen (mutat guttas)\n");
}

int main(int argc, char **argv) {
    const char *out = NULL;
    uint64_t semen = SEMEN_DEFALTUM;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) { auxilium(argv[0]); return 0; }
        else if (!strcmp(argv[i], "-o")) {
            if (i+1 >= argc) { fprintf(stderr, "pollockius: -o?\n"); return 2; }
            out = argv[++i];
        } else if (!strcmp(argv[i], "-s")) {
            if (i+1 >= argc) { fprintf(stderr, "pollockius: -s?\n"); return 2; }
            semen = strtoull(argv[++i], NULL, 0); if (!semen) semen = 1;
        } else { fprintf(stderr, "pollockius: ignotum '%s'\n", argv[i]); return 2; }
    }
    rng = semen;
    depingere();
    if (out) return scribere_ppm(out);
    reddere_ansi();
    return 0;
}
