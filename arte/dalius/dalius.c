/*
 * dalius.c — Persistentia Memoriae (Salvador Dalí)
 *
 * Generat imaginem 256x256 in stilo Daliensi: campus desertus ad
 * horizontem longinquum, caelum crepusculum, et horologium
 * liquescens pendens super angulum mensae. Tempus defluxit.
 *
 * Horologium: discus parabolice deformatus secundum formulam
 *   y = y0 + sigma(theta, rho), ubi sigma defluit ab angulo
 *   ad directionem gravitatis, producens illam classicam
 *   imaginem defluentem.
 *
 * cc -std=c99 -Wall -Wextra -pedantic dalius.c -o dalius -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define LAT 256
#define ALT 256
#define PIX (LAT * ALT)
#define SEMEN_DEFALTUM 0xDA117E1729ULL

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct { double r, g, b; } Colorf;

static unsigned char img[PIX * 3];
static uint64_t rng;

static uint64_t xs(void) { uint64_t x = rng; x^=x<<13; x^=x>>7; x^=x<<17; rng=x; return x; }
static double runit(void) { return (double)(xs() >> 11) / (double)(1ULL << 53); }

static double clamp(double v, double lo, double hi) { return v<lo?lo:(v>hi?hi:v); }

static Colorf lerpc(Colorf a, Colorf b, double t) {
    Colorf c;
    c.r = a.r + (b.r - a.r) * t;
    c.g = a.g + (b.g - a.g) * t;
    c.b = a.b + (b.b - a.b) * t;
    return c;
}

static void setpx(int x, int y, Colorf c) {
    if (x < 0 || x >= LAT || y < 0 || y >= ALT) return;
    double r = clamp(c.r, 0, 1), g = clamp(c.g, 0, 1), b = clamp(c.b, 0, 1);
    size_t i = (size_t)(y * LAT + x) * 3;
    img[i] = (unsigned char)(r * 255);
    img[i+1] = (unsigned char)(g * 255);
    img[i+2] = (unsigned char)(b * 255);
}

static Colorf getpx(int x, int y) {
    Colorf c = {0, 0, 0};
    if (x < 0 || x >= LAT || y < 0 || y >= ALT) return c;
    size_t i = (size_t)(y * LAT + x) * 3;
    c.r = img[i] / 255.0;
    c.g = img[i+1] / 255.0;
    c.b = img[i+2] / 255.0;
    return c;
}

/* Hash/noise pro textura terrae. */
static uint32_t hash2(int x, int y) {
    uint32_t h = (uint32_t)x * 73856093u ^ (uint32_t)y * 19349663u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}
static double noise_at(double x, double y) {
    int xi = (int)floor(x), yi = (int)floor(y);
    double fx = x - xi, fy = y - yi;
    double n00 = (double)hash2(xi,   yi  ) / 4294967295.0;
    double n10 = (double)hash2(xi+1, yi  ) / 4294967295.0;
    double n01 = (double)hash2(xi,   yi+1) / 4294967295.0;
    double n11 = (double)hash2(xi+1, yi+1) / 4294967295.0;
    double sx = fx * fx * (3 - 2 * fx);
    double sy = fy * fy * (3 - 2 * fy);
    double a = n00 + (n10 - n00) * sx;
    double b = n01 + (n11 - n01) * sx;
    return a + (b - a) * sy;
}

/* ============================================================
 * FUNDUM: caelum crepusculum, desertus, mare longinquum
 * ============================================================ */

static void depingere_fundum(void) {
    Colorf caelum_alt = {0.10, 0.05, 0.15};  /* violaceum obscurum */
    Colorf caelum_med = {0.55, 0.35, 0.25};
    Colorf caelum_hor = {0.85, 0.60, 0.35};  /* aurum crepusculum */
    Colorf mare_pro = {0.15, 0.18, 0.25};
    Colorf mare_sup = {0.40, 0.40, 0.45};
    Colorf terra_um = {0.20, 0.12, 0.08};
    Colorf terra_lu = {0.65, 0.45, 0.25};

    int horizon = 170;
    int mare_end = 180;

    for (int y = 0; y < ALT; y++) {
        for (int x = 0; x < LAT; x++) {
            Colorf c;
            if (y < horizon) {
                /* caelum: tres zonae */
                double t = (double)y / horizon;
                if (t < 0.5) {
                    c = lerpc(caelum_alt, caelum_med, t * 2.0);
                } else {
                    c = lerpc(caelum_med, caelum_hor, (t - 0.5) * 2.0);
                }
            } else if (y < mare_end) {
                double t = (double)(y - horizon) / (mare_end - horizon);
                c = lerpc(mare_sup, mare_pro, t);
            } else {
                /* terra: longe clara, prope obscura */
                double t = (double)(y - mare_end) / (ALT - mare_end);
                c = lerpc(terra_lu, terra_um, t);
                /* textura granulosa */
                double n = noise_at(x * 0.3, y * 0.3);
                double n2 = noise_at(x * 0.08, y * 0.08);
                c.r *= 0.85 + 0.3 * n;
                c.g *= 0.85 + 0.25 * n;
                c.b *= 0.85 + 0.2 * n;
                c.r += n2 * 0.08;
                c.g += n2 * 0.05;
            }
            setpx(x, y, c);
        }
    }

    /* Montes longinqui trans horizontem. */
    Colorf mons = {0.30, 0.22, 0.30};
    for (int x = 0; x < LAT; x++) {
        double h = 8 * noise_at(x * 0.04, 100) + 4 * noise_at(x * 0.1, 200);
        int my = horizon - (int)h;
        for (int y = my; y < horizon; y++) {
            double t = (double)(y - my) / (double)(horizon - my + 1);
            Colorf c = lerpc(mons, mare_sup, t * 0.5);
            setpx(x, y, c);
        }
    }
}

/* ============================================================
 * MENSA (angulus cum umbra)
 * ============================================================ */

static void depingere_mensam(void) {
    Colorf mensa_sup = {0.40, 0.28, 0.18};
    Colorf mensa_lat = {0.22, 0.15, 0.10};
    /* Rectangulum perspectivum: (30, 200) sinistrum-superius,
     * (230, 220) dextrum-superius, (250, 255) dextrum-inferius,
     * (10, 255) sinistrum-inferius. */
    for (int y = 195; y < ALT; y++) {
        double t = (double)(y - 195) / (ALT - 195);
        int xl = (int)(30 - 20 * t);
        int xr = (int)(230 + 20 * t);
        for (int x = xl; x <= xr; x++) {
            if (x < 0 || x >= LAT) continue;
            Colorf c;
            if (y < 220) {
                c = lerpc(mensa_sup, mensa_lat, (y - 195) * 0.04);
            } else {
                c = mensa_lat;
                c.r *= 0.7 + 0.3 * cos(y * 0.2);
            }
            setpx(x, y, c);
        }
    }
}

/* ============================================================
 * HOROLOGIUM LIQUESCENS
 *
 * Circulus basalis: centrum (cx, cy), radius r.
 * Deformatio: pro unoquoque puncto in horologio, calculamus angulum
 * theta et radium rho. In spatio redditus, y coordinata defluit:
 *   y_defl = y + defluxus(theta, rho)
 * ubi defluxus est maximus ad theta proximum -pi/2 (inferior) et
 * decrescit ad peripheriam.
 * ============================================================ */

static double drip(double theta, double rho_frac, double dex) {
    /* defluxus maximus in directione dex (radianis). */
    double d = cos(theta - dex);
    if (d < 0) d = 0;
    return pow(d, 3.0) * (0.5 + 0.5 * rho_frac) * 35.0;
}

static int in_horo(double lx, double ly, double r, double dex_angle,
                   double *theta_out, double *rho_out) {
    /* Invertere: datis (lx, ly) in spatio redditus, invenire an
     * aliquod punctum (theta, rho) horologii huc mappetur. Difficile
     * exacte; tamen methodo directa (forward scan) utimur loco. */
    (void)lx; (void)ly; (void)r; (void)dex_angle;
    (void)theta_out; (void)rho_out;
    return 0;
}

static void depingere_horologium(double cx, double cy, double radius, double dex) {
    Colorf horo_corpus = {0.92, 0.85, 0.55};
    Colorf horo_limes = {0.35, 0.25, 0.15};
    Colorf horo_umbra = {0.60, 0.50, 0.30};
    Colorf horo_manus = {0.12, 0.08, 0.05};

    /* Methodus directa: pro unoquoque (theta, rho) puncto in disco,
     * computamus eius locum defluxum et pingimus. Alta densitate
     * punctorum utimur ad pixellos leniter implendos. */
    int samples_r = (int)(radius * 6);
    int samples_t = 720;
    for (int ir = 0; ir <= samples_r; ir++) {
        double rho = (double)ir / samples_r;
        double rad = rho * radius;
        for (int it = 0; it < samples_t; it++) {
            double theta = 2 * M_PI * it / samples_t;
            double x = cx + rad * cos(theta);
            double y = cy + rad * sin(theta);
            double dfy = drip(theta, rho, dex);
            y += dfy;
            /* etiam defluxus horizontalis levior */
            double dfx = drip(theta - M_PI/2, rho, dex) * 0.15;
            x += dfx;

            /* Color: limes ad rho = 1, corpus aliter. */
            Colorf c;
            if (rho > 0.94) {
                c = horo_limes;
            } else {
                /* Umbra: angulus ad sinistram. */
                double shade = 0.5 + 0.5 * cos(theta - M_PI * 0.75);
                c = lerpc(horo_umbra, horo_corpus, shade);
                /* luminositas minor prope marginem defluentem */
                if (dfy > 10) c.r *= 0.9;
            }
            int ix = (int)x, iy = (int)y;
            setpx(ix, iy, c);
            setpx(ix + 1, iy, c);
            setpx(ix, iy + 1, c);
            setpx(ix + 1, iy + 1, c);
        }
    }

    /* Numeri horologii (simpliciter, puncta in 12 positionibus horarum). */
    for (int k = 0; k < 12; k++) {
        double theta = 2 * M_PI * k / 12 - M_PI / 2;
        double rad = radius * 0.82;
        double x = cx + rad * cos(theta);
        double y = cy + rad * sin(theta);
        y += drip(theta, 0.82, dex);
        x += drip(theta - M_PI/2, 0.82, dex) * 0.15;
        Colorf tic = (k % 3 == 0) ? (Colorf){0.10, 0.08, 0.05} : (Colorf){0.25, 0.20, 0.10};
        for (int dx = -2; dx <= 2; dx++)
            for (int dy = -2; dy <= 2; dy++)
                if (dx*dx + dy*dy <= 4) setpx((int)x + dx, (int)y + dy, tic);
    }

    /* Manus horologii (hora circa 4:50): duae lineae rectae deformatae. */
    double a_hor = -M_PI / 2 + 2 * M_PI * (4.83 / 12.0);
    double a_min = -M_PI / 2 + 2 * M_PI * (50.0 / 60.0);
    /* Manus brevior (hora). */
    for (int i = 0; i < 200; i++) {
        double t = (double)i / 200;
        double rad = t * radius * 0.55;
        double theta = a_hor;
        double x = cx + rad * cos(theta);
        double y = cy + rad * sin(theta);
        y += drip(theta, t * 0.55, dex);
        x += drip(theta - M_PI/2, t * 0.55, dex) * 0.15;
        for (int dx = -1; dx <= 1; dx++)
            for (int dy = -1; dy <= 1; dy++)
                setpx((int)x + dx, (int)y + dy, horo_manus);
    }
    /* Manus longior (minuta). */
    for (int i = 0; i < 240; i++) {
        double t = (double)i / 240;
        double rad = t * radius * 0.78;
        double theta = a_min;
        double x = cx + rad * cos(theta);
        double y = cy + rad * sin(theta);
        y += drip(theta, t * 0.78, dex);
        x += drip(theta - M_PI/2, t * 0.78, dex) * 0.15;
        setpx((int)x, (int)y, horo_manus);
        setpx((int)x + 1, (int)y, horo_manus);
    }
    (void)in_horo;
}

/* ============================================================
 * FIGURA
 * ============================================================ */

static void depingere(uint32_t seed) {
    (void)seed;
    depingere_fundum();
    depingere_mensam();
    /* Horologium pendet super mensae marginem dextrum. */
    depingere_horologium(145.0, 190.0, 42.0, M_PI / 2);
    /* Alterum horologium, minus, flottans in longe. */
    depingere_horologium(60.0, 160.0, 18.0, M_PI / 2 + 0.3);
}

/* ============================================================
 * PPM ET ANSI
 * ============================================================ */

static int scribere_ppm(const char *nomen) {
    FILE *f = fopen(nomen, "wb");
    if (!f) { fprintf(stderr, "dalius: nequeo '%s'\n", nomen); return 1; }
    fprintf(f, "P6\n%d %d\n255\n", LAT, ALT);
    size_t n = fwrite(img, 1, (size_t)PIX * 3, f);
    fclose(f);
    return n == (size_t)PIX * 3 ? 0 : 1;
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
            unsigned long tr=0,tg=0,tb=0, br=0,bg=0,bb=0;
            int ct=0, cb=0;
            int bx = cx * 4, by = cy * 8;
            for (int dy = 0; dy < 4; dy++) {
                for (int dx = 0; dx < 4; dx++) {
                    int x = bx + dx, y = by + dy;
                    if (x < LAT && y < ALT) {
                        Colorf c = getpx(x, y);
                        tr += (unsigned long)(c.r * 255);
                        tg += (unsigned long)(c.g * 255);
                        tb += (unsigned long)(c.b * 255);
                        ct++;
                    }
                    int yb2 = by + 4 + dy;
                    if (x < LAT && yb2 < ALT) {
                        Colorf c = getpx(x, yb2);
                        br += (unsigned long)(c.r * 255);
                        bg += (unsigned long)(c.g * 255);
                        bb += (unsigned long)(c.b * 255);
                        cb++;
                    }
                }
            }
            if (ct == 0) ct = 1; if (cb == 0) cb = 1;
            int fg = rgb_ad_256(tr/ct, tg/ct, tb/ct);
            int bg_ = rgb_ad_256(br/cb, bg/cb, bb/cb);
            printf("\x1b[38;5;%dm\x1b[48;5;%dm\xe2\x96\x80", fg, bg_);
        }
        printf("\x1b[0m\n");
    }
}

static void auxilium(const char *n) {
    printf("Usus: %s [-h] [-o imago.ppm] [-s semen]\n", n);
    printf("\nHorologium liquescens super mensam deserti (256x256).\n\n");
    printf("  -h         hoc auxilium\n");
    printf("  -o f.ppm   P6 PPM\n");
    printf("  -s N       semen\n");
}

int main(int argc, char **argv) {
    const char *out = NULL;
    uint64_t semen = SEMEN_DEFALTUM;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) { auxilium(argv[0]); return 0; }
        else if (!strcmp(argv[i], "-o")) {
            if (i+1 >= argc) { fprintf(stderr, "dalius: -o?\n"); return 2; }
            out = argv[++i];
        } else if (!strcmp(argv[i], "-s")) {
            if (i+1 >= argc) { fprintf(stderr, "dalius: -s?\n"); return 2; }
            semen = strtoull(argv[++i], NULL, 0); if (!semen) semen = 1;
        } else { fprintf(stderr, "dalius: ignotum '%s'\n", argv[i]); return 2; }
    }
    rng = semen;
    (void)runit;
    depingere((uint32_t)semen);
    if (out) return scribere_ppm(out);
    reddere_ansi();
    return 0;
}
