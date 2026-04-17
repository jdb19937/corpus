/*
 * baselas.c — Basis clausularum Horn.
 *
 * Clausula habet caput et corpus (0 aut plus sub-goals). Factum
 * est clausula cum corpore vacuo.
 */
#include "rationator.h"
#include <stdio.h>

static Clausula basis[MAX_CLAUSULAE];
static int      n_basis = 0;

int
clausula_fact(Terminus *caput)
{
    if (n_basis >= MAX_CLAUSULAE || !caput) return -1;
    basis[n_basis].caput = caput;
    basis[n_basis].n_corpus = 0;
    return n_basis++;
}

int
clausula_rule(Terminus *caput, int n_body,
              Terminus *b1, Terminus *b2, Terminus *b3, Terminus *b4)
{
    if (n_basis >= MAX_CLAUSULAE || !caput) return -1;
    if (n_body > MAX_CORPORIS) n_body = MAX_CORPORIS;
    basis[n_basis].caput = caput;
    basis[n_basis].n_corpus = n_body;
    Terminus *bb[MAX_CORPORIS] = { b1, b2, b3, b4 };
    for (int i = 0; i < n_body; i++) basis[n_basis].corpus[i] = bb[i];
    return n_basis++;
}

int
baselas_numerus(void) { return n_basis; }

const Clausula *
clausula_at(int i)
{
    return (i >= 0 && i < n_basis) ? &basis[i] : NULL;
}

void
baselas_imprime(void)
{
    printf("Basis clausularum (%d):\n", n_basis);
    for (int i = 0; i < n_basis; i++) {
        printf("  %2d. ", i);
        term_imprime(basis[i].caput);
        if (basis[i].n_corpus > 0) {
            printf(" :- ");
            for (int j = 0; j < basis[i].n_corpus; j++) {
                if (j > 0) printf(", ");
                term_imprime(basis[i].corpus[j]);
            }
        }
        printf(".\n");
    }
}
