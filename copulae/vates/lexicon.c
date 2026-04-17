/*
 * lexicon.c — Aggregata verborum extern visibilia.
 *
 * Omnia externe declarata; grammatica.c eas per `sortio_elige` vocat.
 * Voces sunt feminina singularia primae aut tertiae declinationis
 * ut forma consistens maneat.
 */
#include "vates.h"

const char *const nomina_nom[] = {
    "fortuna",   "mors",      "veritas",   "amor",      "tempus",
    "fatum",     "sapientia", "dolor",     "virtus",    "natura",
    "labor",     "silentium", "spes",      "memoria",   "umbra"
};
const int N_NOMINA_NOM = (int)(sizeof nomina_nom / sizeof nomina_nom[0]);

const char *const nomina_acc[] = {
    "fortunam",  "mortem",    "veritatem", "amorem",    "tempus",
    "fatum",     "sapientiam","dolorem",   "virtutem",  "naturam",
    "laborem",   "silentium", "spem",      "memoriam",  "umbram"
};
const int N_NOMINA_ACC = (int)(sizeof nomina_acc / sizeof nomina_acc[0]);

const char *const nomina_gen[] = {
    "fortunae",  "mortis",    "veritatis", "amoris",    "temporis",
    "fati",      "sapientiae","doloris",   "virtutis",  "naturae",
    "laboris",   "silentii",  "spei",      "memoriae",  "umbrae"
};
const int N_NOMINA_GEN = (int)(sizeof nomina_gen / sizeof nomina_gen[0]);

const char *const adiectiva[] = {
    "caeca",    "longa",    "brevis",   "nigra",    "dulcis",
    "aspera",   "clara",    "obscura",  "vera",     "falsa",
    "magna",    "parva",    "tacita",   "profunda"
};
const int N_ADIECTIVA = (int)(sizeof adiectiva / sizeof adiectiva[0]);

const char *const verba_3s[] = {
    "regit",    "vincit",   "fallit",   "movet",    "celat",
    "docet",    "consumit", "illuminat","generat",  "destruit",
    "manet",    "fugit",    "quaerit",  "laudat"
};
const int N_VERBA_3S = (int)(sizeof verba_3s / sizeof verba_3s[0]);

const char *const adverbia[] = {
    "semper",  "numquam", "saepe",   "raro",    "clam",
    "palam",   "cito",    "tarde",   "sic",     "ita",
    "heri",    "cras",    "hodie",   "tacite"
};
const int N_ADVERBIA = (int)(sizeof adverbia / sizeof adverbia[0]);

const char *const exclamationes[] = {
    "O!", "Heu!", "Ecce,", "Vide:", "Nota bene,", "Audi:"
};
const int N_EXCLAMATIONUM =
    (int)(sizeof exclamationes / sizeof exclamationes[0]);
