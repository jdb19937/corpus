/*
 * kepler.c — Solutor aequationis Kepleri (Newton-Raphson) et
 * computatio positionis in plano orbitali.
 *
 * positio_in_orbe accipit ElementaOrbis per valorem (struct magna,
 * 6 doubles + int64_t = 56 bytes) — probat memoria-passing ABI.
 * Reddit Vec3 per valorem — probat aggregate return.
 */
#include "orbes.h"
#include <math.h>

double
kepler_solve(double M, double e)
{
    double E = M;
    for (int i = 0; i < 20; i++) {
        double f  = E - e * sin(E) - M;
        double fp = 1.0 - e * cos(E);
        E         = E - f / fp;
    }
    return E;
}

Vec3
positio_in_orbe(ElementaOrbis orb, double t_anni)
{
    double n = 2.0 * M_PI / pow(orb.a, 1.5);
    double M = orb.M0 + n * t_anni;
    double E = kepler_solve(M, orb.e);

    double cx = orb.a * (cos(E) - orb.e);
    double cy = orb.a * sqrt(1.0 - orb.e * orb.e) * sin(E);

    /* rotatio per omega, i, Omega — simplificata */
    Vec3 r = { cx, cy, 0.0 };
    r = vec_rota_z(r, orb.omega);
    Vec3 r2 = { r.x, cos(orb.i) * r.y, sin(orb.i) * r.y };
    return vec_rota_z(r2, orb.Omega);
}
