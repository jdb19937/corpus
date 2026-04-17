/*
 * interroga.c — Interrogationes trans tres ceteras TU coniungens.
 *
 * Hic nullam tabulam privatam habemus; solum functiones quae
 * taxonomiam, proprietates, et asserta per eorum API publicas
 * usurpant.  Linker hic maxime probatur: quinque functiones
 * publicae ex tribus aliis files.
 */
#include "cyclopaedia.h"
#include <stdio.h>

int
interroga_est(const char *individuum, const char *classis)
{
    int i = individuum_index(individuum);
    if (i < 0) return 0;
    int c = classis_index(classis);
    if (c < 0) return 0;
    return classis_sub(c, individuum_classis(i));
}

const char *
interroga_habet(const char *individuum, const char *clavis)
{
    int i = individuum_index(individuum);
    if (i < 0) return NULL;
    return proprietas_quaere(individuum_classis(i), clavis);
}

void
interroga_de(const char *individuum)
{
    int i = individuum_index(individuum);
    if (i < 0) {
        printf("Interrogatum: %s (ignotum)\n", individuum);
        return;
    }
    printf("De %s:\n", individuum);
    int c = individuum_classis(i);
    printf("  Classis directa: %s\n", classis_nomen(c));

    printf("  Catena classium: ");
    int first = 1;
    int guard = MAX_CLASSIUM + 1;
    for (int k = c; k >= 0 && guard-- > 0; k = classis_parens(k)) {
        if (!first) printf(" < ");
        printf("%s", classis_nomen(k));
        first = 0;
    }
    printf("\n");
}
