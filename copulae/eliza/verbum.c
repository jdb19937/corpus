/*
 * verbum.c — Tokenizatio et reflexio pronominum.
 *
 * Functiones publicae: tokeniza, reflecte, concatena_post.
 * Tabula privata: mutationes[] (pronomina Latina ego<->tu etc.).
 */
#include "eliza.h"
#include <ctype.h>
#include <string.h>

static void
minuscula(char *s)
{
    for (; *s; s++) *s = (char)tolower((unsigned char)*s);
}

static void
purga_punctum(char *s)
{
    char *w = s;
    for (char *r = s; *r; r++) {
        unsigned char c = (unsigned char)*r;
        if (isalnum(c) || c == ' ' || c == '-') *w++ = (char)c;
        else if (ispunct(c)) *w++ = ' ';
    }
    *w = '\0';
}

void
tokeniza(const char *input, Sententia *out)
{
    static char buf[MAX_VERBA * LONG_VERBI];
    size_t cap = sizeof buf - 1;
    strncpy(buf, input, cap);
    buf[cap] = '\0';
    minuscula(buf);
    purga_punctum(buf);

    out->n = 0;
    char *tok = strtok(buf, " \t\n");
    while (tok && out->n < MAX_VERBA) {
        strncpy(out->verba[out->n], tok, LONG_VERBI - 1);
        out->verba[out->n][LONG_VERBI - 1] = '\0';
        out->n++;
        tok = strtok(NULL, " \t\n");
    }
}

typedef struct { const char *de; const char *ad; } Reflexio;

static const Reflexio mutationes[] = {
    { "ego",   "tu"    },
    { "me",    "te"    },
    { "mihi",  "tibi"  },
    { "meus",  "tuus"  },
    { "mea",   "tua"   },
    { "meum",  "tuum"  },
    { "mei",   "tui"   },
    { "nos",   "vos"   },
    { "noster","vester"},
    { "nostra","vestra"},
    { "tu",    "ego"   },
    { "te",    "me"    },
    { "tibi",  "mihi"  },
    { "tuus",  "meus"  },
    { "tua",   "mea"   },
    { "tuum",  "meum"  },
    { "tui",   "mei"   },
    { "vos",   "nos"   },
    { "sum",   "es"    },
    { "es",    "sum"   },
    { "sumus", "estis" },
    { "estis", "sumus" },
    { NULL, NULL }
};

void
reflecte(Sententia *s)
{
    for (int i = 0; i < s->n; i++) {
        for (const Reflexio *r = mutationes; r->de; r++) {
            if (strcmp(s->verba[i], r->de) == 0) {
                strncpy(s->verba[i], r->ad, LONG_VERBI - 1);
                s->verba[i][LONG_VERBI - 1] = '\0';
                break;
            }
        }
    }
}

void
concatena_post(char *dest, size_t cap, const Sententia *s, int ab)
{
    if (cap == 0) return;
    dest[0] = '\0';
    size_t used = 0;
    for (int i = ab; i < s->n; i++) {
        size_t need = strlen(s->verba[i]);
        if (used + need + 2 >= cap) break;
        if (used > 0) { dest[used++] = ' '; dest[used] = '\0'; }
        memcpy(dest + used, s->verba[i], need);
        used += need;
        dest[used] = '\0';
    }
}
