/*
 * peritus.c — Systema Peritum ad Humores Hippocraticos Diagnoscendos.
 *
 * Imitatio programmatum peritarum saeculi XX (MYCIN, DENDRAL): motor
 * inferentiae forward-chaining super regulas SI-TUM. Humoralismum
 * Hippocraticum (sanguis, cholera, melancholia, phlegma) pro dominio
 * adhibet. Accipit symptomata per argumenta aut symptomatis defectis
 * "fervidus siccus iracundus" (persona cholerica) utitur.
 *
 * Linker probat: tabula `regulae[]` in regulae.c definita, ex motor.c
 * et explicatio.c usurpata; status facti privatus in facta.c; motor
 * functionem `explicatio_adde` ex alio TU vocat.
 */
#include "peritus.h"
#include <stdio.h>
#include <string.h>

static const char *const defecta_symptomata[] = {
    "fervidus", "siccus", "iracundus", NULL
};

static const char *const humores[] = {
    "humor-sanguis",
    "humor-cholera",
    "humor-melancholia",
    "humor-phlegma",
    NULL
};

static void
pone_observationes(int argc, char **argv)
{
    if (argc > 1) {
        for (int i = 1; i < argc; i++) factum_adde(argv[i]);
    } else {
        for (int i = 0; defecta_symptomata[i]; i++)
            factum_adde(defecta_symptomata[i]);
    }
}

int
main(int argc, char **argv)
{
    pone_observationes(argc, argv);

    printf("=== Systema Peritum Humoralis ===\n\n");
    facta_imprime();
    facta_marca_originalia();

    printf("\n");
    int n_fired = motor_currit();
    printf("Motor inferentiae: %d regulae applicatae.\n\n", n_fired);

    facta_imprime();

    printf("\n=== Diagnosis Humoralis ===\n");
    int aliquid = 0;
    for (int i = 0; humores[i]; i++) {
        if (factum_habet(humores[i])) {
            printf("  inventus: %s\n", humores[i]);
            aliquid = 1;
        }
    }
    if (!aliquid) printf("  (nullus humor ex symptomatis derivari potuit)\n");

    printf("\n=== Catenae Explicationis ===\n");
    for (int i = 0; humores[i]; i++)
        if (factum_habet(humores[i])) {
            printf("\n[%s]\n", humores[i]);
            explicatio_imprime(humores[i]);
        }

    printf("\n=== Iudicium Finalis ===\n");
    if (factum_habet("aegrotus")) {
        printf("AEGROTUS.\n\nCatena:\n");
        explicatio_imprime("aegrotus");
    } else {
        printf("SANUS (nulla cura necessaria).\n");
    }

    return 0;
}
