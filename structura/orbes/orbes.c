/*
 * orbes.c — Mechanica orbitalis planetarum systematis solaris.
 *
 * Exercet ABI floating-point: Vec3 (3 doubles, HFA) et ElementaOrbis
 * (magna struct 6 doubles + int64_t) per valorem transferuntur et
 * redduntur. Kepler aequatio solvitur iterative.
 */
#include "orbes.h"
#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
    srand(17);

    printf("=== Orbes Planetarum ===\n");
    printf("(N = %d)\n\n", N_PLANETARUM);

    double t = 2.0;  /* anni post epocham */
    for (int i = 0; i < N_PLANETARUM; i++) {
        Planeta p = planetae[i];
        Vec3 r = positio_in_orbe(p.orb, t);
        double d = vec_norma(r);

        fprintf(stderr, "[%s] r = (%.6f, %.6f, %.6f)\n",
                p.nomen, r.x, r.y, r.z);

        printf("%-10s a=%.1f AU  |r|=%.1f  id=%lld\n",
               p.nomen, p.orb.a, d, (long long)p.orb.id);
    }

    /* Vec3 per valorem: probat HFA classificationem */
    Vec3 a = { 1.0, 2.0, 3.0 };
    Vec3 b = { 4.0, -1.0, 0.5 };
    Vec3 c = vec_adde(a, b);
    Vec3 s = vec_scala(c, 0.5);
    printf("\nvec_adde  -> (%.1f, %.1f, %.1f)\n", c.x, c.y, c.z);
    printf("vec_scala -> (%.1f, %.1f, %.1f)\n", s.x, s.y, s.z);
    printf("vec_norma -> %.1f\n", vec_norma(c));

    return 0;
}
