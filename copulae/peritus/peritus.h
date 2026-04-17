#ifndef PERITUS_H
#define PERITUS_H

#define MAX_FACTA    64
#define LONG_FACTI   40
#define MAX_ANTE      4

/* facta.c */
int         factum_adde(const char *nomen);
int         factum_habet(const char *nomen);
int         factum_index(const char *nomen);
const char *factum_nomen(int i);
int         factum_numerus(void);
int         factum_ab_origine(int i);
void        facta_marca_originalia(void);
void        facta_imprime(void);

/* regulae.c */
typedef struct {
    const char *nomen;
    const char *ante[MAX_ANTE];
    int         n_ante;
    const char *consequens;
    const char *explicatio;
} Regula;

extern const Regula regulae[];
extern const int    N_REGULARUM;

/* motor.c */
int motor_currit(void);  /* returns total rules applied */

/* explicatio.c */
void explicatio_adde(int regula_idx, int factum_idx);
void explicatio_imprime(const char *conclusio);

#endif
