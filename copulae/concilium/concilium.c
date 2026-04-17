/*
 * concilium.c — Dialogus quattuor scholarum philosophicarum.
 *
 * Quattuor scholae Graeco-Romanae (Stoica, Epicurea, Peripatetica,
 * Sceptica) ad eandem quaestionem respondent. Quaeque schola suum
 * TU habet cum identica signatura (nomen, motto, respondet);
 * dispatcher per tabulam function-pointerorum omnes vocat.
 *
 * Usus: ./concilium                    # omnes quaestiones defaltae
 *       ./concilium "de virtute"       # quaestio particularis
 *
 * Officia linker: polymorphismus classicus — eadem signatura trans
 * quattuor TU, coniuncta in uno aggregato function-pointerorum.
 * Nullus collision nominum quia unusquisque TU suos functiones
 * praefixo-nomenat (stoici_, epicurei_, etc.).
 */
#include "concilium.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *const quaestiones_defaltae[] = {
    "quid est beata vita?",
    "quid est virtus?",
    "quid de morte?",
    "quid de voluptate?",
    "quid de dolore?",
    "quid de fortuna?",
    NULL
};

int
main(int argc, char **argv)
{
    printf("=== Concilium Philosophorum ===\n");
    printf("(Stoici, Epicurei, Peripatetici, Sceptici)\n");

    if (argc > 1) {
        char buf[512];
        buf[0] = '\0';
        size_t used = 0;
        for (int i = 1; i < argc; i++) {
            size_t need = strlen(argv[i]);
            if (used + need + 2 >= sizeof buf) break;
            if (i > 1) { buf[used++] = ' '; buf[used] = '\0'; }
            memcpy(buf + used, argv[i], need);
            used += need;
            buf[used] = '\0';
        }
        quaestor_dispenda(buf);
    } else {
        for (int i = 0; quaestiones_defaltae[i]; i++)
            quaestor_dispenda(quaestiones_defaltae[i]);
    }

    printf("\n=== Summarium Scholarum ===\n");
    for (int i = 0; i < N_SCHOLARUM; i++) {
        const Schola *s = &scholae[i];
        printf("  %-14s | %s\n", s->nomen(), s->motto());
    }

    return 0;
}
