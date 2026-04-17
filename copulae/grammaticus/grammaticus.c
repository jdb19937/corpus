/*
 * grammaticus.c — Analyzator morphologicus Latinus.
 *
 * Speculum inversum ad `vates` (vates generat, grammaticus analyzat).
 * Dat verbum inflexum, reddit omnes analyseis possibiles: lemma/stem,
 * categoria (nomen/verbum/pronomen), casus/numerus/persona/tempus.
 *
 * Ambiguitas: unum verbum saepe plures analyseis habet (e.g. "amas"
 * potest esse verbum 2p sg aut nomen "am-" in casu qui -as finit).
 * Omnes producuntur.
 *
 * Officia linker: quinque aggregata extern in tribus TU (declinatio_tab,
 * coniugatio_tab, pronomina); agnosco.c cum omnibus vinculatur.
 */
#include "grammaticus.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static const char *const demo[] = {
    "rosa", "rosam", "rosae", "rosas", "rosarum",
    "dominus", "domini", "dominum", "dominos",
    "bellum", "bella", "belli", "bellis",
    "amo", "amas", "amat", "amamus", "amant", "amabam", "amavit", "amare",
    "monere", "moneo", "mones", "monet",
    "ego", "mihi", "tibi", "hic", "haec", "huius", "ille", "eius", "qui",
    NULL
};

static void
minuscula(char *s)
{
    for (; *s; s++) *s = (char)tolower((unsigned char)*s);
}

int
main(int argc, char **argv)
{
    printf("=== Grammaticus Latinus ===\n");

    Analysis aa[MAX_ANALYSEUM];
    char buf[64];

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            strncpy(buf, argv[i], sizeof buf - 1);
            buf[sizeof buf - 1] = '\0';
            minuscula(buf);
            int n = agnosce(buf, aa, MAX_ANALYSEUM);
            analysis_imprime(buf, aa, n);
            putchar('\n');
        }
    } else {
        for (int i = 0; demo[i]; i++) {
            int n = agnosce(demo[i], aa, MAX_ANALYSEUM);
            analysis_imprime(demo[i], aa, n);
        }
    }

    printf("\n=== Summarium tabulatum ===\n");
    printf("  declinatio 1a F : %d suffixi\n", N_DECL1F);
    printf("  declinatio 2a M : %d suffixi\n", N_DECL2M);
    printf("  declinatio 2a N : %d suffixi\n", N_DECL2N);
    printf("  coniugatio 1a   : %d suffixi\n", N_CONJ1);
    printf("  coniugatio 2a   : %d suffixi\n", N_CONJ2);
    printf("  pronomina       : %d formae\n", N_PRONOMINA);

    return 0;
}
