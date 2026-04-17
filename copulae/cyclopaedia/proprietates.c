/*
 * proprietates.c — Proprietates classium cum hereditate.
 *
 * Quaerimus primum classi dato; si absens, classi parenti; et sic
 * deinceps usque ad radicem. Prima proprietas inventa vincit (i.e.
 * proprietates in classe specialiori illas in classe generali
 * superant).
 */
#include "cyclopaedia.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    int  classis;
    char clavis[MAX_NOMINIS];
    char valor[MAX_VALORIS];
} Pro;

static Pro pr[MAX_PROPRIETATUM];
static int n_pr = 0;

int
proprietas_pone(const char *classis, const char *clavis, const char *valor)
{
    if (n_pr >= MAX_PROPRIETATUM) return -1;
    int ci = classis_index(classis);
    if (ci < 0) return -1;
    pr[n_pr].classis = ci;
    strncpy(pr[n_pr].clavis, clavis, MAX_NOMINIS - 1);
    pr[n_pr].clavis[MAX_NOMINIS - 1] = '\0';
    strncpy(pr[n_pr].valor, valor, MAX_VALORIS - 1);
    pr[n_pr].valor[MAX_VALORIS - 1] = '\0';
    return n_pr++;
}

const char *
proprietas_quaere(int classis_idx, const char *clavis)
{
    int guard = MAX_CLASSIUM + 1;
    while (classis_idx >= 0 && guard-- > 0) {
        for (int i = 0; i < n_pr; i++) {
            if (pr[i].classis == classis_idx &&
                strcmp(pr[i].clavis, clavis) == 0)
                return pr[i].valor;
        }
        classis_idx = classis_parens(classis_idx);
    }
    return NULL;
}

void
proprietates_imprime(void)
{
    printf("Proprietates classium (%d):\n", n_pr);
    for (int i = 0; i < n_pr; i++)
        printf("  %-14s . %-16s = %s\n",
               classis_nomen(pr[i].classis), pr[i].clavis, pr[i].valor);
}
