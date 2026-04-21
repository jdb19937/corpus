/*
 * meteora.c — Simulatio ingressus meteorum in atmosphaeram Terrae.
 *
 * Exercet: Status (8 doubles = 64 bytes) per valorem (reddit et
 * accipit), variadic rapport_v, union Compositio cum type punning,
 * et floating-point integrationem.
 */
#include "meteora.h"
#include <stdio.h>
#include <stdlib.h>

const Meteorum meteora[] = {
    {
        "Tunguska",   { 0.0,  0, 0, 30000.0,  -15000.0, 0, -20000.0, 1.0e8 },
        { .fr = { 0.40f, 0.05f, 0.35f, 0.20f } }, 30.0
    },
    {
        "Chelyabinsk", { 0.0,  0, 0, 25000.0,  -13000.0, 0, -15000.0, 1.2e7 },
        { .fr = { 0.28f, 0.02f, 0.50f, 0.20f } }, 10.0
    },
    {
        "Barringer",  { 0.0,  0, 0, 20000.0,  -8000.0,  0, -10000.0, 3.0e8 },
        { .fr = { 0.92f, 0.07f, 0.01f, 0.00f } }, 25.0
    },
};

const int N_METEORUM = (int)(sizeof meteora / sizeof meteora[0]);

int
main(void)
{
    srand(31);

    printf("=== Ingressus Meteorum ===\n");

    for (int i = 0; i < N_METEORUM; i++) {
        Meteorum m   = meteora[i];
        Status s_fin = integra(m.init, m.radius_init, 0.05, 400);

        double E0 = energia_kin(m.init);
        double Ef = energia_kin(s_fin);
        double dE = E0 - Ef;
        double Mt = energia_TNT_ton(dE);

        fprintf(
            stderr, "[%s] E0=%.6e Ef=%.6e dE=%.6e\n",
            m.nomen, E0, Ef, dE
        );
        fprintf(stderr, "  comp raw=%llx\n", (unsigned long long)m.comp.raw);

        printf(
            "%-12s  m=%.0f  z=%.0f km  E0=%.0f Mt  Ef=%.0f Mt\n",
            m.nomen,
            s_fin.m,
            s_fin.z / 1000.0,
            energia_TNT_ton(E0),
            energia_TNT_ton(Ef)
        );
        rapport_v(
            m.nomen, 4,
            (double)m.comp.fr.fe,
            (double)m.comp.fr.ni,
            (double)m.comp.fr.si,
            (double)m.comp.fr.mg
        );
        printf("  dE = %.0f Mt TNT\n", Mt);
    }
    return 0;
}
