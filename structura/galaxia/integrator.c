/*
 * integrator.c — Integrator symplecticus "leapfrog".
 */
#include "galaxia.h"

void
leapfrog(Corpus *bodies, int n, double G, double dt, int passus)
{
    for (int s = 0; s < passus; s++) {
        for (int i = 0; i < n; i++) {
            Vec3d a = v3_mul(vis_in(bodies, n, i, G), 1.0 / bodies[i].m);
            bodies[i].v = v3_add(bodies[i].v, v3_mul(a, 0.5 * dt));
        }
        for (int i = 0; i < n; i++) {
            bodies[i].r = v3_add(bodies[i].r, v3_mul(bodies[i].v, dt));
        }
        for (int i = 0; i < n; i++) {
            Vec3d a = v3_mul(vis_in(bodies, n, i, G), 1.0 / bodies[i].m);
            bodies[i].v = v3_add(bodies[i].v, v3_mul(a, 0.5 * dt));
        }
    }
}
