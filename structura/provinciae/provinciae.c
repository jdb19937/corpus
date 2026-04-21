/*
 * provinciae.c — Computationes super regiones geographicas.
 *
 * Exercet ABI pro structis MAGNIS (Regio ~300 bytes) per valorem
 * transmissis — compilator debet per hidden pointer / memoria
 * transferre, quod generatoribus codicis frequenter molestum est.
 */
#include "provinciae.h"
#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
    srand(11);

    printf("=== Provinciae ===\n");
    printf("(N = %d)\n\n", N_REGIONUM);

    for (int i = 0; i < N_REGIONUM; i++) {
        /* copia complete per valorem — probat large-struct copy */
        Mensurae m = poly_mensurae(regiones[i]);
        double a   = poly_area_signata(regiones[i]);

        fprintf(stderr, "[%s] a_sig=%.6f area=%.6f peri=%.6f\n",
                regiones[i].nomen, a, m.area_km2, m.perimetrum_km);

        printf("%-14s  n=%2d  area=%.0f km2  peri=%.0f km  pop=%.1f\n",
               regiones[i].nomen, regiones[i].n,
               m.area_km2, m.perimetrum_km, regiones[i].pop_millionum);
        printf("  centroid (lon=%.1f, lat=%.1f)\n",
               m.centroid.lon, m.centroid.lat);
    }

    /* distantia inter centroidos primarum duarum */
    if (N_REGIONUM >= 2) {
        Punctum c0 = poly_centroid(&regiones[0]);
        Punctum c1 = poly_centroid(&regiones[1]);
        printf("\nd(centroid[0], centroid[1]) = %.0f km\n",
               haversine_km(c0, c1));
        printf("bearing[0->1] = %.0f grad\n",
               bearing_grad(c0, c1));
    }

    return 0;
}
