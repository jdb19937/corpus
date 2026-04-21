/*
 * galaxia.c — Simulatio gravitationalis corporum galacticorum
 * (systema Local-Group reductum).
 *
 * Exercet: Vec3d return/argument (HFA 3 doubles), Corpus per pointer
 * cum nested structs, leapfrog integratio floating-point.
 */
#include "galaxia.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int
main(void)
{
    srand(41);
    double G = 4.498e-3;   /* unitates reductae */
    double dt = 0.001;
    int passus = 50;

    printf("=== Galaxia Localis ===\n");
    printf("(N = %d, passus = %d)\n\n", N_CORPORUM, passus);

    double E0 = energia_total(corpora, N_CORPORUM, G);
    Vec3d  p0 = impetus_total(corpora, N_CORPORUM);

    leapfrog(corpora, N_CORPORUM, G, dt, passus);

    double Ef = energia_total(corpora, N_CORPORUM, G);
    Vec3d  pf = impetus_total(corpora, N_CORPORUM);

    fprintf(stderr, "E0=%.9e Ef=%.9e\n", E0, Ef);
    fprintf(stderr, "p0=(%.9e, %.9e, %.9e)\n", p0.x, p0.y, p0.z);
    fprintf(stderr, "pf=(%.9e, %.9e, %.9e)\n", pf.x, pf.y, pf.z);

    printf("nomina corporum:\n");
    for (int i = 0; i < N_CORPORUM; i++) {
        Corpus *b = &corpora[i];
        Vec3d f = vis_in(corpora, N_CORPORUM, i, G);
        double fmag = sqrt(v3_dot(f, f));
        fprintf(stderr, "  [%s] r=(%.9f,%.9f,%.9f) |F|=%.9e\n",
                b->nomen, b->r.x, b->r.y, b->r.z, fmag);
        printf("  %-10s  idx=%d\n", b->nomen, b->indicium);
    }
    return 0;
}
