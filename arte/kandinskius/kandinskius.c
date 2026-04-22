/*
 * kandinskius.c — Compositio Abstracta (Wassily Kandinsky)
 *
 * Generat imaginem 256x256 in stilo compositionum geometricarum
 * Kandinskii ("Compositio VIII"): variae figurae abstractae —
 * circuli concentrici, trianguli, lineae rectae et curvae,
 * chorda arcuum — super fundo leni eburneo.
 *
 * cc -std=c99 -Wall -Wextra -pedantic kandinskius.c -o kandinskius -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define LAT 256
#define ALT 256
#define PIX (LAT*ALT)
#define SEMEN_DEFALTUM 0xCA11D15C1ULL

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct { double r, g, b; } Colorf;

static unsigned char img[PIX*3];
static uint64_t rng;
static uint64_t xs(void){ uint64_t x=rng; x^=x<<13; x^=x>>7; x^=x<<17; rng=x; return x; }
static double runit(void){ return (double)(xs()>>11) / (double)(1ULL<<53); }
static int rrange(int a, int b){ if(b<=a) return a; return a + (int)(xs() % (uint64_t)(b-a)); }

static double clampd(double v,double a,double b){ return v<a?a:(v>b?b:v); }
static Colorf mix(Colorf a, Colorf b, double t){ Colorf c={a.r+(b.r-a.r)*t, a.g+(b.g-a.g)*t, a.b+(b.b-a.b)*t}; return c; }
static void setpx(int x, int y, Colorf c){
    if(x<0||x>=LAT||y<0||y>=ALT) return;
    size_t i=(size_t)(y*LAT+x)*3;
    img[i]=(unsigned char)(clampd(c.r,0,1)*255);
    img[i+1]=(unsigned char)(clampd(c.g,0,1)*255);
    img[i+2]=(unsigned char)(clampd(c.b,0,1)*255);
}
static Colorf getpx(int x, int y){
    Colorf c={1,1,1};
    if(x<0||x>=LAT||y<0||y>=ALT) return c;
    size_t i=(size_t)(y*LAT+x)*3;
    c.r=img[i]/255.0; c.g=img[i+1]/255.0; c.b=img[i+2]/255.0;
    return c;
}
static void blend(int x, int y, Colorf c, double alpha){
    Colorf o = getpx(x, y);
    setpx(x, y, mix(o, c, alpha));
}

/* ============================================================
 * PALETTA KANDINSKIANA (novem colores vividi)
 * ============================================================ */
static const Colorf PAL[] = {
    {0.93, 0.17, 0.15}, /* rubrum vivum */
    {0.18, 0.33, 0.68}, /* caeruleum ultramarinum */
    {0.98, 0.82, 0.12}, /* flavum */
    {0.12, 0.14, 0.18}, /* atrum profundum */
    {0.24, 0.52, 0.30}, /* viride pratense */
    {0.85, 0.45, 0.10}, /* aurantium */
    {0.55, 0.20, 0.55}, /* purpureum */
    {0.78, 0.78, 0.80}, /* cinericium clarum */
    {0.08, 0.55, 0.65}, /* turquinum */
};
#define NPAL ((int)(sizeof(PAL)/sizeof(PAL[0])))
static Colorf palrand(void) { return PAL[rrange(0, NPAL)]; }

/* ============================================================
 * PRIMITIVAE
 * ============================================================ */

static void circulus_plenus(double cx, double cy, double r, Colorf c) {
    int x0 = (int)(cx - r - 1), x1 = (int)(cx + r + 1);
    int y0 = (int)(cy - r - 1), y1 = (int)(cy + r + 1);
    for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++) {
            double dx = x + 0.5 - cx, dy = y + 0.5 - cy;
            double d = sqrt(dx*dx + dy*dy);
            if (d < r - 1) setpx(x, y, c);
            else if (d < r) blend(x, y, c, r - d);
        }
}
static void annulus(double cx, double cy, double ri, double ro, Colorf c) {
    int x0 = (int)(cx - ro - 1), x1 = (int)(cx + ro + 1);
    int y0 = (int)(cy - ro - 1), y1 = (int)(cy + ro + 1);
    for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++) {
            double dx = x + 0.5 - cx, dy = y + 0.5 - cy;
            double d = sqrt(dx*dx + dy*dy);
            if (d > ri && d < ro) setpx(x, y, c);
            else if (d <= ri && d > ri - 1) blend(x, y, c, d - (ri - 1));
            else if (d >= ro && d < ro + 1) blend(x, y, c, 1 - (d - ro));
        }
}
static void linea(double x0, double y0, double x1, double y1, double w, Colorf c) {
    double dx = x1 - x0, dy = y1 - y0;
    double l = sqrt(dx*dx + dy*dy);
    if (l < 0.5) return;
    int n = (int)(l * 3);
    for (int i = 0; i <= n; i++) {
        double t = (double)i / n;
        double x = x0 + t * dx, y = y0 + t * dy;
        circulus_plenus(x, y, w * 0.5, c);
    }
}
static void triangulum(double x0, double y0, double x1, double y1,
                       double x2, double y2, Colorf c) {
    double ymin = y0, ymax = y0;
    if (y1 < ymin) ymin = y1; if (y2 < ymin) ymin = y2;
    if (y1 > ymax) ymax = y1; if (y2 > ymax) ymax = y2;
    int iy0 = (int)floor(ymin), iy1 = (int)ceil(ymax);
    if (iy0 < 0) iy0 = 0; if (iy1 > ALT) iy1 = ALT;
    for (int y = iy0; y < iy1; y++) {
        double yc = y + 0.5;
        double xs[6]; int nx = 0;
        double px[3] = {x0, x1, x2}, py[3] = {y0, y1, y2};
        for (int e = 0; e < 3; e++) {
            double ax = px[e], ay = py[e];
            double bx = px[(e+1)%3], by = py[(e+1)%3];
            if ((ay <= yc && by > yc) || (by <= yc && ay > yc)) {
                double t = (yc - ay) / (by - ay);
                xs[nx++] = ax + t * (bx - ax);
            }
        }
        if (nx < 2) continue;
        for (int i = 1; i < nx; i++) {
            double v = xs[i]; int j = i - 1;
            while (j >= 0 && xs[j] > v) { xs[j+1] = xs[j]; j--; }
            xs[j+1] = v;
        }
        int a = (int)floor(xs[0]), b = (int)ceil(xs[nx-1]);
        if (a < 0) a = 0; if (b > LAT) b = LAT;
        for (int x = a; x < b; x++) setpx(x, y, c);
    }
}

static void arcus(double cx, double cy, double r, double theta0, double theta1,
                  double w, Colorf c) {
    int n = (int)(r * fabs(theta1 - theta0) * 2) + 4;
    for (int i = 0; i <= n; i++) {
        double t = theta0 + (theta1 - theta0) * i / n;
        double x = cx + r * cos(t), y = cy + r * sin(t);
        circulus_plenus(x, y, w * 0.5, c);
    }
}

/* ============================================================
 * COMPOSITIO
 * ============================================================ */

static void depingere(void) {
    Colorf fund = {0.96, 0.93, 0.87};
    for (int y = 0; y < ALT; y++)
        for (int x = 0; x < LAT; x++) {
            /* textura tenuis: subtilis variatio */
            double n = ((sin(x*0.3)*cos(y*0.27))+1.0)*0.5;
            Colorf c = fund;
            c.r *= 0.97 + n * 0.03;
            c.g *= 0.97 + n * 0.03;
            c.b *= 0.97 + n * 0.03;
            setpx(x, y, c);
        }

    /* Retroductum: magna figura abstracta (cinericia). */
    Colorf bg = {0.72, 0.76, 0.82};
    triangulum(20, 200, 200, 40, 240, 220, bg);

    /* Circuli concentrici maior. */
    double cx = 180, cy = 75;
    circulus_plenus(cx, cy, 40, PAL[1]);
    circulus_plenus(cx, cy, 32, PAL[7]);
    circulus_plenus(cx, cy, 24, PAL[0]);
    annulus(cx, cy, 15, 20, PAL[3]);
    circulus_plenus(cx, cy, 10, PAL[2]);

    /* Alter circulus minor. */
    circulus_plenus(60, 80, 22, PAL[2]);
    annulus(60, 80, 17, 20, PAL[3]);
    circulus_plenus(60, 80, 10, PAL[0]);

    /* Triangulus magnus angularis. */
    triangulum(30, 60, 110, 150, 10, 170, PAL[4]);

    /* Triangulus acutus inversus. */
    triangulum(140, 180, 200, 240, 100, 245, PAL[5]);

    /* Lineae rectae audaces. */
    linea(10, 30, 245, 50, 2.5, PAL[3]);
    linea(200, 10, 170, 250, 3.0, PAL[3]);
    linea(50, 100, 220, 210, 1.8, PAL[6]);
    linea(80, 180, 240, 150, 1.4, PAL[1]);

    /* Arcus. */
    arcus(128, 128, 60, M_PI * 0.2, M_PI * 0.9, 2.5, PAL[0]);
    arcus(40, 200, 35, -M_PI * 0.4, M_PI * 0.5, 1.8, PAL[8]);
    arcus(210, 175, 28, M_PI, M_PI * 2.0, 2.2, PAL[2]);

    /* Pauca puncta sparsa. */
    for (int i = 0; i < 14; i++) {
        double x = runit() * LAT;
        double y = runit() * ALT;
        double r = 1.8 + runit() * 2.5;
        circulus_plenus(x, y, r, PAL[rrange(0, NPAL)]);
    }

    /* Tesselatio minor: parva quadrata varia. */
    for (int i = 0; i < 8; i++) {
        int x = rrange(10, 240);
        int y = rrange(10, 240);
        int s = rrange(3, 8);
        Colorf c = palrand();
        for (int dy = 0; dy < s; dy++)
            for (int dx = 0; dx < s; dx++)
                setpx(x + dx, y + dy, c);
    }

    /* "Tabula scacharia" angularis. */
    double tcx = 55, tcy = 210;
    Colorf ta = PAL[3], tb = {0.98, 0.96, 0.90};
    for (int j = 0; j < 4; j++)
        for (int i = 0; i < 4; i++) {
            Colorf c = ((i + j) & 1) ? ta : tb;
            /* rotate 20° */
            double ang = 0.35;
            for (double dy = 0; dy < 6; dy += 0.5)
                for (double dx = 0; dx < 6; dx += 0.5) {
                    double rx = tcx + (i*6 + dx - 12) * cos(ang) - (j*6 + dy - 12) * sin(ang);
                    double ry = tcy + (i*6 + dx - 12) * sin(ang) + (j*6 + dy - 12) * cos(ang);
                    setpx((int)rx, (int)ry, c);
                }
        }

    /* Linea crispans serpentina. */
    Colorf serp = PAL[6];
    double sx = 10, sy = 130;
    for (int i = 0; i < 240; i++) {
        double t = (double)i / 240;
        double x = 10 + t * 236;
        double y = 130 + 25 * sin(t * M_PI * 4.0);
        linea(sx, sy, x, y, 1.5, serp);
        sx = x; sy = y;
    }
}

/* ============================================================
 * PPM + ANSI
 * ============================================================ */

static int scribere_ppm(const char *n) {
    FILE *f = fopen(n, "wb");
    if (!f) { fprintf(stderr, "kandinskius: nequeo '%s'\n", n); return 1; }
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
                    int x = bx + dx, y = by + dy;
                    if (x<LAT && y<ALT) { size_t i=(size_t)(y*LAT+x)*3; tr+=img[i]; tg+=img[i+1]; tb+=img[i+2]; ct++; }
                    y = by + 4 + dy;
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
    printf("\nCompositio abstracta Kandinskiana (256x256).\n\n");
    printf("  -h         auxilium\n");
    printf("  -o f.ppm   scriptio PPM\n");
    printf("  -s N       semen (mutat puncta sparsa)\n");
}

int main(int argc, char **argv) {
    const char *out = NULL;
    uint64_t semen = SEMEN_DEFALTUM;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) { auxilium(argv[0]); return 0; }
        else if (!strcmp(argv[i], "-o")) {
            if (i+1 >= argc) { fprintf(stderr, "kandinskius: -o?\n"); return 2; }
            out = argv[++i];
        } else if (!strcmp(argv[i], "-s")) {
            if (i+1 >= argc) { fprintf(stderr, "kandinskius: -s?\n"); return 2; }
            semen = strtoull(argv[++i], NULL, 0); if (!semen) semen = 1;
        } else { fprintf(stderr, "kandinskius: ignotum '%s'\n", argv[i]); return 2; }
    }
    rng = semen;
    depingere();
    if (out) return scribere_ppm(out);
    reddere_ansi();
    return 0;
}
