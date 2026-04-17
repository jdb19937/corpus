/*
 * taxonomia.c — Arbor classium (isa-hierarchia).
 *
 * Quisque classis habet nomen et indicem parentis (aut -1 si radix).
 * Functio `classis_sub` per catenam parentum traversat ad subsumptio-
 * nem determinandam.
 */
#include "cyclopaedia.h"
#include <stdio.h>
#include <string.h>

static char nomina[MAX_CLASSIUM][MAX_NOMINIS];
static int  parentes[MAX_CLASSIUM];
static int  n = 0;

int
classis_adde(const char *nomen, const char *parens)
{
    int existens = classis_index(nomen);
    if (existens >= 0) return existens;
    if (n >= MAX_CLASSIUM) return -1;
    strncpy(nomina[n], nomen, MAX_NOMINIS - 1);
    nomina[n][MAX_NOMINIS - 1] = '\0';
    parentes[n] = parens ? classis_index(parens) : -1;
    return n++;
}

int
classis_index(const char *nomen)
{
    for (int i = 0; i < n; i++)
        if (strcmp(nomina[i], nomen) == 0) return i;
    return -1;
}

int
classis_parens(int i)
{
    return (i >= 0 && i < n) ? parentes[i] : -1;
}

const char *
classis_nomen(int i)
{
    return (i >= 0 && i < n) ? nomina[i] : "(ignotum)";
}

int
classis_numerus(void) { return n; }

int
classis_sub(int superior, int putativus)
{
    int guard = MAX_CLASSIUM + 1;
    while (putativus >= 0 && guard-- > 0) {
        if (putativus == superior) return 1;
        putativus = parentes[putativus];
    }
    return 0;
}

void
taxonomia_imprime(void)
{
    printf("Taxonomia classium (%d):\n", n);
    for (int i = 0; i < n; i++) {
        int p = parentes[i];
        if (p < 0) printf("  %-14s (radix)\n", nomina[i]);
        else       printf("  %-14s isa %s\n", nomina[i], nomina[p]);
    }
}
