/*
 * gravitas.c — Operationes Vec3d (per valorem) et computatio virium
 * gravitationalium.
 *
 * Vec3d argument/return probat HFA passing. vis_in reddit Vec3d ex
 * loop super omnia corpora.
 */
#include "galaxia.h"
#include <math.h>

Vec3d
v3_sub(Vec3d a, Vec3d b)
{
    Vec3d r = { a.x - b.x, a.y - b.y, a.z - b.z };
    return r;
}

Vec3d
v3_add(Vec3d a, Vec3d b)
{
    Vec3d r = { a.x + b.x, a.y + b.y, a.z + b.z };
    return r;
}

Vec3d
v3_mul(Vec3d v, double s)
{
    Vec3d r = { v.x * s, v.y * s, v.z * s };
    return r;
}

double
v3_dot(Vec3d a, Vec3d b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3d
vis_in(const Corpus *bodies, int n, int i, double G)
{
    Vec3d F = { 0.0, 0.0, 0.0 };
    double eps2 = 1.0;  /* softening */
    for (int j = 0; j < n; j++) {
        if (j == i) continue;
        Vec3d d = v3_sub(bodies[j].r, bodies[i].r);
        double r2 = v3_dot(d, d) + eps2;
        double inv = 1.0 / (r2 * sqrt(r2));
        double k = G * bodies[i].m * bodies[j].m * inv;
        F = v3_add(F, v3_mul(d, k));
    }
    return F;
}

double
energia_total(const Corpus *bodies, int n, double G)
{
    double K = 0.0, U = 0.0;
    for (int i = 0; i < n; i++) {
        K += 0.5 * bodies[i].m * v3_dot(bodies[i].v, bodies[i].v);
        for (int j = i + 1; j < n; j++) {
            Vec3d d = v3_sub(bodies[j].r, bodies[i].r);
            double r = sqrt(v3_dot(d, d) + 1.0);
            U -= G * bodies[i].m * bodies[j].m / r;
        }
    }
    return K + U;
}

Vec3d
impetus_total(const Corpus *bodies, int n)
{
    Vec3d p = { 0.0, 0.0, 0.0 };
    for (int i = 0; i < n; i++) {
        p = v3_add(p, v3_mul(bodies[i].v, bodies[i].m));
    }
    return p;
}
