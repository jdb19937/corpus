/*
 * grammatica.c — Grammatica libera contextus cum templatis.
 *
 * Templata utuntur marcis: %N (nom), %C (acc), %G (gen), %A (adi),
 * %V (verbum 3ae sg), %R (adverbium), %X (exclamatio). Functio
 * `expande_templatum` marcas substituit per vocationes ad sortio.c
 * et lexicon.c; deinde prima littera in maiusculam vertitur.
 */
#include "vates.h"
#include <string.h>

static int
appende(char *buf, int cap, int used, const char *s)
{
    int len = (int)strlen(s);
    if (used + len >= cap - 1) return used;
    memcpy(buf + used, s, (size_t)len);
    used += len;
    buf[used] = '\0';
    return used;
}

static int
appende_char(char *buf, int cap, int used, char c)
{
    if (used >= cap - 1) return used;
    buf[used++] = c;
    buf[used] = '\0';
    return used;
}

static void
expande_templatum(const char *templatum, char *buf, int cap)
{
    if (cap <= 0) return;
    buf[0] = '\0';
    int used = 0;
    for (const char *p = templatum; *p; p++) {
        if (*p == '%' && p[1]) {
            const char *w = NULL;
            switch (p[1]) {
            case 'N': w = sortio_elige(nomina_nom,    N_NOMINA_NOM);    break;
            case 'C': w = sortio_elige(nomina_acc,    N_NOMINA_ACC);    break;
            case 'G': w = sortio_elige(nomina_gen,    N_NOMINA_GEN);    break;
            case 'A': w = sortio_elige(adiectiva,     N_ADIECTIVA);     break;
            case 'V': w = sortio_elige(verba_3s,      N_VERBA_3S);      break;
            case 'R': w = sortio_elige(adverbia,      N_ADVERBIA);      break;
            case 'X': w = sortio_elige(exclamationes, N_EXCLAMATIONUM); break;
            default:
                used = appende_char(buf, cap, used, *p);
                continue;
            }
            p++;
            if (w) used = appende(buf, cap, used, w);
        } else {
            used = appende_char(buf, cap, used, *p);
        }
    }
    /* capitaliza primam litteram */
    if (buf[0] >= 'a' && buf[0] <= 'z')
        buf[0] = (char)(buf[0] - ('a' - 'A'));
}

static const char *const regulae_templata[] = {
    "%N %V %C.",
    "%A %N %V %R.",
    "%N %G %V %C.",
    "%X %N %V, et %N %V.",
    "%R %N %V; %N autem %V.",
    "sine %G non est %N.",
    "%N non %V %C; %C %V %N.",
    "ubi %N, ibi %N.",
    "%X %A %N sub %G manet.",
    "%N %V %R, sed %N %V %C.",
};

const int N_REGULARUM =
    (int)(sizeof regulae_templata / sizeof regulae_templata[0]);

const char *
regula_templatum(int i)
{
    return (i >= 0 && i < N_REGULARUM) ? regulae_templata[i] : "";
}

void
sententia_expande(char *buf, int cap, int regula)
{
    if (regula < 0 || regula >= N_REGULARUM) regula = 0;
    expande_templatum(regulae_templata[regula], buf, cap);
}

void
carmen_expande(char *buf, int cap)
{
    int i = sortio_int(N_REGULARUM);
    sententia_expande(buf, cap, i);
}
