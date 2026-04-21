#ifndef GALAXIA_H
#define GALAXIA_H

#include <stdint.h>

/*
 * Vec3d — 3 doubles, passing/return per valorem, HFA probatio.
 */
typedef struct {
    double x, y, z;
} Vec3d;

/*
 * Corpus — struct mediocris (6 doubles + double m + int32 = ~64 B).
 * Necnon cum NESTED Vec3 membris, transferri per valorem.
 */
typedef struct {
    char    nomen[16];
    Vec3d   r;       /* positio (pc)       */
    Vec3d   v;       /* velocitas (km/s)   */
    double  m;       /* massa (Msun * 1e9) */
    int32_t indicium;
} Corpus;

/* corpora.c */
extern Corpus corpora[];
extern const int N_CORPORUM;

/* gravitas.c */
Vec3d  vis_in(const Corpus *bodies, int n, int i, double G);
double energia_total(const Corpus *bodies, int n, double G);
Vec3d  impetus_total(const Corpus *bodies, int n);

/* integrator.c */
void   leapfrog(Corpus *bodies, int n, double G, double dt, int passus);

/* helpers in gravitas.c */
Vec3d  v3_sub(Vec3d a, Vec3d b);
Vec3d  v3_add(Vec3d a, Vec3d b);
Vec3d  v3_mul(Vec3d v, double s);
double v3_dot(Vec3d a, Vec3d b);

#endif
