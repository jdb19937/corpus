/*
 * navigatio.c — Magnus-circuli navigatio inter portus mundi.
 *
 * Exercet: Iter (72 B) per valorem retornatus — memoria-pass ABI;
 * Locus (16 B) argumenta — HFA classic; Vector2 (8 B, 2 floats)
 * — probat diversum float HFA; function pointer calls — typus
 * LocusFn probat ABI-consistency trans indirect calls.
 */
#include "navigatio.h"
#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
    srand(79);

    printf("=== Navigatio Maritima ===\n");
    printf("(N portuum = %d)\n\n", N_PORTUUM);

    const Portus *a = portus_quaere("Lisboa");
    const Portus *b = portus_quaere("Havana");
    const Portus *c = portus_quaere("Sydney");
    const Portus *d = portus_quaere("Tokyo");

    struct { const Portus *x, *y; } itinera[] = {
        { a, b },
        { b, c },
        { c, d },
        { a, d },
    };

    for (int i = 0; i < 4; i++) {
        Iter it = cursus_solve(itinera[i].x->loc, itinera[i].y->loc, 20.0);
        fprintf(
            stderr, "[%s->%s] d=%.6f b0=%.6f bf=%.6f t=%.6f\n",
            itinera[i].x->nomen, itinera[i].y->nomen,
            it.distantia_km, it.bearing_init_grad,
            it.bearing_fin_grad, it.duratio_horae
        );
        printf(
            "  %-8s -> %-8s  d=%.0f km  b0=%.0f  t=%.0f h\n",
            itinera[i].x->nomen, itinera[i].y->nomen,
            it.distantia_km, it.bearing_init_grad, it.duratio_horae
        );

        Vector2 ned = velocitas_ned(20.0, it.bearing_init_grad);
        printf(
            "    v_ned = (N=%.1f, E=%.1f) kn\n",
            (double)ned.n, (double)ned.e
        );
    }

    /* Function pointer: probat indirect call ABI cum struct arg.
     * Resultatum ad stderr ne output nondeterministicum esset si
     * compilator hoc incorrecte tractat. */
    double sum_lat = applic_each(ad_latitudinem, portus_tabula, N_PORTUUM);
    double sum_lon = applic_each(ad_longitudinem, portus_tabula, N_PORTUUM);
    fprintf(stderr, "sum lat = %.6f\n", sum_lat);
    fprintf(stderr, "sum lon = %.6f\n", sum_lon);
    printf("\n(applic_each vocata %d vicibus)\n", N_PORTUUM);

    Locus mid = medium_geodesicum(a->loc, d->loc);
    fprintf(stderr, "medium(Lisboa,Tokyo) = (%.6f, %.6f)\n", mid.lat, mid.lon);
    printf("medium_geodesicum vocatus\n");

    return 0;
}
