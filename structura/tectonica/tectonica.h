#ifndef TECTONICA_H
#define TECTONICA_H

#include <stdint.h>

/*
 * Vector — par doubles (16 B) — register-class HFA in multis ABIs.
 */
typedef struct {
    double u;
    double v;
} Vector;

/*
 * Placa — MIXED int/float/bitfield struct. Compilatoris ABI debet
 * tractare mixt-class (int + float) classificationem.
 *
 * Totum ~56 B — plerumque memoria-transit.
 */
typedef struct {
    char      nomen[24];
    int32_t   id;
    uint32_t  flags;
    unsigned  tectonicus : 2;   /* 0=continens, 1=oceanus, 2=transitio */
    unsigned  activus    : 1;
    unsigned  reserved   : 29;
    Vector    motus_cm_annum;    /* vector motus in cm/anno */
    double    area_km2;
} Placa;

typedef struct {
    int32_t  a;
    int32_t  b;
    double   stress_MPa;
    double   velocity_rel;
} Collisio;

/* placae.c */
extern const Placa placae[];
extern const int   N_PLACARUM;

/* collisio.c */
Collisio eval_collisio(Placa pa, Placa pb);   /* magna args per val */
int      collisio_n(const Placa *p, int n, Collisio *out, int cap);

/* crusta.c */
double   stress_calc(Vector rel);
double   kinetica_relativa(Vector a, Vector b);
Vector   motus_relativus(Vector a, Vector b);

#endif
