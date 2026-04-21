/*
 * crusta.c — Computationes stress et motus crustalis. Vector (16 B)
 * per valorem: parvum HFA.
 */
#include "tectonica.h"
#include <math.h>

Vector
motus_relativus(Vector a, Vector b)
{
    Vector r = { a.u - b.u, a.v - b.v };
    return r;
}

double
stress_calc(Vector rel)
{
    /* hypothetica: stress proportionalis velocitate relativa */
    double v = sqrt(rel.u * rel.u + rel.v * rel.v);
    return 12.5 * v;  /* MPa per cm/anno */
}

double
kinetica_relativa(Vector a, Vector b)
{
    Vector r = motus_relativus(a, b);
    return sqrt(r.u * r.u + r.v * r.v);
}
