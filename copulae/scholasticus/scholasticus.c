/*
 * scholasticus.c — Disputatio Scholastica stylu Summae Theologicae.
 *
 * Accipit thesim ("utrum virtus sit habitus"), construit articulum
 * classicum quinque partium: obiectiones → sed contra → respondeo →
 * ad obiectiones. Basis auctoritatum (Aristoteles, Augustinus, Thomas,
 * Averroes, Boethius, Dionysius, Avicenna, Anselmus, Cicero, Seneca)
 * selegitur per tags thematis.
 *
 * Usus: ./scholasticus                              # quattuor theses defaltae
 *       ./scholasticus "utrum virtus sit habitus"   # thesis particularis
 *       ./scholasticus "..." virtu                  # thesis cum thema explicito
 *
 * Officia linker: basis auctoritatum in auctoritates.c; quattuor
 * alii TU (obiectio, sedcontra, responsio, compositor) eam per
 * `auctoritas_inveni` legant.
 */
#include "scholasticus.h"
#include <stdio.h>
#include <string.h>

static const struct {
    const char *thesis;
    const char *thema;
} theses_defaltae[] = {
    { "anima humana est immortalis",      "anim immortal" },
    { "virtus est habitus electivus",     "virtu habit"   },
    { "Deus est",                         "deu motu"      },
    { "beatitudo consistit in visione Dei", "beat deu"    },
};
#define N_THESIUM (int)(sizeof theses_defaltae / sizeof theses_defaltae[0])

static const char *
thema_ex_thesi(const char *thesis)
{
    static const char *const stems[] = {
        "anim", "virtu", "deu", "beat", "mort",
        "habit", "intell", "natur", "fortun", NULL
    };
    for (int i = 0; stems[i]; i++)
        if (strstr(thesis, stems[i])) return stems[i];
    return "ens";  /* default */
}

int
main(int argc, char **argv)
{
    printf("=== Scholasticus: Disputatio in Stilu Summae ===\n");
    printf("(Basis: %d auctoritates)\n", N_AUCTORITATUM);

    if (argc > 1) {
        const char *thesis = argv[1];
        const char *thema  = argc > 2 ? argv[2] : thema_ex_thesi(thesis);
        articulus_compone(thesis, thema);
    } else {
        for (int i = 0; i < N_THESIUM; i++)
            articulus_compone(theses_defaltae[i].thesis,
                              theses_defaltae[i].thema);
    }

    printf("\n═══════════════════════════════════════\n");
    return 0;
}
