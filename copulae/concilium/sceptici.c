/*
 * sceptici.c — Schola Sceptica (Pyrrho, Sextus Empiricus, Arcesilaus,
 * Carneades).
 *
 * Principia: epoche (suspensio iudicii); nihil vere cognoscibile;
 * tropes contrariorum per utramque partem argumentandi; ataraxia
 * consequitur suspensionem.
 */
#include "concilium.h"
#include <string.h>
#include <stdio.h>

static const struct {
    const char *thema;
    const char *responsum;
} responsa[] = {
    { "beat",   "Utrum beata vita sit non constat. Si constat, non est "
                "unum quod omnes agnoscant. Epoche; inde ataraxia." },
    { "virtu",  "Pro virtute tot argumenta sunt quot contra; epoche "
                "suadetur. Ex ipsa suspensione consequitur tranquillitas." },
    { "mort",   "De morte aliqui aiunt malum, aliqui bonum, aliqui neutrum. "
                "Ephoche: non iudicandum quid sit mors." },
    { "volupt", "Utrum voluptas summum bonum an vitanda sit, disputabile est. "
                "Epicurei aiunt bonum; Stoici adiaphoron; sapiens suspendit." },
    { "dolor",  "Dolor affectio est corporis, sed quale iudicium de eo, "
                "non constat. Suspensio iudicii levat; opinio gravat." },
    { "deu",    "De deis tot sunt sententiae quot philosophi. Ex hac ipsa "
                "multitudine dogmatum epoche consequitur." },
    { "deo",    "De deis tot sunt sententiae quot philosophi. Ex hac ipsa "
                "multitudine dogmatum epoche consequitur." },
    { "fortun", "Quid fortuna sit, utrum causa necne, nobis ignotum. "
                "Sapiens neque speret neque timeat; suspendat." },
    { NULL, NULL }
};

const char *
sceptici_nomen(void)
{
    return "Sceptici";
}

const char *
sceptici_motto(void)
{
    return "Ouden mallon; epoche; ataraxia per suspensionem.";
}

void
sceptici_respondet(const char *q, char *buf, size_t cap)
{
    for (int i = 0; responsa[i].thema; i++) {
        if (strstr(q, responsa[i].thema)) {
            snprintf(buf, cap, "%s", responsa[i].responsum);
            return;
        }
    }
    snprintf(buf, cap,
             "De hoc Sceptici nihil affirmant; pro et contra utrumque "
             "argumentum aequi pollet; epoche sola securus portus.");
}
