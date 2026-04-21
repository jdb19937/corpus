/*
 * tectonica.c — Analysis collisionum placarum tectonicarum.
 *
 * Exercet: struct MIXTA (int + double + bitfields) per valorem
 * (Placa), Vector (2 doubles) per valorem, Collisio retornatus
 * per valorem.
 */
#include "tectonica.h"
#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
    srand(53);

    printf("=== Placae Tectonicae ===\n");
    printf("(N = %d)\n\n", N_PLACARUM);

    for (int i = 0; i < N_PLACARUM; i++) {
        Placa p = placae[i];
        fprintf(
            stderr, "[%s] flags=%u tect=%u motus=(%.6f,%.6f) area=%.6f\n",
            p.nomen, p.flags, p.tectonicus,
            p.motus_cm_annum.u, p.motus_cm_annum.v, p.area_km2
        );
        const char *tipo = (p.tectonicus == 0) ? "cont" :
            (p.tectonicus == 1) ? "ocea" : "tran";
        printf(
            "  %-18s  id=%3d  %s  area=%.0f Mkm2  v=(%.1f,%.1f) cm/a\n",
            p.nomen, p.id, tipo, p.area_km2 / 1.0e6,
            p.motus_cm_annum.u, p.motus_cm_annum.v
        );
    }

    Collisio colls[32];
    int nc = collisio_n(placae, N_PLACARUM, colls, 32);
    printf("\nCollisiones significantes (%d):\n", nc);
    for (int k = 0; k < nc; k++) {
        printf(
            "  [%d<->%d]  stress=%.1f MPa  v_rel=%.1f\n",
            colls[k].a, colls[k].b,
            colls[k].stress_MPa, colls[k].velocity_rel
        );
    }

    return 0;
}
