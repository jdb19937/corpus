/*
 * cursus.c — Computationes cursuum magni-circuli.
 *
 * cursus_solve reddit Iter (~72 B) per valorem — compilator per
 * hidden pointer hoc tractat. velocitas_ned reddit Vector2 (8 B)
 * — float HFA, diversus ABI ab double HFA.
 */
#include "navigatio.h"
#include <math.h>

#define R_TERRA_KM 6371.0

static double rad(double d) { return d * M_PI / 180.0; }
static double deg(double r) { return r * 180.0 / M_PI; }

double
distantia_km(Locus a, Locus b)
{
    double dlat = rad(b.lat - a.lat);
    double dlon = rad(b.lon - a.lon);
    double la1  = rad(a.lat);
    double la2  = rad(b.lat);
    double h    = sin(dlat / 2.0) * sin(dlat / 2.0)
        + cos(la1) * cos(la2) * sin(dlon / 2.0) * sin(dlon / 2.0);
    return 2.0 * R_TERRA_KM * asin(sqrt(h));
}

double
bearing_init(Locus a, Locus b)
{
    double dlon = rad(b.lon - a.lon);
    double la1  = rad(a.lat);
    double la2  = rad(b.lat);
    double y    = sin(dlon) * cos(la2);
    double x    = cos(la1) * sin(la2) - sin(la1) * cos(la2) * cos(dlon);
    double br   = deg(atan2(y, x));
    if (br < 0.0)
        br += 360.0;
    return br;
}

static double
bearing_final(Locus a, Locus b)
{
    Locus rev = { a.lat, a.lon };
    double br = bearing_init(b, rev) + 180.0;
    if (br >= 360.0)
        br -= 360.0;
    return br;
}

Iter
cursus_solve(Locus a, Locus b, double velocitas_knot)
{
    Iter it;
    it.seq               = 0;
    it.origo             = a;
    it.finis             = b;
    it.distantia_km      = distantia_km(a, b);
    it.bearing_init_grad = bearing_init(a, b);
    it.bearing_fin_grad  = bearing_final(a, b);
    double km_h = velocitas_knot * 1.852;
    it.duratio_horae = (km_h > 0.0) ? it.distantia_km / km_h : 0.0;
    return it;
}

Vector2
velocitas_ned(double vknot, double bearing_grad)
{
    double b = rad(bearing_grad);
    Vector2 r;
    r .n = (float)(vknot * cos(b));
    r .e = (float)(vknot * sin(b));
    return r;
}
