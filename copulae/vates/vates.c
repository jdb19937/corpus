/*
 * vates.c — Generator sententiarum latinarum oraculi.
 *
 * Generator per grammaticam liberam contextus cum templatis quattuor
 * casuum; lexicon continet voces in nominativo, accusativo, genitivo,
 * plus adiectiva, verba 3ae sg, adverbia, et exclamationes.
 *
 * Usus: ./vates [numerus_versuum] [semen]
 *
 * Officia linker: status RNG privatus in sortio.c accessibilis tantum
 * per API; septem tabulae extern in lexicon.c declaratae et hic (pro
 * reportagio) directae lectae; grammatica.c cum ambabus aliis TU
 * vinculata per vocationes functionum et aggregatorum.
 */
#include "vates.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int
main(int argc, char **argv)
{
    int n = 8;
    unsigned seed = 67;

    if (argc >= 2) {
        int v = atoi(argv[1]);
        if (v > 0) n = v;
    }
    if (argc >= 3) {
        int v = atoi(argv[2]);
        if (v != 0) seed = (unsigned)v;
    }

    if (n > 64) n = 64;
    sortio_semen(seed);

    printf("=== Carmen Oraculi ===\n");
    printf("(semen = %u, versus = %d)\n\n", seed, n);

    char buf[256];
    for (int i = 0; i < n; i++) {
        carmen_expande(buf, sizeof buf);
        printf("  %s\n", buf);
    }

    printf("\n=== Per regulam uniquamque (demonstratio) ===\n");
    for (int i = 0; i < N_REGULARUM; i++) {
        sententia_expande(buf, sizeof buf, i);
        printf("  regula %d [%s]\n    -> %s\n",
               i, regula_templatum(i), buf);
    }

    printf("\n=== Capacitas lexici ===\n");
    printf("  nomina (nom, acc, gen) : %d / %d / %d\n",
           N_NOMINA_NOM, N_NOMINA_ACC, N_NOMINA_GEN);
    printf("  adiectiva              : %d\n", N_ADIECTIVA);
    printf("  verba (3a sg)          : %d\n", N_VERBA_3S);
    printf("  adverbia               : %d\n", N_ADVERBIA);
    printf("  exclamationes          : %d\n", N_EXCLAMATIONUM);

    return 0;
}
