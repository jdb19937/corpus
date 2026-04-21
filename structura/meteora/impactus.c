/*
 * impactus.c — Computationes energiae et rapportatio variadica.
 * rapport_v est FUNCTIO VARIADICA (double args) — probat variadic
 * floating-point ABI (sysv: promotio ad double + RAX count).
 */
#include "meteora.h"
#include <stdio.h>
#include <stdarg.h>

double
energia_kin(Status s)
{
    double v2 = s.vx * s.vx + s.vy * s.vy + s.vz * s.vz;
    return 0.5 * s.m * v2;
}

double
energia_TNT_ton(double joules)
{
    return joules / 4.184e15;
}

void
rapport_v(const char *nom, int n, ...)
{
    va_list ap;
    va_start(ap, n);
    fprintf(stderr, "  comp[%s]:", nom);
    double total = 0.0;
    for (int i = 0; i < n; i++) {
        double x = va_arg(ap, double);
        total += x;
        fprintf(stderr, " %.6f", x);
    }
    fprintf(stderr, "  sum=%.6f\n", total);
    va_end(ap);

    va_start(ap, n);
    printf("  comp:");
    for (int i = 0; i < n; i++) {
        double x = va_arg(ap, double);
        printf(" %.2f", x);
    }
    printf("\n");
    va_end(ap);
}
