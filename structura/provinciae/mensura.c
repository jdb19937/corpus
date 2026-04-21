/*
 * mensura.c — Distantiae et anguli in sphaera (approximatio Terrae).
 * Punctum per valorem — parvum HFA.
 */
#include "provinciae.h"
#include <math.h>

#define R_TERRA_KM 6371.0

static double rad(double d) { return d * M_PI / 180.0; }
static double deg(double r) { return r * 180.0 / M_PI; }

double
haversine_km(Punctum a, Punctum b)
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
bearing_grad(Punctum a, Punctum b)
{
    double dlon = rad(b.lon - a.lon);
    double la1  = rad(a.lat);
    double la2  = rad(b.lat);
    double y    = sin(dlon) * cos(la2);
    double x    = cos(la1) * sin(la2) - sin(la1) * cos(la2) * cos(dlon);
    double br   = deg(atan2(y, x));
    if (br < 0.0) br += 360.0;
    return br;
}
