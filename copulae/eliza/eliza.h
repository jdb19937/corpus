#ifndef ELIZA_H
#define ELIZA_H

#include <stddef.h>

#define MAX_VERBA  64
#define LONG_VERBI 32

typedef struct {
    char verba[MAX_VERBA][LONG_VERBI];
    int  n;
} Sententia;

/* verbum.c */
void tokeniza(const char *input, Sententia *out);
void reflecte(Sententia *s);
void concatena_post(char *dest, size_t cap, const Sententia *s, int ab);

/* patrona.c */
typedef struct {
    const char *clavis;     /* key word; NULL means sentinel / generic */
    int         responsum;  /* which response bucket to use */
    int         post;       /* 1: quote reflected words after key; 0: do not */
} Patrona;

extern const Patrona patronae[];
extern const int     N_PATRONARUM;

int patrona_inveni(const Sententia *s, int *patrona_idx, int *loco);

/* responsa.c */
const char *responsum_elige(int bucket, unsigned seed);
extern const char *const responsa_generica[];
extern const int         N_GENERICORUM;

/* memoria.c */
void        memoria_adde(const char *text);
const char *memoria_extrahe(void);
int         memoria_plena(void);

#endif
