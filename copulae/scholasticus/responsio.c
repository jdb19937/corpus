/*
 * responsio.c — "Respondeo dicendum" et "Ad primum" responsiones.
 *
 * Respondeo applicat quattuor distinctiones classicas (secundum
 * quid, simpliciter; materialiter, formaliter; in potentia, in actu;
 * quoad nos, quoad se) prout thema permittit, et concludit in favorem
 * thesis.
 */
#include "scholasticus.h"
#include <stdio.h>
#include <string.h>

void
responsio_compone(const char *thesis, const char *thema,
                  char *buf, size_t cap)
{
    if (cap == 0) return;
    const Auctoritas *support = NULL;
    auctoritas_inveni(thema, +1, 1, &support);  /* alternam citationem */

    const char *distinctio = "secundum quid et simpliciter";
    if (strstr(thema, "deu"))        distinctio = "quoad se et quoad nos";
    else if (strstr(thema, "anim"))  distinctio = "materialiter et formaliter";
    else if (strstr(thema, "virtu")) distinctio = "in habitu et in actu";
    else if (strstr(thema, "beat"))  distinctio = "in via et in patria";
    else if (strstr(thema, "mort"))  distinctio = "quoad corpus et quoad animam";

    snprintf(buf, cap,
             "Respondeo dicendum quod %s. Ad cuius evidentiam "
             "distinguendum est %s. Nam %s.%s%s",
             thesis,
             distinctio,
             "primo modo obiectiones valent, secundo modo thesis vera remanet",
             support ? " Et hoc confirmat " : "",
             support ? support->auctor : "");
}

void
adprimum_compone(const char *thema, char *buf, size_t cap)
{
    if (cap == 0) return;
    const char *gloss = "in obiectione accipienda est secundum quid, non simpliciter";

    if (strstr(thema, "deu"))
        gloss = "de Deo non cognoscimus quid sit, sed quid non sit";
    else if (strstr(thema, "anim"))
        gloss = "anima separata est in statu imperfecto, non autem definitive";
    else if (strstr(thema, "virtu"))
        gloss = "habitus operativus manet etiam in potentia, licet non in actu";

    snprintf(buf, cap,
             "Ad primum ergo dicendum quod citatio obiectionis "
             "verificatur uno modo, non universaliter. Nempe %s.",
             gloss);
}
