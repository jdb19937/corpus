/*
 * eliza.c — ELIZA Rogeriana latine loquens.
 *
 * Imitatio programmatis quod Iosephus Weizenbaum anno 1966 scripsit.
 * Accipit sententiam aegrotantis per argumenta (si desint, sessionem
 * demonstrativam agit). Invenit patronam, reflectit pronomina post
 * clavem, eligit responsum, et memoriam circularem thematum servat.
 *
 * Officia linker probata: aggregatum `patronae[]` definitum in
 * patrona.c et legitum ex eliza.c; aggregatum `responsa_generica[]`
 * definitum in responsa.c et legitum hic; functiones trans quinque
 * units vocatae; status privatus in memoria.c et verbum.c occultus.
 */
#include "eliza.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char *const sessio[] = {
    "ego sum tristis et mater mea me non intellegit",
    "sentio me solum semper esse",
    "somnio de magno mari et navibus nigris",
    "volo fugere sed non possum",
    "cur omnia mala mihi accidunt",
    "nemo me vere amat",
    NULL
};

static void
responde(const char *input, unsigned seed)
{
    Sententia s;
    tokeniza(input, &s);

    int idx = 0, loc = 0;
    int matched = patrona_inveni(&s, &idx, &loc);
    int bucket = patronae[idx].responsum;

    printf("AEGROTUS: %s\n", input);
    printf("ELIZA   : %s", responsum_elige(bucket, seed));

    if (matched && patronae[idx].post && loc < s.n) {
        Sententia tail = s;
        reflecte(&tail);
        char buf[256];
        concatena_post(buf, sizeof buf, &tail, loc);
        if (buf[0]) {
            printf("  (de: %s)", buf);
            memoria_adde(buf);
        }
    } else if (!matched && memoria_plena()) {
        printf("  (revocamus: %s)", memoria_extrahe());
    }
    printf("\n\n");
}

int
main(int argc, char **argv)
{
    unsigned seed = (unsigned)time(NULL);

    if (argc > 1) {
        char buf[1024];
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
        responde(buf, seed);
        return 0;
    }

    printf("=== Sessio Rogeriana ===\n\n");
    for (int i = 0; sessio[i]; i++)
        responde(sessio[i], seed + (unsigned)(i * 7));

    printf("=== Responsa generica in promptu ===\n");
    for (int i = 0; i < N_GENERICORUM; i++)
        printf("  - %s\n", responsa_generica[i]);

    printf("\n=== Memoria ultimi thematis ===\n");
    if (memoria_plena())
        printf("  %s\n", memoria_extrahe());
    else
        printf("  (nulla memoria)\n");

    return 0;
}
