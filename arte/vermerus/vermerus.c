/*
 * vermerus.c — Puella cum Margarita (Johannes Vermeer)
 *
 * Generat imaginem 256x256 in stilo Vermeri: figura feminina
 * a sinistra luce illuminata, fundus obscurus, et unio in aure
 * (reflexus minimus). Chiaroscuro classicus hollandicus.
 *
 * Constat ex:
 *  - fundo obscuro nigro-olivaceo
 *  - capite ovali cum cervice et umero
 *  - velamine capitis caeruleo cum parte flava (turbante)
 *  - luce a fenestra sinistra
 *  - parva margarita in aure dextra (a spectatoris sinistra)
 *
 * cc -std=c99 -Wall -Wextra -pedantic vermerus.c -o vermerus -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define LAT 256
#define ALT 256
#define PIX (LAT * ALT)
#define SEMEN_DEFALTUM 0xFE8EE8ULL

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct { double r, g, b; } Colorf;

static unsigned char img[PIX * 3];

static double clampd(double x, double lo, double hi) { return x<lo?lo:(x>hi?hi:x); }
static Colorf mix(Colorf a, Colorf b, double t) {
    Colorf c = { a.r + (b.r-a.r)*t, a.g + (b.g-a.g)*t, a.b + (b.b-a.b)*t };
    return c;
}
static void setpx(int x, int y, Colorf c) {
    if (x<0||x>=LAT||y<0||y>=ALT) return;
    size_t i = (size_t)(y*LAT+x)*3;
    img[i]   = (unsigned char)(clampd(c.r, 0, 1) * 255);
    img[i+1] = (unsigned char)(clampd(c.g, 0, 1) * 255);
    img[i+2] = (unsigned char)(clampd(c.b, 0, 1) * 255);
}
static Colorf getpx(int x, int y) {
    Colorf c = {0,0,0};
    if (x<0||x>=LAT||y<0||y>=ALT) return c;
    size_t i = (size_t)(y*LAT+x)*3;
    c.r = img[i]/255.0; c.g = img[i+1]/255.0; c.b = img[i+2]/255.0;
    return c;
}

/* Lumen a sinistra: illuminatio decrescens cum distantia ab eo
 * puncto, ubi (lx, ly) est origo lucis. Producit gradientem
 * chiaroscuri. */
static double lux_fact(double x, double y, double lx, double ly) {
    double dx = x - lx, dy = y - ly;
    double d = sqrt(dx*dx + dy*dy);
    double f = 1.0 / (1.0 + d * 0.012);
    /* directional: facies versus lumen clarior */
    return f;
}

/* Caput ovale: centrum (cx, cy), axes (a, b). Retro, oblique. */
static int in_capite(double x, double y, double cx, double cy,
                     double a, double b, double *dx_out, double *dy_out) {
    double dx = (x - cx) / a;
    double dy = (y - cy) / b;
    if (dx_out) *dx_out = dx;
    if (dy_out) *dy_out = dy;
    return dx*dx + dy*dy < 1.0;
}

/* Textura carnea: subtilis varietas rubri et carnationis. */
static double noise_simple(double x, double y) {
    double s = sin(x * 12.9898 + y * 78.233) * 43758.5453;
    return s - floor(s);
}

/* ============================================================
 * DEPINGERE
 * ============================================================ */

static void depingere(uint32_t seed) {
    (void)seed;
    Colorf fund_obsc = {0.04, 0.04, 0.06};
    Colorf fund_lene = {0.14, 0.13, 0.10};

    /* Fundum: gradiente de sinistro ad dextrum, obscurus. */
    for (int y = 0; y < ALT; y++) {
        for (int x = 0; x < LAT; x++) {
            double lf = lux_fact(x, y, -30, 30);
            Colorf c = mix(fund_obsc, fund_lene, clampd(lf * 2.0, 0, 1));
            setpx(x, y, c);
        }
    }

    /* Cervix et humerus: forma rudi. */
    Colorf vest_caer = {0.18, 0.22, 0.40};
    Colorf vest_obsc = {0.05, 0.08, 0.15};
    for (int y = 150; y < ALT; y++) {
        for (int x = 0; x < LAT; x++) {
            double cx = 128, cy = 200;
            double dx = (x - cx);
            double dy = (y - cy) * 2.0;
            double r = sqrt(dx * dx + dy * dy);
            double max_r = 75 + 35 * sin((x - 128) * 0.02);
            if (r < max_r) {
                double lf = lux_fact(x, y, 60, 100);
                double t = clampd(lf * 1.5, 0, 1);
                Colorf c = mix(vest_obsc, vest_caer, t);
                setpx(x, y, c);
            }
        }
    }

    /* Caput ovale. */
    double hx = 130, hy = 115;
    double ha = 45, hb = 58;
    Colorf carn_lux = {0.93, 0.82, 0.70};
    Colorf carn_med = {0.70, 0.55, 0.42};
    Colorf carn_umb = {0.22, 0.15, 0.10};

    for (int y = (int)(hy - hb - 4); y <= (int)(hy + hb + 4); y++) {
        for (int x = (int)(hx - ha - 4); x <= (int)(hx + ha + 4); x++) {
            double dx, dy;
            if (!in_capite(x, y, hx, hy, ha, hb, &dx, &dy)) continue;
            /* Simulare sphaerae illuminationem. */
            double r2 = dx*dx + dy*dy;
            double nz = sqrt(1.0 - r2);
            double nx = dx, ny = dy;
            /* Vector ad lumen */
            double lx = -0.7, ly = -0.5, lz = 0.5;
            double lnrm = sqrt(lx*lx + ly*ly + lz*lz);
            lx /= lnrm; ly /= lnrm; lz /= lnrm;
            double diff = nx * lx + ny * ly + nz * lz;
            if (diff < 0) diff = 0;
            double amb = 0.18;
            double lum = amb + 0.82 * diff;
            Colorf c;
            if (lum < 0.4) c = mix(carn_umb, carn_med, lum / 0.4);
            else c = mix(carn_med, carn_lux, (lum - 0.4) / 0.6);
            /* Textura minima: granulae pelliculae */
            double n = noise_simple(x * 0.7, y * 0.7) * 0.05 - 0.025;
            c.r += n; c.g += n * 0.8; c.b += n * 0.6;
            setpx(x, y, c);
        }
    }

    /* Turbans caeruleus et flavus: corona super caput. */
    Colorf turb_caer = {0.12, 0.20, 0.48};
    Colorf turb_lux  = {0.45, 0.55, 0.80};
    Colorf turb_fla  = {0.85, 0.75, 0.25};

    for (int y = 30; y < 120; y++) {
        for (int x = 60; x < 210; x++) {
            double dx = (x - hx) / (ha * 1.15);
            double dy = (y - (hy - 40)) / (hb * 0.75);
            double r = dx * dx + dy * dy;
            if (r > 1.0) continue;
            /* Ubi directe super caput (intra ovale), omitte. */
            double ddx, ddy;
            if (in_capite(x, y, hx, hy, ha, hb, &ddx, &ddy)) {
                /* super capite: tantum si dy < 0 (supra frontem) */
                if (ddy > -0.15) continue;
            }
            /* Segmentum flavum in parte dextra-superiore */
            int flavum = (x - hx) > 10 && (y - hy) < -30;
            double lf = lux_fact(x, y, 10, 40);
            double t = clampd(lf * 1.2, 0, 1);
            Colorf c;
            if (flavum) {
                c = mix(turb_caer, turb_fla, clampd(t + 0.3, 0, 1));
            } else {
                c = mix(turb_caer, turb_lux, t);
            }
            setpx(x, y, c);
        }
    }

    /* Umbra inter turbantem et cervicem. */
    for (int x = 60; x < 210; x++) {
        int y_band = 110;
        double lf = lux_fact(x, y_band, -30, 30);
        for (int dy = -2; dy <= 2; dy++) {
            int y = y_band + dy;
            if (y < 0 || y >= ALT) continue;
            Colorf c = getpx(x, y);
            c.r *= 0.6 + lf * 0.3;
            c.g *= 0.6 + lf * 0.3;
            c.b *= 0.6 + lf * 0.3;
            setpx(x, y, c);
        }
    }

    /* Oculi, nasus, labiae (simplicia signa). */
    Colorf ocul = {0.08, 0.06, 0.04};
    Colorf lab  = {0.65, 0.35, 0.30};
    Colorf refl = {0.95, 0.92, 0.85};
    /* Oculus sinister */
    for (int y = 108; y <= 112; y++)
        for (int x = 115; x <= 123; x++)
            if ((x - 119) * (x - 119) + (y - 110) * (y - 110) * 3 < 12)
                setpx(x, y, ocul);
    /* Oculus dexter */
    for (int y = 108; y <= 112; y++)
        for (int x = 140; x <= 148; x++)
            if ((x - 144) * (x - 144) + (y - 110) * (y - 110) * 3 < 12)
                setpx(x, y, ocul);
    /* Reflexus in oculis */
    setpx(118, 109, refl);
    setpx(143, 109, refl);
    /* Nasus: linea obscura lenis */
    for (int y = 115; y < 135; y++) {
        Colorf c = getpx(130, y);
        c.r *= 0.75; c.g *= 0.75; c.b *= 0.75;
        setpx(130, y, c);
        c = getpx(131, y);
        c.r *= 0.78; c.g *= 0.78; c.b *= 0.78;
        setpx(131, y, c);
    }
    /* Labiae */
    for (int y = 140; y <= 144; y++) {
        for (int x = 122; x <= 138; x++) {
            double dx = (x - 130) / 8.0;
            double dy = (y - 142) / 2.2;
            if (dx*dx + dy*dy < 1.0) {
                Colorf c = getpx(x, y);
                c = mix(c, lab, 0.7);
                setpx(x, y, c);
            }
        }
    }

    /* Margarita in aure dextra (ad 160, 135). */
    double mx = 163, my = 138;
    double mr = 4.5;
    Colorf marg_bas = {0.85, 0.85, 0.88};
    Colorf marg_lux = {1.0, 1.0, 0.98};
    Colorf marg_umb = {0.18, 0.18, 0.22};
    for (int y = (int)(my - mr - 1); y <= (int)(my + mr + 1); y++) {
        for (int x = (int)(mx - mr - 1); x <= (int)(mx + mr + 1); x++) {
            double dx = x - mx, dy = y - my;
            double r = sqrt(dx*dx + dy*dy);
            if (r < mr) {
                double nx = dx/mr, ny = dy/mr;
                double nz = sqrt(1 - nx*nx - ny*ny);
                double diff = -nx * 0.6 - ny * 0.5 + nz * 0.6;
                if (diff < 0) diff = 0;
                Colorf c;
                if (diff < 0.3) c = mix(marg_umb, marg_bas, diff / 0.3);
                else c = mix(marg_bas, marg_lux, (diff - 0.3) / 0.7);
                /* Reflexus altus specular */
                double spec = pow(clampd(-nx*0.6 - ny*0.5 + nz*0.6, 0, 1), 20);
                c.r += spec * 0.5; c.g += spec * 0.5; c.b += spec * 0.5;
                setpx(x, y, c);
            }
        }
    }

    /* Fenestra implicata: area lucis clarae in angulo sinistro
     * superiore (extra marginem imaginis suggesta). */
    for (int y = 10; y < 60; y++) {
        for (int x = 0; x < 50; x++) {
            double dx = x - 0;
            double dy = y - 30;
            double d = sqrt(dx*dx + dy*dy);
            double t = clampd(1.0 - d / 60.0, 0, 1);
            Colorf c = getpx(x, y);
            Colorf flv = {0.95, 0.88, 0.65};
            c = mix(c, flv, t * 0.4);
            setpx(x, y, c);
        }
    }
}

/* ============================================================
 * PPM + ASCII
 * ============================================================ */

static int scribere_ppm(const char *n) {
    FILE *f = fopen(n, "wb");
    if (!f) { fprintf(stderr, "vermerus: nequeo '%s'\n", n); return 1; }
    fprintf(f, "P6\n%d %d\n255\n", LAT, ALT);
    size_t m = fwrite(img, 1, (size_t)PIX * 3, f);
    fclose(f);
    return m == (size_t)PIX * 3 ? 0 : 1;
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
                    if (x < LAT && y < ALT) {
                        size_t i = (size_t)(y*LAT+x)*3;
                        tr += img[i]; tg += img[i+1]; tb += img[i+2]; ct++;
                    }
                    y = by + 4 + dy;
                    if (x < LAT && y < ALT) {
                        size_t i = (size_t)(y*LAT+x)*3;
                        br += img[i]; bg += img[i+1]; bb += img[i+2]; cb++;
                    }
                }
            if (!ct) ct = 1; if (!cb) cb = 1;
            int fg = rgb_ad_256(tr/ct, tg/ct, tb/ct);
            int bg_ = rgb_ad_256(br/cb, bg/cb, bb/cb);
            printf("\x1b[38;5;%dm\x1b[48;5;%dm\xe2\x96\x80", fg, bg_);
        }
        printf("\x1b[0m\n");
    }
}

static void auxilium(const char *n) {
    printf("Usus: %s [-h] [-o imago.ppm] [-s semen]\n", n);
    printf("\nPuella cum margarita, in stilo Vermeri (256x256).\n\n");
    printf("  -h         auxilium\n");
    printf("  -o f.ppm   scriptio PPM\n");
    printf("  -s N       semen (paene effectus nullus)\n");
}

int main(int argc, char **argv) {
    const char *out = NULL;
    uint64_t semen = SEMEN_DEFALTUM;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) { auxilium(argv[0]); return 0; }
        else if (!strcmp(argv[i], "-o")) {
            if (i+1 >= argc) { fprintf(stderr, "vermerus: -o?\n"); return 2; }
            out = argv[++i];
        } else if (!strcmp(argv[i], "-s")) {
            if (i+1 >= argc) { fprintf(stderr, "vermerus: -s?\n"); return 2; }
            semen = strtoull(argv[++i], NULL, 0); if (!semen) semen = 1;
        } else { fprintf(stderr, "vermerus: ignotum '%s'\n", argv[i]); return 2; }
    }
    depingere((uint32_t)semen);
    if (out) return scribere_ppm(out);
    reddere_ansi();
    return 0;
}
