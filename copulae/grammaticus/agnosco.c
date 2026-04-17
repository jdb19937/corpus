/*
 * agnosco.c — Motor agnitionis morphologicae.
 *
 * Strategia: (1) exact match in tabula pronominum; (2) tentatio
 * suffixorum e tabulis declinationum et coniugationum ordine decres-
 * centis longitudinis. Quisque match producit (stem, descriptio).
 */
#include "grammaticus.h"
#include <stdio.h>
#include <string.h>

static int
pone(Analysis *out, int *n, int max, Categoria cat,
     const char *stem, const char *descr)
{
    if (*n >= max) return 0;
    strncpy(out[*n].stem, stem, sizeof out[0].stem - 1);
    out[*n].stem[sizeof out[0].stem - 1] = '\0';
    out[*n].cat = cat;
    out[*n].descriptio = descr;
    (*n)++;
    return 1;
}

static int
ends_with(const char *verbum, size_t vlen, const char *suf)
{
    size_t slen = strlen(suf);
    if (slen >= vlen) return 0;
    return strcmp(verbum + vlen - slen, suf) == 0;
}

static void
tenta_tabulam(const char *verbum, size_t vlen,
              const Finitio *tab, int n_tab,
              Analysis *out, int *n, int max)
{
    char stem[40];
    for (int i = 0; i < n_tab; i++) {
        if (!ends_with(verbum, vlen, tab[i].sufixus)) continue;
        size_t slen = strlen(tab[i].sufixus);
        size_t stlen = vlen - slen;
        if (stlen == 0 || stlen >= sizeof stem) continue;
        memcpy(stem, verbum, stlen);
        stem[stlen] = '\0';
        pone(out, n, max, tab[i].cat, stem, tab[i].descriptio);
    }
}

int
agnosce(const char *verbum, Analysis *out, int max_out)
{
    int n = 0;
    size_t vlen = strlen(verbum);
    if (vlen == 0 || max_out <= 0) return 0;

    /* Pronomina: exact match */
    for (int i = 0; i < N_PRONOMINA; i++) {
        if (strcmp(verbum, pronomina[i].forma) == 0) {
            pone(out, &n, max_out, CAT_PRON,
                 pronomina[i].lemma, pronomina[i].descriptio);
        }
    }

    /* Nomina et verba: suffix matching */
    tenta_tabulam(verbum, vlen, declinatio1f, N_DECL1F, out, &n, max_out);
    tenta_tabulam(verbum, vlen, declinatio2m, N_DECL2M, out, &n, max_out);
    tenta_tabulam(verbum, vlen, declinatio2n, N_DECL2N, out, &n, max_out);
    tenta_tabulam(verbum, vlen, coniugatio1,  N_CONJ1,  out, &n, max_out);
    tenta_tabulam(verbum, vlen, coniugatio2,  N_CONJ2,  out, &n, max_out);

    return n;
}

static const char *
categoria_nomen(Categoria c)
{
    switch (c) {
    case CAT_DECL1F: return "NOMEN ";
    case CAT_DECL2M: return "NOMEN ";
    case CAT_DECL2N: return "NOMEN ";
    case CAT_CONJ1:  return "VERBUM";
    case CAT_CONJ2:  return "VERBUM";
    case CAT_PRON:   return "PRON  ";
    }
    return "?     ";
}

void
analysis_imprime(const char *verbum, const Analysis *aa, int n)
{
    printf("\"%s\":\n", verbum);
    if (n == 0) {
        printf("  (nulla analysis)\n");
        return;
    }
    for (int i = 0; i < n; i++) {
        printf("  [%s] stem=%-10s %s\n",
               categoria_nomen(aa[i].cat),
               aa[i].stem,
               aa[i].descriptio);
    }
}
