/*
 * magnitudo.c — Mathematica magnitudinis stellarum. Exercet
 * pow/log10 et reditus double.
 */
#include "stellae.h"
#include <math.h>

double
mag_absoluta(double m_app, double parallax_mas)
{
    if (parallax_mas <= 0.0) return m_app;
    double dist_pc = 1000.0 / parallax_mas;
    return m_app - 5.0 * (log10(dist_pc) - 1.0);
}

double
luminositas_relativa(double m_abs)
{
    /* Mabs_sol approx = 4.83 */
    return pow(10.0, 0.4 * (4.83 - m_abs));
}

double
flux_ratio(double m1, double m2)
{
    return pow(10.0, 0.4 * (m2 - m1));
}

int
cmp_mag(const void *a, const void *b)
{
    const Stella *sa = (const Stella *)a;
    const Stella *sb = (const Stella *)b;
    if (sa->mag < sb->mag) return -1;
    if (sa->mag > sb->mag) return 1;
    return 0;
}

int
cmp_hd(const void *a, const void *b)
{
    const Stella *sa = (const Stella *)a;
    const Stella *sb = (const Stella *)b;
    if (sa->hd < sb->hd) return -1;
    if (sa->hd > sb->hd) return 1;
    return 0;
}
