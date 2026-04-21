#ifndef ORBES_H
#define ORBES_H

#include <stdint.h>

/*
 * Vec3 — vector in R^3 cum tribus componentibus floating-point.
 * Per valorem traditur/reditur ut ABI 64-bit exerceatur.
 */
typedef struct {
    double x, y, z;
} Vec3;

/*
 * Elementa orbitalia Kepleriana. Struct magna (6 doubles + int64_t)
 * quae per valorem vel referentiam transferri potest.
 */
typedef struct {
    double    a;      /* semiaxis maior (AU)           */
    double    e;      /* eccentricitas                 */
    double    i;      /* inclinatio (rad)              */
    double    omega;  /* argumentum perihelii (rad)    */
    double    Omega;  /* longitudo nodi ascendentis    */
    double    M0;     /* anomalia media initialis      */
    int64_t   id;     /* identificator planetae        */
} ElementaOrbis;

typedef struct {
    const char       *nomen;
    ElementaOrbis     orb;
    double            massa;   /* masses Terrae */
} Planeta;

/* planetae.c */
extern const Planeta planetae[];
extern const int     N_PLANETARUM;

/* vectores.c — operationes per valorem */
Vec3   vec_adde(Vec3 a, Vec3 b);
Vec3   vec_scala(Vec3 v, double s);
double vec_norma(Vec3 v);
Vec3   vec_rota_z(Vec3 v, double theta);

/* kepler.c */
double  kepler_solve(double M, double e);
Vec3    positio_in_orbe(ElementaOrbis orb, double t_anni);

#endif
