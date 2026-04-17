#ifndef VATES_H
#define VATES_H

/* sortio.c — status RNG globaliter inter TU custoditus */
void         sortio_semen(unsigned seed);
unsigned     sortio_nextu(void);
int          sortio_int(int modulus);
const char  *sortio_elige(const char *const *arr, int n);

/* lexicon.c — voces in casibus declarata */
extern const char *const nomina_nom[];
extern const int         N_NOMINA_NOM;
extern const char *const nomina_acc[];
extern const int         N_NOMINA_ACC;
extern const char *const nomina_gen[];
extern const int         N_NOMINA_GEN;
extern const char *const adiectiva[];
extern const int         N_ADIECTIVA;
extern const char *const verba_3s[];
extern const int         N_VERBA_3S;
extern const char *const adverbia[];
extern const int         N_ADVERBIA;
extern const char *const exclamationes[];
extern const int         N_EXCLAMATIONUM;

/* grammatica.c — productiones et expansio templatorum */
void        carmen_expande(char *buf, int cap);
void        sententia_expande(char *buf, int cap, int regula);
const char *regula_templatum(int i);
extern const int N_REGULARUM;

#endif
