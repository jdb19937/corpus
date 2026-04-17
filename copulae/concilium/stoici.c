/*
 * stoici.c — Schola Stoica (Zeno, Cleanthes, Chrysippus, Seneca,
 * Epictetus, Marcus Aurelius).
 *
 * Principia: virtus sola sufficit; apatheia; res externae adiafora;
 * vivere secundum naturam; logos rationalis mundi.
 */
#include "concilium.h"
#include <string.h>
#include <stdio.h>

static const struct {
    const char *thema;
    const char *responsum;
} responsa[] = {
    { "beat",   "Beata vita est vivere secundum naturam et rationem. "
                "Nihil externum requiritur; virtus sola sufficit." },
    { "virtu",  "Virtus est summum bonum, sola et per se sufficiens. "
                "Quattuor cardines: prudentia, iustitia, fortitudo, temperantia." },
    { "mort",   "Mors pars naturae est. 'Memento mori' ut recte vivas. "
                "Sapiens eam aequo animo accipit." },
    { "volupt", "Voluptas neque bona neque mala est; adiaphoron. "
                "Non est quod sapiens quaerat, nec quod fugiat." },
    { "dolor",  "Dolor non malum, sed adiaphoron: neque bonum neque malum. "
                "Quid a te avertat virtutem, si nihil externum valet?" },
    { "deu",    "Deus est logos rationalis per totum cosmum diffusus. "
                "Nos particulae eius sumus; providentia regit omnia." },
    { "deo",    "Deus est logos rationalis per totum cosmum diffusus. "
                "Nos particulae eius sumus; providentia regit omnia." },
    { "fortun", "Fortuna adiaphoron est; nec bona nec mala in se. "
                "Non quae accidunt sed quomodo feramus nos afficit." },
    { NULL, NULL }
};

const char *
stoici_nomen(void)
{
    return "Stoici";
}

const char *
stoici_motto(void)
{
    return "Sustine et abstine; virtus sola sufficit.";
}

void
stoici_respondet(const char *q, char *buf, size_t cap)
{
    for (int i = 0; responsa[i].thema; i++) {
        if (strstr(q, responsa[i].thema)) {
            snprintf(buf, cap, "%s", responsa[i].responsum);
            return;
        }
    }
    snprintf(buf, cap,
             "De hoc Stoici dicerent: si non in tua potestate, "
             "indifferens est; si in tua potestate, virtus sufficit.");
}
