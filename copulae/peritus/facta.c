/*
 * facta.c — Basis factorum: additio, quaestio, nomen per indicem.
 *
 * Stati privati (static) in hoc TU; cetera TU per API publica attingunt.
 * Originalia (signata per facta_marca_originalia) distinguuntur a
 * factis derivatis ut explicatio recta fieri possit.
 */
#include "peritus.h"
#include <stdio.h>
#include <string.h>

static char facta_nomina[MAX_FACTA][LONG_FACTI];
static int  originalia[MAX_FACTA];
static int  n_facta = 0;
static int  signatus = 0;   /* 0: nondum signatum; 1: originalia iam marcata */

int
factum_adde(const char *nomen)
{
    if (!nomen || !*nomen) return -1;
    int existens = factum_index(nomen);
    if (existens >= 0) return existens;
    if (n_facta >= MAX_FACTA) return -1;
    strncpy(facta_nomina[n_facta], nomen, LONG_FACTI - 1);
    facta_nomina[n_facta][LONG_FACTI - 1] = '\0';
    originalia[n_facta] = signatus ? 0 : 1;
    return n_facta++;
}

int
factum_habet(const char *nomen)
{
    return factum_index(nomen) >= 0;
}

int
factum_index(const char *nomen)
{
    for (int i = 0; i < n_facta; i++)
        if (strcmp(facta_nomina[i], nomen) == 0) return i;
    return -1;
}

const char *
factum_nomen(int i)
{
    return (i >= 0 && i < n_facta) ? facta_nomina[i] : NULL;
}

int
factum_numerus(void) { return n_facta; }

int
factum_ab_origine(int i)
{
    return (i >= 0 && i < n_facta) ? originalia[i] : 0;
}

void
facta_marca_originalia(void) { signatus = 1; }

void
facta_imprime(void)
{
    printf("Facta cognita (%d):\n", n_facta);
    for (int i = 0; i < n_facta; i++)
        printf("  %2d. %-28s %s\n", i, facta_nomina[i],
               originalia[i] ? "(ex observatione)" : "(derivatum)");
}
