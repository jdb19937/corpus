/*
 * asserta.c — Individua (instantiae) cum classibus suis.
 *
 * Quisque individuum habet nomen proprium et indicem classi directi;
 * classes ulteriores (superiores) per taxonomiam inventae sunt.
 */
#include "cyclopaedia.h"
#include <stdio.h>
#include <string.h>

static char nomina[MAX_INDIVIDUORUM][MAX_NOMINIS];
static int  classes[MAX_INDIVIDUORUM];
static int  n = 0;

int
individuum_adde(const char *nomen, const char *classis)
{
    int ci = classis_index(classis);
    if (ci < 0) return -1;
    if (individuum_index(nomen) >= 0) return -1;
    if (n >= MAX_INDIVIDUORUM) return -1;
    strncpy(nomina[n], nomen, MAX_NOMINIS - 1);
    nomina[n][MAX_NOMINIS - 1] = '\0';
    classes[n] = ci;
    return n++;
}

int
individuum_index(const char *nomen)
{
    for (int i = 0; i < n; i++)
        if (strcmp(nomina[i], nomen) == 0) return i;
    return -1;
}

int
individuum_classis(int i)
{
    return (i >= 0 && i < n) ? classes[i] : -1;
}

const char *
individuum_nomen(int i)
{
    return (i >= 0 && i < n) ? nomina[i] : "(ignotum)";
}

int
individuum_numerus(void) { return n; }

void
asserta_imprime(void)
{
    printf("Individua (%d):\n", n);
    for (int i = 0; i < n; i++)
        printf("  %-14s isa %s\n",
               nomina[i], classis_nomen(classes[i]));
}
