/*
 * coordinatae.c — Operationes coordinatarum et applicator per
 * function pointer.
 *
 * applic_each invocat LocusFn indirect cum Locus per valorem —
 * probat ABI-consistentiam inter direct et indirect calls pro
 * structis aggregatis.
 */
#include "navigatio.h"
#include <math.h>

double
ad_latitudinem(Locus p)
{
    return p.lat;
}

double
ad_longitudinem(Locus p)
{
    return p.lon;
}

Locus
medium_geodesicum(Locus a, Locus b)
{
    double la1  = a.lat * M_PI / 180.0;
    double la2  = b.lat * M_PI / 180.0;
    double lo1  = a.lon * M_PI / 180.0;
    double lo2  = b.lon * M_PI / 180.0;
    double dlon = lo2 - lo1;
    double Bx   = cos(la2) * cos(dlon);
    double By   = cos(la2) * sin(dlon);
    double lat_m = atan2(
        sin(la1) + sin(la2),
        sqrt((cos(la1) + Bx) * (cos(la1) + Bx) + By * By)
    );
    double lon_m = lo1 + atan2(By, cos(la1) + Bx);
    Locus r = { lat_m * 180.0 / M_PI, lon_m * 180.0 / M_PI };
    return r;
}

double
applic_each(LocusFn fn, const Portus *arr, int n)
{
    double s = 0.0;
    for (int i = 0; i < n; i++) {
        s += fn(arr[i].loc);
    }
    return s;
}
