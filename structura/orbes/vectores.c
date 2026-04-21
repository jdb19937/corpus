/*
 * vectores.c — Operationes in Vec3. Argumenta et reditus per valorem,
 * quod HFA classificationem et floating-point register passing probat.
 */
#include "orbes.h"
#include <math.h>

Vec3
vec_adde(Vec3 a, Vec3 b)
{
    Vec3 r = { a.x + b.x, a.y + b.y, a.z + b.z };
    return r;
}

Vec3
vec_scala(Vec3 v, double s)
{
    Vec3 r = { v.x * s, v.y * s, v.z * s };
    return r;
}

double
vec_norma(Vec3 v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vec3
vec_rota_z(Vec3 v, double theta)
{
    double ct = cos(theta);
    double st = sin(theta);
    Vec3 r = { ct * v.x - st * v.y, st * v.x + ct * v.y, v.z };
    return r;
}
