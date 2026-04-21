/*
 * collisio.c — Detectio et quantificatio collisionum placarum.
 * eval_collisio accipit DUAS Placa per VALOREM — probat passing
 * multarum structarum magnarum in eadem vocatione.
 */
#include "tectonica.h"
#include <stdio.h>

Collisio
eval_collisio(Placa pa, Placa pb)
{
    Vector rel = motus_relativus(pa.motus_cm_annum, pb.motus_cm_annum);
    Collisio c;
    c .a            = pa.id;
    c .b            = pb.id;
    c .velocity_rel = kinetica_relativa(pa.motus_cm_annum, pb.motus_cm_annum);
    c .stress_MPa   = stress_calc(rel);
    return c;
}

int
collisio_n(const Placa *p, int n, Collisio *out, int cap)
{
    int k = 0;
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (k >= cap)
                return k;
            Collisio c = eval_collisio(p[i], p[j]);
            if (c.stress_MPa > 80.0) {
                out[k++] = c;
            }
        }
    }
    return k;
}
