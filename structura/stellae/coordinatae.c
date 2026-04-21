/*
 * coordinatae.c — Transformationes coordinatarum caelestium.
 * Coord (2 doubles) per valorem: probat parvum HFA.
 */
#include "stellae.h"
#include <math.h>

static double deg(double rad) { return rad * 180.0 / M_PI; }
static double rad(double d)   { return d   * M_PI / 180.0; }

Coord
ad_galacticas(Coord eq)
{
    /* approximatio simplificata ad coordinatas galacticas */
    double ra_deg  = eq.ra * 15.0;
    double alpha_G = 192.85;
    double delta_G = 27.13;
    double l_NCP   = 122.93;

    double sd = sin(rad(eq.dec));
    double cd = cos(rad(eq.dec));
    double sg = sin(rad(delta_G));
    double cg = cos(rad(delta_G));
    double sb = sd * sg + cd * cg * cos(rad(ra_deg - alpha_G));
    double b  = asin(sb);
    double l  = l_NCP - atan2(
        cd * sin(rad(ra_deg - alpha_G)),
        sd * cg - cd * sg * cos(rad(ra_deg - alpha_G))
    );
    Coord r = { deg(l), deg(b) };
    return r;
}

double
angulus_inter(Coord a, Coord b)
{
    double ra1 = rad(a.ra * 15.0);
    double ra2 = rad(b.ra * 15.0);
    double d1  = rad(a.dec);
    double d2  = rad(b.dec);
    double c   = sin(d1) * sin(d2) + cos(d1) * cos(d2) * cos(ra1 - ra2);
    if (c > 1.0)
        c = 1.0;
    if (c < -1.0)
        c = -1.0;
    return deg(acos(c));
}

Coord
coord_medium(Coord a, Coord b)
{
    Coord r = { (a.ra + b.ra) * 0.5, (a.dec + b.dec) * 0.5 };
    return r;
}
