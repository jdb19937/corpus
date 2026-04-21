#ifndef PROVINCIAE_H
#define PROVINCIAE_H

#include <stdint.h>

#define MAX_VERTICES 16

/* Punctum — parvum (2 doubles); HFA classis. */
typedef struct {
    double lon;
    double lat;
} Punctum;

/*
 * Regio — struct MAGNA cum embedded array (16 * 16 = 256 bytes +
 * metadata), passing per valorem forcit memoria-transit ABI.
 */
typedef struct {
    char      nomen[32];
    int32_t   n;
    Punctum   vert[MAX_VERTICES];
    double    pop_millionum;
    int64_t   codex;
} Regio;

/* Resultatum — retornatus per valorem, struct mediocris. */
typedef struct {
    double area_km2;
    Punctum centroid;
    double perimetrum_km;
} Mensurae;

/* regiones.c */
extern const Regio regiones[];
extern const int   N_REGIONUM;

/* polygonum.c */
double    poly_area_signata(Regio r);        /* struct magna per valorem */
Punctum   poly_centroid(const Regio *r);     /* per pointer - compara */
double    poly_perimetrum(const Regio *r);
Mensurae  poly_mensurae(Regio r);            /* magna in, mediocris out */

/* mensura.c */
double haversine_km(Punctum a, Punctum b);
double bearing_grad(Punctum a, Punctum b);

#endif
