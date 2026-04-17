/*
 * explicatio.c — Tracer catenae causalis rerum factarum.
 *
 * Unamquamque factum derivatum cum indice regulae quae eum peperit
 * associamus; per hanc tabulam recursive catenam ad observationes
 * originales retrogredi possumus.
 */
#include "peritus.h"
#include <stdio.h>

static int regula_de_facto[MAX_FACTA];
static int n_registratorum = 0;

void
explicatio_adde(int regula_idx, int factum_idx)
{
    while (n_registratorum <= factum_idx && n_registratorum < MAX_FACTA)
        regula_de_facto[n_registratorum++] = -1;
    if (factum_idx >= 0 && factum_idx < MAX_FACTA)
        regula_de_facto[factum_idx] = regula_idx;
}

static void
recurse(const char *conclusio, int depth, int max_depth)
{
    for (int i = 0; i < depth; i++) fputs("  ", stdout);
    if (depth > max_depth) {
        printf("... (profunditas exhausta)\n");
        return;
    }

    int fi = factum_index(conclusio);
    if (fi < 0) {
        printf("? %s (ignotum)\n", conclusio);
        return;
    }
    if (factum_ab_origine(fi)) {
        printf("o %s (ex observatione)\n", conclusio);
        return;
    }
    int r = (fi < MAX_FACTA) ? regula_de_facto[fi] : -1;
    if (r < 0 || r >= N_REGULARUM) {
        printf("* %s (sine regula nota)\n", conclusio);
        return;
    }
    const Regula *R = &regulae[r];
    printf("+ %s  <-  %s\n", conclusio, R->nomen);
    for (int a = 0; a < R->n_ante; a++)
        recurse(R->ante[a], depth + 1, max_depth);
}

void
explicatio_imprime(const char *conclusio)
{
    recurse(conclusio, 0, 12);
}
