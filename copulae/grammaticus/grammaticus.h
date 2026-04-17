#ifndef GRAMMATICUS_H
#define GRAMMATICUS_H

#include <stddef.h>

#define MAX_ANALYSEUM 16

typedef enum {
    CAT_DECL1F,    /* 1a declinatio feminina */
    CAT_DECL2M,    /* 2a declinatio masculina */
    CAT_DECL2N,    /* 2a declinatio neutra */
    CAT_CONJ1,     /* 1a coniugatio */
    CAT_CONJ2,     /* 2a coniugatio */
    CAT_PRON       /* pronomen */
} Categoria;

typedef struct {
    Categoria   cat;
    const char *sufixus;       /* terminatio */
    const char *descriptio;    /* e.g. "nom sg F" */
} Finitio;

typedef struct {
    char        stem[40];
    Categoria   cat;
    const char *descriptio;
} Analysis;

/* declinatio_tab.c */
extern const Finitio declinatio1f[];
extern const int     N_DECL1F;
extern const Finitio declinatio2m[];
extern const int     N_DECL2M;
extern const Finitio declinatio2n[];
extern const int     N_DECL2N;

/* coniugatio_tab.c */
extern const Finitio coniugatio1[];
extern const int     N_CONJ1;
extern const Finitio coniugatio2[];
extern const int     N_CONJ2;

/* pronomina.c */
typedef struct {
    const char *forma;
    const char *lemma;
    const char *descriptio;
} Pronomen;

extern const Pronomen pronomina[];
extern const int      N_PRONOMINA;

/* agnosco.c */
int  agnosce(const char *verbum, Analysis *out, int max_out);
void analysis_imprime(const char *verbum, const Analysis *aa, int n);

#endif
