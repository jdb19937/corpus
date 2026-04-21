/*
 * montes.c — Analyses grid elevationis montium famosorum.
 *
 * Exercet: Cella (12 B) et Gradiens (8 B) parvae per valorem reddi
 * (probat register-packing ABI), MonsData magna per pointer,
 * float arithmeticam (distinctum ab double ABI).
 */
#include "montes.h"
#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
    srand(67);
    terrenum_init();

    printf("=== Montes Maximi ===\n");
    printf("(N = %d, grid = %dx%d)\n\n", N_MONTIUM, GRID_N, GRID_N);

    for (int i = 0; i < N_MONTIUM; i++) {
        const MonsData *m = &montes[i];
        Cella med      = cella_mediana(m);
        Gradiens g     = grad_medianum(m);
        double sum     = altitudo_summa(m);
        double smax    = slope_max(m);

        fprintf(
            stderr, "[%s] sum=%.6f smax=%.6f g=(%.6f,%.6f)\n",
            m->nomen, sum, smax, g.dx, g.dy
        );

        printf(
            "%-14s (lat=%.1f, lon=%.1f)  max=%.0f m  ord=%d\n",
            m->nomen, m->lat, m->lon, m->altitudo_max_m,
            m->ordo_prominentiae
        );
        printf(
            "  mediana: alt=%.0f  slope=%.1f  cat=%d\n",
            (double)med.alt_m, (double)med.slope, med.cat
        );
        printf("  slope_max=%.1f  sum=%.0f\n", smax, sum / 1000.0);
    }

    /* interpolatio bilinearis — probat float ABI */
    const MonsData *m0 = &montes[0];
    float h        = interp_bilin(m0, 0.5f, 0.5f);
    printf("\ninterp[%s](0.5,0.5) = %.0f\n", m0->nomen, (double)h);

    return 0;
}
