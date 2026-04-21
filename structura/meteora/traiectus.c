/*
 * traiectus.c — Integrator RK2 pro traiectu meteoro. Status struct
 * (8 doubles) per VALOREM redditur et accipitur.
 */
#include "meteora.h"
#include <math.h>

static Status
derivatives(Status s, double radius, double Cd)
{
    double vmag = sqrt(s.vx * s.vx + s.vy * s.vy + s.vz * s.vz);
    double rho  = densitas_aeris(s.z);
    double A    = M_PI * radius * radius;
    double Fd   = 0.5 * Cd * rho * vmag * vmag * A;
    double g    = g_accel(s.z);

    double ax = (vmag > 0.0) ? -Fd * s.vx / (vmag * s.m) : 0.0;
    double ay = (vmag > 0.0) ? -Fd * s.vy / (vmag * s.m) : 0.0;
    double az = ((vmag > 0.0) ? -Fd * s.vz / (vmag * s.m) : 0.0) - g;

    /* ablatio massae: simplificata */
    double sigma = 1.0e-8;
    double dm    = -sigma * A * rho * vmag * vmag * vmag;

    Status d = { 1.0, s.vx, s.vy, s.vz, ax, ay, az, dm };
    return d;
}

Status
step_rk2(Status s, double dt, double radius, double Cd)
{
    Status k1 = derivatives(s, radius, Cd);
    Status sm = {
        s.t + 0.5 * dt * k1.t,
        s.x + 0.5 * dt * k1.x, s.y + 0.5 * dt * k1.y, s.z + 0.5 * dt * k1.z,
        s.vx + 0.5 * dt * k1.vx, s.vy + 0.5 * dt * k1.vy, s.vz + 0.5 * dt * k1.vz,
        s.m + 0.5 * dt * k1.m
    };
    Status k2 = derivatives(sm, radius, Cd);
    Status r = {
        s.t + dt * k2.t,
        s.x + dt * k2.x, s.y + dt * k2.y, s.z + dt * k2.z,
        s.vx + dt * k2.vx, s.vy + dt * k2.vy, s.vz + dt * k2.vz,
        s.m + dt * k2.m
    };
    if (r.m < 0.0) r.m = 0.0;
    if (r.z < 0.0) r.z = 0.0;
    return r;
}

Status
integra(Status s0, double radius, double dt, int passus)
{
    Status s = s0;
    double Cd = 1.0;
    for (int i = 0; i < passus; i++) {
        s = step_rk2(s, dt, radius, Cd);
        if (s.z <= 0.0) break;
    }
    return s;
}
