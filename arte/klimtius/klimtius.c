/*
 * klimtius.c — Arbor Vitae (Gustav Klimt)
 *
 * Generat imaginem 256x256 in stilo aureo Klimtiano: arbor cum
 * ramis spiralibus super fundo aureo, et ornamenta geometrica
 * (circuli, oculi, spirae) variis formis disposita.
 *
 * Rami generantur per systema recursivum simile systemati L
 * (L-system), more testudinis graphicae; folia sunt spirae
 * Archimedis minutae.
 *
 * cc -std=c99 -Wall -Wextra -pedantic klimtius.c -o klimtius -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define LAT 256
#define ALT 256
#define PIX (LAT*ALT)
#define SEMEN_DEFALTUM 0xC177A8ULL

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct { double r, g, b; } Colorf;

static unsigned char img[PIX*3];
static uint64_t rng;
static uint64_t xs(void){ uint64_t x=rng; x^=x<<13; x^=x>>7; x^=x<<17; rng=x; return x; }
static double runit(void){ return (double)(xs()>>11)/(double)(1ULL<<53); }

static double clampd(double v, double a, double b){ return v<a?a:(v>b?b:v); }
static Colorf mix(Colorf a, Colorf b, double t){ Colorf c={a.r+(b.r-a.r)*t, a.g+(b.g-a.g)*t, a.b+(b.b-a.b)*t}; return c; }
static void setpx(int x, int y, Colorf c){
    if(x<0||x>=LAT||y<0||y>=ALT) return;
    size_t i=(size_t)(y*LAT+x)*3;
    img[i]=(unsigned char)(clampd(c.r,0,1)*255);
    img[i+1]=(unsigned char)(clampd(c.g,0,1)*255);
    img[i+2]=(unsigned char)(clampd(c.b,0,1)*255);
}
static Colorf getpx(int x, int y){
    Colorf c={0,0,0};
    if(x<0||x>=LAT||y<0||y>=ALT) return c;
    size_t i=(size_t)(y*LAT+x)*3;
    c.r=img[i]/255.0; c.g=img[i+1]/255.0; c.b=img[i+2]/255.0;
    return c;
}

/* Palettae aureae: varia. */
static const Colorf AURUM_CLARUM = {0.98, 0.82, 0.30};
static const Colorf AURUM_MEDIUM = {0.85, 0.65, 0.15};
static const Colorf AURUM_OBSCURUM = {0.55, 0.40, 0.08};
static const Colorf ATRUM = {0.05, 0.05, 0.08};
static const Colorf RUBRUM = {0.65, 0.10, 0.08};
static const Colorf ALBUM = {0.98, 0.95, 0.82};
static const Colorf VIRIDIS = {0.25, 0.55, 0.25};

/* ============================================================
 * FUNDUM AUREUM
 * ============================================================ */
static void fundum(void) {
    for (int y = 0; y < ALT; y++) {
        for (int x = 0; x < LAT; x++) {
            double t = 0.5 + 0.5 * sin(x * 0.09 + y * 0.07);
            Colorf c = mix(AURUM_MEDIUM, AURUM_CLARUM, t);
            /* vibratio auri: subtile puncta */
            double n = sin(x * 3.1 + y * 1.7) * cos(y * 2.3 - x * 1.1);
            c.r += n * 0.04;
            c.g += n * 0.03;
            setpx(x, y, c);
        }
    }
    /* Margine obscuro. */
    for (int y = 0; y < ALT; y++) {
        for (int x = 0; x < LAT; x++) {
            double dx = (x - 128.0) / 128.0;
            double dy = (y - 128.0) / 128.0;
            double d = sqrt(dx*dx + dy*dy);
            if (d > 0.9) {
                double t = clampd((d - 0.9) * 10.0, 0, 1);
                Colorf c = getpx(x, y);
                c = mix(c, AURUM_OBSCURUM, t * 0.7);
                setpx(x, y, c);
            }
        }
    }
}

/* ============================================================
 * CIRCULI ORNAMENTALES
 * ============================================================ */
static void circulus_plenus(double cx, double cy, double r, Colorf c) {
    int x0 = (int)(cx - r - 1), x1 = (int)(cx + r + 1);
    int y0 = (int)(cy - r - 1), y1 = (int)(cy + r + 1);
    for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++) {
            double dx = x + 0.5 - cx, dy = y + 0.5 - cy;
            if (dx*dx + dy*dy < r*r) setpx(x, y, c);
        }
}
static void circulus_cavus(double cx, double cy, double r, double w, Colorf c) {
    int x0 = (int)(cx - r - w - 1), x1 = (int)(cx + r + w + 1);
    int y0 = (int)(cy - r - w - 1), y1 = (int)(cy + r + w + 1);
    for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++) {
            double dx = x + 0.5 - cx, dy = y + 0.5 - cy;
            double d = sqrt(dx*dx + dy*dy);
            if (d >= r && d <= r + w) setpx(x, y, c);
        }
}

/* ============================================================
 * SPIRA ARCHIMEDES (folia spiralia Klimtiana)
 * ============================================================ */
static void spira(double cx, double cy, double scala, double rotatio,
                  double crassitudo, Colorf c) {
    double a = scala * 0.08;
    double b = scala * 0.55;
    int n = 360;
    for (int i = 0; i < n; i++) {
        double theta = i * M_PI / 48.0;
        double r = a + b * theta / (M_PI * 2);
        double x = cx + r * cos(theta + rotatio);
        double y = cy + r * sin(theta + rotatio);
        circulus_plenus(x, y, crassitudo, c);
        if (r > scala * 4.5) break;
    }
}

/* ============================================================
 * RAMI (more testudinis graphicae recursivo)
 * ============================================================ */
static void linea_crassa(double x0, double y0, double x1, double y1,
                         double w, Colorf c) {
    double dx = x1 - x0, dy = y1 - y0;
    double len = sqrt(dx*dx + dy*dy);
    if (len < 0.5) return;
    int n = (int)(len * 2) + 1;
    for (int i = 0; i <= n; i++) {
        double t = (double)i / n;
        double x = x0 + t * dx, y = y0 + t * dy;
        circulus_plenus(x, y, w * 0.5, c);
    }
}

static void ramus(double x, double y, double angle, double len, int depth) {
    if (depth <= 0 || len < 2) return;
    /* Corpus rami. */
    double x1 = x + cos(angle) * len;
    double y1 = y + sin(angle) * len;
    double w = 1.0 + depth * 0.7;
    linea_crassa(x, y, x1, y1, w, ATRUM);

    /* Folium spirale ad apicem si profundum. */
    if (depth <= 2 && runit() < 0.4) {
        double scala = 3.5 + runit() * 4.0;
        spira(x1, y1, scala, runit() * M_PI * 2, 0.9, AURUM_OBSCURUM);
    }

    /* Recursiones: 2-3 sub-rami cum angulis variis. */
    int branches = 2 + (runit() < 0.3 ? 1 : 0);
    for (int i = 0; i < branches; i++) {
        double delta = (runit() - 0.5) * 1.4 + (i == 0 ? -0.4 : 0.4);
        double new_angle = angle + delta;
        double new_len = len * (0.65 + runit() * 0.2);
        ramus(x1, y1, new_angle, new_len, depth - 1);
    }
}

/* ============================================================
 * ORNAMENTA (puncta, quadrata, trianguli sparsi)
 * ============================================================ */
static void ornamenta(void) {
    for (int i = 0; i < 80; i++) {
        double x = runit() * LAT;
        double y = runit() * ALT;
        double dx = x - 128, dy = y - 128;
        /* Ne in centro (ubi arbor). */
        if (sqrt(dx*dx + dy*dy) < 30 && fabs(dx) < 20) continue;
        int k = (int)(runit() * 5);
        Colorf c = (k % 2) ? AURUM_OBSCURUM : ATRUM;
        if (k == 0) {
            /* Circulus ornatus */
            double r = 2 + runit() * 3;
            circulus_plenus(x, y, r, c);
        } else if (k == 1) {
            /* Oculus (circulus in circulo) */
            double r = 4 + runit() * 2;
            circulus_plenus(x, y, r, ATRUM);
            circulus_plenus(x, y, r * 0.5, ALBUM);
            circulus_plenus(x, y, r * 0.2, ATRUM);
        } else if (k == 2) {
            /* Quadratum parvum */
            int s = (int)(2 + runit() * 3);
            for (int dyy = 0; dyy < s; dyy++)
                for (int dxx = 0; dxx < s; dxx++)
                    setpx((int)x + dxx, (int)y + dyy, c);
        } else if (k == 3) {
            /* Stellula */
            for (int a = 0; a < 6; a++) {
                double ang = a * M_PI / 3;
                double ex = x + cos(ang) * 4;
                double ey = y + sin(ang) * 4;
                linea_crassa(x, y, ex, ey, 1, c);
            }
        } else {
            /* Spirae minima */
            spira(x, y, 1.5, runit() * M_PI * 2, 0.5, c);
        }
    }
}

/* ============================================================
 * DEPINGERE
 * ============================================================ */
static void depingere(void) {
    fundum();
    ornamenta();

    /* Radix arboris. */
    double root_x = 128, root_y = 240;
    linea_crassa(root_x - 20, root_y + 5, root_x, 220, 6, ATRUM);
    linea_crassa(root_x + 18, root_y + 5, root_x, 220, 6, ATRUM);
    linea_crassa(root_x, root_y, root_x, 200, 8, ATRUM);

    /* Truncus principalis. */
    linea_crassa(root_x, 220, root_x - 2, 150, 7, ATRUM);
    linea_crassa(root_x - 2, 150, root_x + 4, 120, 6, ATRUM);

    /* Rami. */
    ramus(root_x + 4, 120, -M_PI / 2 - 0.1, 28, 5);
    ramus(root_x + 4, 120, -M_PI / 2 + 0.6, 22, 5);
    ramus(root_x + 4, 120, -M_PI / 2 - 0.7, 24, 5);
    ramus(root_x, 150, -M_PI / 2 - 1.1, 20, 4);
    ramus(root_x, 150, -M_PI / 2 + 1.1, 20, 4);
    ramus(root_x - 2, 170, -M_PI / 2 + 1.4, 16, 3);
    ramus(root_x - 2, 190, -M_PI / 2 - 1.4, 16, 3);

    /* Foliata maiora — spirae in vertice. */
    for (int i = 0; i < 12; i++) {
        double ang = M_PI * 2 * i / 12 - M_PI / 2;
        double r = 50 + runit() * 20;
        double x = root_x + cos(ang) * r;
        double y = 120 + sin(ang) * r * 0.8;
        if (y < 10) y = 10;
        spira(x, y, 6 + runit() * 4, runit() * M_PI * 2, 1.2,
              i % 2 ? AURUM_OBSCURUM : ATRUM);
    }

    /* Flores (circuli rubro et albo). */
    double flores[][2] = {
        {70, 80}, {200, 70}, {60, 150}, {210, 140},
        {100, 60}, {170, 85}, {80, 200}, {195, 195},
        {140, 75}, {115, 165}, {175, 165}
    };
    int nf = (int)(sizeof flores / sizeof flores[0]);
    for (int i = 0; i < nf; i++) {
        double fx = flores[i][0], fy = flores[i][1];
        Colorf centrum_col = (i % 3 == 0) ? RUBRUM : ATRUM;
        Colorf pet_col = (i % 2 == 0) ? ALBUM : AURUM_CLARUM;
        if (i % 4 == 3) pet_col = VIRIDIS;
        circulus_plenus(fx, fy, 5.5, pet_col);
        circulus_plenus(fx, fy, 2.5, centrum_col);
        /* Petals */
        for (int p = 0; p < 6; p++) {
            double a = p * M_PI / 3;
            double px = fx + cos(a) * 5.5;
            double py = fy + sin(a) * 5.5;
            circulus_plenus(px, py, 1.5, pet_col);
        }
    }

    /* Margo circumdans tenuis. */
    for (int i = 0; i < 3; i++) {
        Colorf c = (i == 1) ? AURUM_CLARUM : ATRUM;
        for (int x = 0; x < LAT; x++) {
            setpx(x, i, c); setpx(x, ALT - 1 - i, c);
        }
        for (int y = 0; y < ALT; y++) {
            setpx(i, y, c); setpx(LAT - 1 - i, y, c);
        }
    }
}

/* ============================================================
 * PPM + ANSI
 * ============================================================ */
static int scribere_ppm(const char *n) {
    FILE *f = fopen(n, "wb");
    if (!f) { fprintf(stderr, "klimtius: nequeo '%s'\n", n); return 1; }
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
    printf("\nArbor Vitae aurea Klimtiana (256x256).\n\n");
    printf("  -h         auxilium\n");
    printf("  -o f.ppm   scriptio PPM\n");
    printf("  -s N       semen ramorum\n");
    (void)circulus_cavus;
}

int main(int argc, char **argv) {
    const char *out = NULL;
    uint64_t semen = SEMEN_DEFALTUM;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) { auxilium(argv[0]); return 0; }
        else if (!strcmp(argv[i], "-o")) {
            if (i+1 >= argc) { fprintf(stderr, "klimtius: -o?\n"); return 2; }
            out = argv[++i];
        } else if (!strcmp(argv[i], "-s")) {
            if (i+1 >= argc) { fprintf(stderr, "klimtius: -s?\n"); return 2; }
            semen = strtoull(argv[++i], NULL, 0); if (!semen) semen = 1;
        } else { fprintf(stderr, "klimtius: ignotum '%s'\n", argv[i]); return 2; }
    }
    rng = semen;
    depingere();
    if (out) return scribere_ppm(out);
    reddere_ansi();
    return 0;
}
