/*
 * atmosphaera.c — Modellum densitatis atmosphaericae (exponentiale)
 * et accelerationis gravitatis.
 */
#include "meteora.h"
#include <math.h>

#define R_TERRA 6371000.0
#define G_SURFACE 9.80665

double
densitas_aeris(double altitudo_m)
{
    if (altitudo_m < 0.0)
        altitudo_m = 0.0;
    double H = 8500.0;
    return 1.225 * exp(-altitudo_m / H);
}

double
g_accel(double altitudo_m)
{
    double r = R_TERRA + altitudo_m;
    return G_SURFACE * (R_TERRA * R_TERRA) / (r * r);
}
