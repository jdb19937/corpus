#ifndef NAVIGATIO_H
#define NAVIGATIO_H

#include <stdint.h>

/* Locus — par doubles (16 B) cum lat/lon. Parva HFA. */
typedef struct {
    double lat;
    double lon;
} Locus;

/* Vector2 — par floats (8 B) — FLOAT HFA diversus ab double HFA. */
typedef struct {
    float n;  /* North component  */
    float e;  /* East  component  */
} Vector2;

/*
 * Iter — mixtum int + 4 doubles + nested Locus duo — ~ 72 B, memoria.
 * Redditur per valorem ex cursus_solve.
 */
typedef struct {
    int64_t  seq;
    Locus    origo;
    Locus    finis;
    double   distantia_km;
    double   bearing_init_grad;
    double   bearing_fin_grad;
    double   duratio_horae;
} Iter;

/* Portus — tabula portuum maritimorum. */
typedef struct {
    const char *nomen;
    Locus       loc;
    int32_t     codex;
} Portus;

/* portus.c */
extern const Portus portus_tabula[];
extern const int    N_PORTUUM;
const Portus       *portus_quaere(const char *nom);

/* cursus.c */
Iter     cursus_solve(Locus a, Locus b, double velocitas_knot);  /* magnum ret */
double   distantia_km(Locus a, Locus b);
double   bearing_init(Locus a, Locus b);
Vector2  velocitas_ned(double vknot, double bearing_grad);       /* parvum ret */

/* coordinatae.c — typus Callback probat ABI pro function pointers */
typedef double (*LocusFn)(Locus);
double      ad_latitudinem(Locus p);
double      ad_longitudinem(Locus p);
Locus       medium_geodesicum(Locus a, Locus b);
double      applic_each(LocusFn fn, const Portus *arr, int n);

#endif
