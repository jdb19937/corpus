#ifndef MONTES_H
#define MONTES_H

#include <stdint.h>

#define GRID_N 8

/*
 * Cella — pars grid cum float+float+int16+int16 (12 B). Parva struct
 * quae in uno register passari potest (SSE/integer packing ABI).
 */
typedef struct {
    float   alt_m;
    float   slope;
    int16_t cat;
    int16_t flux;
} Cella;

/*
 * Gradiens — par floats (8 B) — probat single-register HFA/packing.
 */
typedef struct {
    float dx;
    float dy;
} Gradiens;

/*
 * MonsData — struct MAGNISSIMA cum grid array embedded (~800 B).
 * Semper per pointer transmittitur; praeterea continet nomen et
 * meta floats.
 */
typedef struct {
    char    nomen[24];
    double  lat;
    double  lon;
    double  altitudo_max_m;
    int32_t ordo_prominentiae;
    Cella   grid[GRID_N][GRID_N];
} MonsData;

/* terrenum.c */
extern MonsData montes[];
extern const int N_MONTIUM;
void     terrenum_init(void);

/* elevatio.c */
float    interp_bilin(const MonsData *m, float u, float v);
Cella    cella_mediana(const MonsData *m);  /* parvam struct per val */
double   altitudo_summa(const MonsData *m);

/* gradientes.c */
Gradiens grad_in(const MonsData *m, int i, int j);
double   slope_max(const MonsData *m);
Gradiens grad_medianum(const MonsData *m);

#endif
