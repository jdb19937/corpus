/*
 * epicurei.c — Schola Epicurea (Epicurus, Lucretius, Philodemus).
 *
 * Principia: voluptas summum bonum (sed ataraxia, non hedone bruta);
 * atomi et inane; dei nos non curant; tetrapharmakon:
 *   (1) deum ne timeas,  (2) mortem ne timeas,
 *   (3) bonum facile paratur,  (4) malum facile toleratur.
 */
#include "concilium.h"
#include <string.h>
#include <stdio.h>

static const struct {
    const char *thema;
    const char *responsum;
} responsa[] = {
    { "beat",   "Beata vita est ataraxia: animi tranquillitas et aponia "
                "(absentia doloris corporis). Paucis opus est." },
    { "virtu",  "Virtus instrumentum est voluptatis; sine prudentia non est "
                "vivere iucunde, sine iucunde non est vivere prudenter." },
    { "mort",   "Mors nihil est ad nos: dum sumus, mors non adest; "
                "cum mors adest, nos non sumus. (Epicurus ad Menoeceum)" },
    { "volupt", "Voluptas summum bonum, sed non libido — sed absentia "
                "doloris corporis et perturbationis animi (ataraxia)." },
    { "dolor",  "Dolor acutus brevis est; chronicus tolerabilis. "
                "Memoria voluptatum praeteritarum dolorem lenit." },
    { "deu",    "Dei in intermundiis beati vivunt; nos non curant. "
                "Metus deorum vanus est; ne timeas." },
    { "deo",    "Dei in intermundiis beati vivunt; nos non curant. "
                "Metus deorum vanus est; ne timeas." },
    { "fortun", "Fortuna sapienti parum interest: quia voluptas interna, "
                "non in rebus externis sita." },
    { NULL, NULL }
};

const char *
epicurei_nomen(void)
{
    return "Epicurei";
}

const char *
epicurei_motto(void)
{
    return "Late vive; ataraxia summum bonum.";
}

void
epicurei_respondet(const char *q, char *buf, size_t cap)
{
    for (int i = 0; responsa[i].thema; i++) {
        if (strstr(q, responsa[i].thema)) {
            snprintf(buf, cap, "%s", responsa[i].responsum);
            return;
        }
    }
    snprintf(buf, cap,
             "Hic Epicurei dicerent: quaere voluptatem vera (ataraxia), "
             "fuge voluptates quae dolorem maiorem gignunt.");
}
