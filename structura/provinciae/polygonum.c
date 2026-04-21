/*
 * polygonum.c — Geometria polygonorum.
 *
 * poly_area_signata accipit Regio per VALOREM (~300 bytes) —
 * compilator debet struct transferre per hidden pointer in stack.
 * poly_centroid accipit per pointer pro comparatione.
 * poly_mensurae reddit Mensurae (3 doubles + embedded Punctum) per
 * valorem — struct mediocris, probat multi-register return.
 */
#include "provinciae.h"
#include <math.h>

double
poly_area_signata(Regio r)
{
    double s = 0.0;
    for (int i = 0; i < r.n; i++) {
        int j = (i + 1) % r.n;
        s += r.vert[i].lon * r.vert[j].lat;
        s -= r.vert[j].lon * r.vert[i].lat;
    }
    return 0.5 * s;
}

Punctum
poly_centroid(const Regio *r)
{
    double sx = 0.0, sy = 0.0;
    for (int i = 0; i < r->n; i++) {
        sx += r->vert[i].lon;
        sy += r->vert[i].lat;
    }
    Punctum c = { sx / r->n, sy / r->n };
    return c;
}

double
poly_perimetrum(const Regio *r)
{
    double s = 0.0;
    for (int i = 0; i < r->n; i++) {
        int j = (i + 1) % r->n;
        s += haversine_km(r->vert[i], r->vert[j]);
    }
    return s;
}

Mensurae
poly_mensurae(Regio r)
{
    double lat0 = 0.0;
    for (int i = 0; i < r.n; i++) lat0 += r.vert[i].lat;
    lat0 /= r.n;

    /* area in km^2: scale degrees by km-per-degree at centroid lat */
    double km_per_lat = 111.0;
    double km_per_lon = 111.0 * cos(lat0 * M_PI / 180.0);
    double a_deg      = poly_area_signata(r);
    double area_km2   = fabs(a_deg) * km_per_lat * km_per_lon;

    Mensurae m;
    m.area_km2      = area_km2;
    m.centroid      = poly_centroid(&r);
    m.perimetrum_km = poly_perimetrum(&r);
    return m;
}
