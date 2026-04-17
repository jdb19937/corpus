/*
 * cyclopaedia.c — Scientia communis in structura ontologica.
 *
 * Inspiratum a Cyc (Douglas Lenat, 1984–), qui scientiam communem
 * saeculi humani in logicam formalem transferre conatus est. Hic
 * minor imago: hierarchia classium (Ens > Substantia > Corpus >
 * Animal > Mammal > Homo > Philosophus), proprietates hereditariae,
 * individua cum classibus, et motor interrogator.
 *
 * Officia linker: quattuor tabulae static privatae in diversis TU
 * (taxonomia.c, proprietates.c, asserta.c) per API publica solum
 * attingibiles; interroga.c tres alios TU per octo functiones
 * publicas coniungit.
 */
#include "cyclopaedia.h"
#include <stdio.h>

static void
construe_mundum(void)
{
    /* Hierarchia metaphysica */
    classis_adde("Ens",         NULL);
    classis_adde("Substantia",  "Ens");
    classis_adde("Corpus",      "Substantia");
    classis_adde("Animal",      "Corpus");
    classis_adde("Mammal",      "Animal");
    classis_adde("Homo",        "Mammal");
    classis_adde("Philosophus", "Homo");
    classis_adde("Poeta",       "Homo");
    classis_adde("Divinitas",   "Ens");
    classis_adde("Olympius",    "Divinitas");
    classis_adde("Mundanus",    "Divinitas");

    /* Proprietates per classes */
    proprietas_pone("Animal",      "mortalis",       "verum");
    proprietas_pone("Animal",      "sensitivus",     "verum");
    proprietas_pone("Mammal",      "habet-pilos",    "verum");
    proprietas_pone("Mammal",      "sanguis-calidus","verum");
    proprietas_pone("Homo",        "habet-pedes",    "duos");
    proprietas_pone("Homo",        "rationalis",     "verum");
    proprietas_pone("Homo",        "loquitur",       "verum");
    proprietas_pone("Philosophus", "cogitat",        "multum");
    proprietas_pone("Poeta",       "cogitat",        "versibus");
    proprietas_pone("Divinitas",   "mortalis",       "falsum");
    proprietas_pone("Divinitas",   "rationalis",     "verum");
    proprietas_pone("Divinitas",   "habet-potentiam","magnam");
    proprietas_pone("Olympius",    "habitat",        "in-monte-olympo");
    proprietas_pone("Mundanus",    "habitat",        "in-silvis-et-rivis");

    /* Individua */
    individuum_adde("Socrates",   "Philosophus");
    individuum_adde("Plato",      "Philosophus");
    individuum_adde("Aristoteles","Philosophus");
    individuum_adde("Vergilius",  "Poeta");
    individuum_adde("Horatius",   "Poeta");
    individuum_adde("Zeus",       "Olympius");
    individuum_adde("Iuppiter",   "Olympius");
    individuum_adde("Pan",        "Mundanus");
    individuum_adde("Bucephalus", "Mammal");
}

int
main(void)
{
    construe_mundum();

    printf("=== Cyclopaedia Minor ===\n\n");
    taxonomia_imprime();
    printf("\n");
    proprietates_imprime();
    printf("\n");
    asserta_imprime();

    printf("\n=== Interrogationes Subsumptionis ===\n");
    static const char *const probae[][2] = {
        { "Socrates",   "Homo"        },
        { "Socrates",   "Mammal"      },
        { "Socrates",   "Animal"      },
        { "Socrates",   "Divinitas"   },
        { "Zeus",       "Divinitas"   },
        { "Zeus",       "Homo"        },
        { "Zeus",       "Ens"         },
        { "Pan",        "Olympius"    },
        { "Pan",        "Divinitas"   },
        { "Bucephalus", "Mammal"      },
        { "Bucephalus", "Homo"        },
        { NULL, NULL }
    };
    for (int i = 0; probae[i][0]; i++)
        printf("  estne %-12s %-12s ? %s\n",
               probae[i][0], probae[i][1],
               interroga_est(probae[i][0], probae[i][1]) ? "ita" : "non");

    printf("\n=== Proprietates Hereditariae ===\n");
    static const char *const proprii[] = {
        "mortalis", "rationalis", "habet-pilos", "habet-pedes",
        "cogitat", "habitat", "habet-potentiam", NULL
    };
    static const char *const qui[] = {
        "Socrates", "Vergilius", "Zeus", "Pan", "Bucephalus", NULL
    };
    for (int q = 0; qui[q]; q++) {
        printf("\n  %s:\n", qui[q]);
        for (int p = 0; proprii[p]; p++) {
            const char *v = interroga_habet(qui[q], proprii[p]);
            printf("    %-16s = %s\n", proprii[p], v ? v : "(ignotum)");
        }
    }

    printf("\n=== Catena Singulorum ===\n");
    interroga_de("Socrates");
    interroga_de("Zeus");
    interroga_de("Pan");
    interroga_de("Bucephalus");

    return 0;
}
