/*
 * patrona.c — Tabula patronarum et functio quaerens.
 *
 * Tabula globalis `patronae[]` (extern): clavis, index responsi, flag
 * de verbis post clavem. Ultima sedes cum clavi NULL est sentinella
 * pro casu ubi nullum verbum clavem convenit.
 */
#include "eliza.h"
#include <string.h>

const Patrona patronae[] = {
    { "mater",    0, 1 },
    { "pater",    0, 1 },
    { "familia",  0, 1 },
    { "frater",   0, 1 },
    { "soror",    0, 1 },
    { "uxor",     0, 1 },
    { "sentio",   1, 1 },
    { "doleo",    1, 1 },
    { "credo",    2, 1 },
    { "cogito",   2, 1 },
    { "puto",     2, 1 },
    { "timeo",    3, 1 },
    { "metuo",    3, 1 },
    { "amo",      4, 1 },
    { "diligo",   4, 1 },
    { "odi",      5, 1 },
    { "volo",     6, 1 },
    { "cupio",    6, 1 },
    { "somnio",   7, 1 },
    { "videor",   8, 1 },
    { "quomodo",  9, 1 },
    { "cur",     10, 1 },
    { "quid",    11, 1 },
    { "nemo",    12, 0 },
    { "nullus",  12, 0 },
    { "semper",  13, 0 },
    { "numquam", 13, 0 },
    { "nihil",   14, 0 },
    { NULL,      15, 0 }
};

const int N_PATRONARUM = sizeof patronae / sizeof patronae[0];

int
patrona_inveni(const Sententia *s, int *idx_pat, int *loco)
{
    for (int k = 0; k < N_PATRONARUM; k++) {
        if (!patronae[k].clavis) continue;
        for (int i = 0; i < s->n; i++) {
            if (strcmp(s->verba[i], patronae[k].clavis) == 0) {
                *idx_pat = k;
                *loco = i + 1;
                return 1;
            }
        }
    }
    for (int k = 0; k < N_PATRONARUM; k++) {
        if (!patronae[k].clavis) {
            *idx_pat = k;
            *loco = 0;
            return 0;
        }
    }
    *idx_pat = 0;
    *loco = 0;
    return 0;
}
