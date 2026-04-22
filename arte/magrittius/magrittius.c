/*
 * magrittius.c — "Hoc non est pomum" (René Magritte)
 *
 * Generat imaginem 256x256 in stilo Magrittiano: caelum nubilum
 * tranquillum, pomum viride pendens ante ovalem formam auream,
 * super horizonte supra aquam tranquillam. Surrealismi
 * apparens simplicitas sed significatio abstrusa.
 *
 * Technica: sphaera pomi cum illuminatione lambertiana simulata,
 * gradientes caeli, nubes procedurales per valorem-noise bilinearem.
 * Nulla libraria externa.
 *
 * Invocatio:
 *   magrittius             — reddit ASCII in stdout (gradus lucis)
 *   magrittius -o f.ppm    — scribit P6 PPM
 *   magrittius -s N        — semen nubium
 *   magrittius -h          — auxilium
 *
 * cc -std=c99 -Wall -Wextra -pedantic magrittius.c -o magrittius -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define LAT 256
#define ALT 256
#define PIX (LAT * ALT)
#define SEMEN_DEFALTUM 0xA661ECE1017ULL

typedef struct { unsigned char r, g, b; } Color;
typedef struct { double r, g, b; } Colorf;

static unsigned char img[PIX * 3];
static uint64_t rng_s;

static uint64_t xs(void) { uint64_t x = rng_s; x^=x<<13; x^=x>>7; x^=x<<17; rng_s=x; return x; }
static double runit(void) { return (double)(xs() >> 11) / (double)(1ULL << 53); }

/* Valorem-noise bidimensionale simplicissimum: hash-gradientes
 * in gradibus integris, interpolatio bilinearis cum curva levi. */
static uint32_t hash2(int x, int y, uint32_t seed) {
    uint32_t h = (uint32_t)x * 374761393u + (uint32_t)y * 668265263u + seed * 2246822519u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}
static double grad_val(int x, int y, uint32_t seed) {
    return (double)hash2(x, y, seed) / 4294967295.0;
}
static double smoothf(double t) { return t * t * (3.0 - 2.0 * t); }

static double value_noise(double x, double y, uint32_t seed) {
    int x0 = (int)floor(x), y0 = (int)floor(y);
    double fx = x - x0, fy = y - y0;
    double v00 = grad_val(x0,   y0,   seed);
    double v10 = grad_val(x0+1, y0,   seed);
    double v01 = grad_val(x0,   y0+1, seed);
    double v11 = grad_val(x0+1, y0+1, seed);
    double sx = smoothf(fx), sy = smoothf(fy);
    double a = v00 * (1 - sx) + v10 * sx;
    double b = v01 * (1 - sx) + v11 * sx;
    return a * (1 - sy) + b * sy;
}

static double fbm(double x, double y, uint32_t seed, int oct) {
    double sum = 0.0, amp = 0.5, freq = 1.0;
    for (int i = 0; i < oct; i++) {
        sum += amp * value_noise(x * freq, y * freq, seed + (uint32_t)i * 131u);
        amp *= 0.5; freq *= 2.0;
    }
    return sum;
}

static void setpxf(int x, int y, Colorf c) {
    if (x < 0 || x >= LAT || y < 0 || y >= ALT) return;
    double r = c.r, g = c.g, b = c.b;
    if (r < 0) r = 0; if (r > 1) r = 1;
    if (g < 0) g = 0; if (g > 1) g = 1;
    if (b < 0) b = 0; if (b > 1) b = 1;
    size_t i = (size_t)(y * LAT + x) * 3;
    img[i] = (unsigned char)(r * 255.0);
    img[i+1] = (unsigned char)(g * 255.0);
    img[i+2] = (unsigned char)(b * 255.0);
}

/* ============================================================
 * PICTURA
 * ============================================================ */

static Colorf lerp(Colorf a, Colorf b, double t) {
    Colorf c;
    c.r = a.r + (b.r - a.r) * t;
    c.g = a.g + (b.g - a.g) * t;
    c.b = a.b + (b.b - a.b) * t;
    return c;
}

static double clamp(double x, double lo, double hi) {
    return x < lo ? lo : (x > hi ? hi : x);
}

static void depingere(uint32_t seed) {
    /* Horizonte y = 150 (0 est sursum). */
    int horizon = 150;

    /* Caelum superior: caeruleum clarum ad caeruleum album prope
     * horizontem. Nubes sparsae. */
    Colorf cae_sup = {0.30, 0.48, 0.75};
    Colorf cae_inf = {0.80, 0.85, 0.92};
    Colorf nub_lux = {1.00, 1.00, 0.98};
    Colorf nub_umb = {0.60, 0.63, 0.70};

    /* Aqua inferior: caeruleum profundum ad speculum. */
    Colorf aqua_sup = {0.25, 0.38, 0.55};
    Colorf aqua_inf = {0.08, 0.15, 0.28};

    for (int y = 0; y < ALT; y++) {
        for (int x = 0; x < LAT; x++) {
            Colorf c;
            if (y < horizon) {
                double t = (double)y / horizon;
                c = lerp(cae_sup, cae_inf, t);
                /* nubes */
                double n = fbm((x + 0.5) * 0.03, (y + 0.5) * 0.05, seed, 5);
                double m = fbm((x + 17.3) * 0.012, (y + 5.1) * 0.02, seed ^ 0x1234u, 4);
                double nv = clamp((n + m - 0.7) * 2.5, 0.0, 1.0);
                Colorf nube = lerp(nub_umb, nub_lux, clamp((n - 0.3) * 2.0, 0.0, 1.0));
                c = lerp(c, nube, nv * 0.85);
            } else {
                double t = (double)(y - horizon) / (ALT - horizon);
                c = lerp(aqua_sup, aqua_inf, t);
                /* reflexio caeli in aqua cum undis */
                double wv = 0.02 * sin((x + 0.5) * 0.15 + (y - horizon) * 0.3);
                wv += 0.008 * sin((x + 0.5) * 0.4 - (y - horizon) * 0.2);
                int ry = horizon - (y - horizon);
                ry += (int)(wv * 40);
                if (ry >= 0 && ry < horizon) {
                    double tr = (double)ry / horizon;
                    Colorf refl = lerp(cae_sup, cae_inf, tr);
                    c = lerp(c, refl, 0.35 * (1.0 - t * 0.6));
                }
            }
            setpxf(x, y, c);
        }
    }

    /* Pomum viride pendens. Sphaera simulata cum illuminatione. */
    double cx = 128.0, cy = 118.0;
    double radius = 38.0;
    Colorf pom_cor = {0.35, 0.58, 0.22};   /* viride Grannii */
    Colorf pom_lux = {0.85, 0.95, 0.55};
    Colorf pom_umb = {0.10, 0.22, 0.08};

    for (int y = (int)(cy - radius - 4); y <= (int)(cy + radius + 8); y++) {
        for (int x = (int)(cx - radius - 4); x <= (int)(cx + radius + 4); x++) {
            if (x < 0 || x >= LAT || y < 0 || y >= ALT) continue;
            double dx = x - cx, dy = y - cy;
            /* Pomum non perfecte sphaericum: leniter oblongum. */
            double dyd = dy * 0.95;
            double r2 = dx * dx + dyd * dyd;
            double r = radius;
            if (r2 < r * r) {
                /* Normal de sphaerula. */
                double z = sqrt(r * r - r2);
                double len = sqrt(r * r);
                double nx = dx / len, ny = dyd / len, nz = z / len;
                /* Lux de sinistro-supero. */
                double lx = -0.5, ly = -0.7, lz = 0.5;
                double ln = sqrt(lx*lx + ly*ly + lz*lz);
                lx /= ln; ly /= ln; lz /= ln;
                double diff = nx * lx + ny * ly + nz * lz;
                if (diff < 0) diff = 0;
                double amb = 0.25;
                double lum = amb + 0.75 * diff;
                /* Specular simplex. */
                double spec = pow(clamp(nx*(-lx) + ny*(-ly) + nz*lz, 0.0, 1.0), 8.0);
                Colorf base = lerp(pom_umb, pom_lux, lum);
                base = lerp(base, pom_cor, 0.4);
                base.r += spec * 0.3;
                base.g += spec * 0.3;
                base.b += spec * 0.3;
                setpxf(x, y, base);

                /* stipes parvus supra. */
                if (dy < -radius * 0.85 && fabs(dx) < 3) {
                    Colorf stip = {0.30, 0.20, 0.10};
                    setpxf(x, y, stip);
                }
            }
            /* Umbra sub pomo super aquam. */
            {
                double sx = cx;
                double sy = 180.0;
                double sdx = x - sx;
                double sdy = (y - sy) * 3.0;
                double sd = sqrt(sdx * sdx + sdy * sdy);
                if (sd < radius * 1.1 && y > horizon - 5 && y < horizon + 30) {
                    double t = clamp(1.0 - sd / (radius * 1.1), 0.0, 1.0);
                    size_t idx = (size_t)(y * LAT + x) * 3;
                    Colorf here = {img[idx] / 255.0, img[idx+1] / 255.0, img[idx+2] / 255.0};
                    Colorf dark = {here.r * 0.5, here.g * 0.5, here.b * 0.55};
                    setpxf(x, y, lerp(here, dark, t * 0.6));
                }
            }
        }
    }

    /* Forma ovalis ante pomum (tenuis aurea): lineamentum Magrittianum. */
    double fcx = 128.0, fcy = 128.0;
    double fa = 95.0, fb = 110.0;
    double crass = 3.5;
    Colorf cadrum = {0.75, 0.60, 0.25};
    for (int y = 0; y < ALT; y++) {
        for (int x = 0; x < LAT; x++) {
            double dx = (x - fcx) / fa;
            double dy = (y - fcy) / fb;
            double r = dx * dx + dy * dy;
            double inner = 1.0 - crass / ((fa + fb) * 0.5);
            if (r < 1.0 && r > inner * inner) {
                setpxf(x, y, cadrum);
            }
        }
    }
}

/* ============================================================
 * PPM ET ASCII
 * ============================================================ */

static int scribere_ppm(const char *nomen) {
    FILE *f = fopen(nomen, "wb");
    if (!f) { fprintf(stderr, "magrittius: nequeo '%s'\n", nomen); return 1; }
    fprintf(f, "P6\n%d %d\n255\n", LAT, ALT);
    size_t n = fwrite(img, 1, (size_t)PIX * 3, f);
    fclose(f);
    return n == (size_t)PIX * 3 ? 0 : 1;
}

static void reddere_ascii(void) {
    const char *ramp = " .`:-+*#%@";
    int nramp = (int)strlen(ramp);
    int cols = 128, rows = 64;
    double sx = (double)LAT / cols, sy = (double)ALT / rows;
    for (int cy = 0; cy < rows; cy++) {
        for (int cx = 0; cx < cols; cx++) {
            int x = (int)(cx * sx), y = (int)(cy * sy);
            size_t i = (size_t)(y * LAT + x) * 3;
            int lum = (img[i] * 30 + img[i+1] * 59 + img[i+2] * 11) / 100;
            int k = lum * nramp / 256;
            if (k < 0) k = 0; if (k >= nramp) k = nramp - 1;
            putchar(ramp[k]);
        }
        putchar('\n');
    }
    printf("(Hoc non est pomum.)\n");
}

static void auxilium(const char *nomen) {
    printf("Usus: %s [-h] [-o imago.ppm] [-s semen]\n", nomen);
    printf("\nPictura Magrittiana 256x256: pomum viride pendens\n");
    printf("ante formam ovalem super mare tranquillum.\n\n");
    printf("  -h         hoc auxilium\n");
    printf("  -o f.ppm   scriptio PPM\n");
    printf("  -s N       semen nubium\n");
}

int main(int argc, char **argv) {
    const char *out = NULL;
    uint64_t semen = SEMEN_DEFALTUM;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) { auxilium(argv[0]); return 0; }
        else if (!strcmp(argv[i], "-o")) {
            if (i+1 >= argc) { fprintf(stderr, "magrittius: -o?\n"); return 2; }
            out = argv[++i];
        } else if (!strcmp(argv[i], "-s")) {
            if (i+1 >= argc) { fprintf(stderr, "magrittius: -s?\n"); return 2; }
            semen = strtoull(argv[++i], NULL, 0); if (!semen) semen = 1;
        } else { fprintf(stderr, "magrittius: ignotum '%s'\n", argv[i]); return 2; }
    }
    rng_s = semen;
    (void)runit;
    depingere((uint32_t)semen);
    if (out) return scribere_ppm(out);
    reddere_ascii();
    return 0;
}
