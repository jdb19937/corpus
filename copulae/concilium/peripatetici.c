/*
 * peripatetici.c — Schola Peripatetica (Aristoteles, Theophrastus,
 * Strato, Alexander Aphrodisiensis).
 *
 * Principia: virtus in medio; eudaimonia est activitas animae
 * secundum virtutem in vita completa; bona externa nonnihil
 * conferunt; hylomorphismus (forma + materia).
 */
#include "concilium.h"
#include <string.h>
#include <stdio.h>

static const struct {
    const char *thema;
    const char *responsum;
} responsa[] = {
    { "beat",   "Eudaimonia: activitas animae rationalis secundum virtutem "
                "in vita completa, cum bonis externis sufficientibus." },
    { "virtu",  "Virtus habitus est medius inter duo vitia (defectum et excessum). "
                "E.g., fortitudo inter timiditatem et audaciam." },
    { "mort",   "Mors privatio vitae, quae est activitas rationalis. "
                "Non timenda ab eo qui bene vixit." },
    { "volupt", "Voluptas per se non est bonum, sed comes activitatis naturalis. "
                "Vera voluptas est activitas virtutis perfecta." },
    { "dolor",  "Dolor malum est, sed non maximum. Sapiens dolori fortiter "
                "occurrit, neque fugiendo neque patiendo nimis." },
    { "deu",    "Deus est actus purus, primum movens immobile, cogitatio "
                "cogitationis. Non curat res singulas sed orbem movet." },
    { "deo",    "Deus est actus purus, primum movens immobile, cogitatio "
                "cogitationis. Non curat res singulas sed orbem movet." },
    { "fortun", "Fortuna externa nonnihil confert beatae vitae. Perfecta "
                "beatitudo requirit amicos, divitias, sanitatem, pulchritudinem." },
    { NULL, NULL }
};

const char *
peripatetici_nomen(void)
{
    return "Peripatetici";
}

const char *
peripatetici_motto(void)
{
    return "In medio stat virtus; eudaimonia secundum virtutem.";
}

void
peripatetici_respondet(const char *q, char *buf, size_t cap)
{
    for (int i = 0; responsa[i].thema; i++) {
        if (strstr(q, responsa[i].thema)) {
            snprintf(buf, cap, "%s", responsa[i].responsum);
            return;
        }
    }
    snprintf(buf, cap,
             "Peripatetici quaerunt medium: nec excessus nec defectus, "
             "sed habitus rectus in utroque extremo formatus.");
}
