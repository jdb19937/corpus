/*
 * quaestor.c — Dispatcher trans quattuor scholas per tabulam
 * function-pointerorum.
 *
 * Hoc est cor probationis linker polymorphismi: quaeque schola
 * suus TU eandem signaturam (3 functiones) habet; hic eas in
 * unam tabulam coniungimus et iterative vocamus.
 */
#include "concilium.h"
#include <stdio.h>

const Schola scholae[] = {
    { stoici_nomen,       stoici_motto,       stoici_respondet       },
    { epicurei_nomen,     epicurei_motto,     epicurei_respondet     },
    { peripatetici_nomen, peripatetici_motto, peripatetici_respondet },
    { sceptici_nomen,     sceptici_motto,     sceptici_respondet     },
};

const int N_SCHOLARUM = (int)(sizeof scholae / sizeof scholae[0]);

void
quaestor_dispenda(const char *quaestio)
{
    printf("\n──── Quaestio: %s ────\n", quaestio);
    char buf[512];
    for (int i = 0; i < N_SCHOLARUM; i++) {
        const Schola *s = &scholae[i];
        s->respondet(quaestio, buf, sizeof buf);
        printf("\n[%s] (%s)\n  %s\n",
               s->nomen(), s->motto(), buf);
    }
}
