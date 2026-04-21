#ifndef METEORA_H
#define METEORA_H

#include <stdint.h>
#include <stddef.h>

/*
 * Status — struct MAGNA cum 8 doubles + timestamp. Redditur per
 * valorem (probat large aggregate return via hidden pointer),
 * necnon accipitur sic.
 */
typedef struct {
    double t;        /* tempus (s)                 */
    double x, y, z;  /* positio (m)                */
    double vx, vy, vz; /* velocitas (m/s)          */
    double m;        /* massa currens (kg)         */
} Status;

/*
 * Compositio — union cum struct pro type-punning.
 * Probat ABI in contextu unionum et nested aggregates.
 */
typedef union {
    struct { float fe, ni, si, mg; } fr;
    double  packed[2];
    uint64_t raw;
} Compositio;

typedef struct {
    const char *nomen;
    Status      init;
    Compositio  comp;
    double      radius_init;  /* m */
} Meteorum;

/* atmosphaera.c */
double densitas_aeris(double altitudo_m);   /* kg/m^3 */
double g_accel(double altitudo_m);

/* traiectus.c */
Status step_rk2(Status s, double dt, double radius, double Cd);
Status integra(Status s0, double radius, double dt, int passus);

/* impactus.c */
double energia_kin(Status s);
double energia_TNT_ton(double joules);
void   rapport_v(const char *nom, int n, ...);  /* variadic */

/* meteora.c data */
extern const Meteorum meteora[];
extern const int      N_METEORUM;

#endif
