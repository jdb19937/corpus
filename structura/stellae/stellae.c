/*
 * stellae.c — Catalogus stellarum clararum cum operationibus
 * magnitudinis et coordinatarum.
 *
 * Exercet: passing Stella (int64_t + 4 doubles + nested Coord) per
 * valorem, qsort cum function-pointer callback, reditus Coord (par
 * doubles) per valorem.
 */
#include "stellae.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(void)
{
    srand(29);

    printf("=== Catalogus Stellarum ===\n");
    printf("(N = %d)\n\n", N_STELLARUM);

    Stella arr[64];
    memcpy(arr, catalogus, N_STELLARUM * sizeof(Stella));

    qsort(arr, N_STELLARUM, sizeof(Stella), cmp_mag);

    printf("Tres clarissimae (secundum mag apparentem):\n");
    for (int i = 0; i < 3 && i < N_STELLARUM; i++) {
        Stella s = arr[i];
        double Mabs = mag_absoluta(s.mag, s.parallax_mas);
        double L    = luminositas_relativa(Mabs);
        fprintf(stderr, "  HD%lld mag=%.6f Mabs=%.6f L=%.6f\n",
                (long long)s.hd, s.mag, Mabs, L);
        printf("  HD%-7lld  m=%.1f  Mabs=%.1f  L/Lsol=%.1f\n",
               (long long)s.hd, s.mag, Mabs, L);
    }

    qsort(arr, N_STELLARUM, sizeof(Stella), cmp_hd);
    printf("\nPrima quattuor secundum HD:\n");
    for (int i = 0; i < 4 && i < N_STELLARUM; i++) {
        Stella s = arr[i];
        Coord g = ad_galacticas(s.pos);
        printf("  HD%-7lld  ra=%.1f  dec=%.1f  l=%.1f  b=%.1f\n",
               (long long)s.hd, s.pos.ra, s.pos.dec, g.ra, g.dec);
    }

    /* probat Coord return per valorem */
    Coord m = coord_medium(arr[0].pos, arr[1].pos);
    double ang = angulus_inter(arr[0].pos, arr[1].pos);
    printf("\ncoord_medium  -> (ra=%.2f, dec=%.2f)\n", m.ra, m.dec);
    printf("angulus_inter -> %.1f gradus\n", ang);

    return 0;
}
