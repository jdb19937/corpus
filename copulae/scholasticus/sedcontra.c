/*
 * sedcontra.c — Componit "Sed contra" ex una auctoritate affirmativa.
 *
 * Structura classica Summae: brevis pagina cum una potenti citatione
 * quae thesim directe supportat.
 */
#include "scholasticus.h"
#include <stdio.h>
#include <string.h>

int
sedcontra_compone(const char *thema, char *buf, size_t cap)
{
    if (cap == 0) return 0;
    const Auctoritas *a = NULL;
    int idx = auctoritas_inveni(thema, +1, 0, &a);
    if (idx < 0 || !a) {
        snprintf(buf, cap,
                 "Sed contra: ratio naturalis docet thesim tenendam.");
        return 0;
    }
    snprintf(buf, cap,
             "Sed contra: %s in %s ait: \"%s\"",
             a->auctor, a->opus, a->citatio);
    return 1;
}
