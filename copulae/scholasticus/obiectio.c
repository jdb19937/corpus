/*
 * obiectio.c — Componit obiectiones contra thesim.
 *
 * Pro unaquaque obiectione, selegit auctoritatem ex basi cuius
 * polaritas est negativa aut neutralis, et eam format in forma
 * Scholastica: "Praeterea, sicut dicit N in Opere K: 'citatio'.
 * Ergo videtur quod non..."
 */
#include "scholasticus.h"
#include <stdio.h>
#include <string.h>

static int
appende(char *buf, size_t cap, size_t used, const char *s)
{
    size_t len = strlen(s);
    if (used + len >= cap) return used;
    memcpy(buf + used, s, len);
    used += len;
    buf[used] = '\0';
    return (int)used;
}

int
obiectio_compone(const char *thema, char *buf, size_t cap, int max_obi)
{
    if (cap == 0) return 0;
    buf[0] = '\0';
    size_t used = 0;
    used = (size_t)appende(buf, cap, used,
        "Videtur quod non, quia:\n");

    int posuit = 0;
    /* Primum negativas quaerimus; deinde neutras; deinde qualicumque */
    int pols[3] = { -1, 0, +1 };
    int pol_i = 0;
    int skip = 0;
    while (posuit < max_obi && pol_i < 3) {
        const Auctoritas *a;
        int idx = auctoritas_inveni(thema, pols[pol_i], skip, &a);
        if (idx < 0) {
            pol_i++;
            skip = 0;
            continue;
        }
        skip++;
        char block[512];
        snprintf(block, sizeof block,
                 "\n  %d. Praeterea, %s in %s dicit: \"%s\" "
                 "Ergo %s.\n",
                 posuit + 1, a->auctor, a->opus, a->citatio,
                 a->polaritas < 0
                   ? "ad hoc pertinet contra thesim"
                   : "inde non sequitur thesim necessario");
        used = (size_t)appende(buf, cap, used, block);
        posuit++;
    }
    if (posuit == 0)
        used = (size_t)appende(buf, cap, used,
            "  (nullae obiectiones notae ex auctoribus.)\n");
    (void)used;
    return posuit;
}
