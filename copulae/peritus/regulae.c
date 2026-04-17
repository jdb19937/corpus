/*
 * regulae.c — Tabula regularum si-tum humoralis Hippocratici.
 *
 * Sedecim regulae: primae quattuor humores ex qualitatibus corporis
 * derivant (fervidus/humidus, etc.), quaternae ex indolibus animi,
 * quattuor curas ex humoribus, ultimae aegrotationem ex curis.
 */
#include "peritus.h"

const Regula regulae[] = {
    { "R01-sanguis-qual",
      { "fervidus", "humidus" }, 2,
      "humor-sanguis",
      "Fervidum et humidum humorem sanguineum denotant." },

    { "R02-cholera-qual",
      { "fervidus", "siccus" }, 2,
      "humor-cholera",
      "Fervidum et siccum humorem cholericum (bilem flavam) denotant." },

    { "R03-melancholia-qual",
      { "frigidus", "siccus" }, 2,
      "humor-melancholia",
      "Frigidum et siccum humorem melancholicum (bilem atram) denotant." },

    { "R04-phlegma-qual",
      { "frigidus", "humidus" }, 2,
      "humor-phlegma",
      "Frigidum et humidum humorem pituitosum denotant." },

    { "R05-sanguis-indoles",
      { "hilaris" }, 1,
      "humor-sanguis",
      "Indoles hilaris humorem sanguineum indicat." },

    { "R06-cholera-indoles",
      { "iracundus" }, 1,
      "humor-cholera",
      "Indoles iracunda humorem cholericum indicat." },

    { "R07-melancholia-indoles",
      { "tristis" }, 1,
      "humor-melancholia",
      "Indoles tristis humorem melancholicum indicat." },

    { "R08-phlegma-indoles",
      { "lentus" }, 1,
      "humor-phlegma",
      "Indoles lenta humorem phlegmaticum indicat." },

    { "R09-cura-sanguis",
      { "humor-sanguis" }, 1,
      "cura-abstine-vino",
      "Nimius sanguis vino aggravatur: abstinentia suadetur." },

    { "R10-cura-cholera",
      { "humor-cholera" }, 1,
      "cura-aqua-frigida",
      "Cholera aqua frigida temperari potest." },

    { "R11-cura-melancholia",
      { "humor-melancholia" }, 1,
      "cura-musica",
      "Melancholia musica levatur (iuxta Boethium)." },

    { "R12-cura-phlegma",
      { "humor-phlegma" }, 1,
      "cura-exercitatio",
      "Phlegma exercitatione diminuitur." },

    { "R13-aegrotus-vinum",
      { "cura-abstine-vino" }, 1,
      "aegrotus",
      "Qui vinum abstinere debet, aegrotat." },

    { "R14-aegrotus-aqua",
      { "cura-aqua-frigida" }, 1,
      "aegrotus",
      "Qui aquam frigidam bibere debet, aegrotat." },

    { "R15-aegrotus-musica",
      { "cura-musica" }, 1,
      "aegrotus",
      "Qui musica uti debet, aegrotat." },

    { "R16-aegrotus-exerc",
      { "cura-exercitatio" }, 1,
      "aegrotus",
      "Qui exercitari debet, aegrotat." },
};

const int N_REGULARUM = (int)(sizeof regulae / sizeof regulae[0]);
