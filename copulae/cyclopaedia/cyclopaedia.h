#ifndef CYCLOPAEDIA_H
#define CYCLOPAEDIA_H

#define MAX_CLASSIUM      32
#define MAX_PROPRIETATUM  64
#define MAX_INDIVIDUORUM  32
#define MAX_NOMINIS       32
#define MAX_VALORIS       40

/* taxonomia.c — hierarchia classium per indicem parentis */
int         classis_adde(const char *nomen, const char *parens);
int         classis_index(const char *nomen);
int         classis_parens(int i);        /* -1 if radix */
const char *classis_nomen(int i);
int         classis_numerus(void);
int         classis_sub(int superior, int putativus);  /* 1 si putativus = superior aut descendit ex eo */
void        taxonomia_imprime(void);

/* proprietates.c — proprietates classium cum hereditate */
int         proprietas_pone(const char *classis, const char *clavis, const char *valor);
const char *proprietas_quaere(int classis_idx, const char *clavis);  /* NULL si absens */
void        proprietates_imprime(void);

/* asserta.c — individua et classes eorum */
int         individuum_adde(const char *nomen, const char *classis);
int         individuum_index(const char *nomen);
int         individuum_classis(int i);
const char *individuum_nomen(int i);
int         individuum_numerus(void);
void        asserta_imprime(void);

/* interroga.c — quaestiones cum traversatione hierarchiae */
int         interroga_est(const char *individuum, const char *classis);
const char *interroga_habet(const char *individuum, const char *proprietas);
void        interroga_de(const char *individuum);

#endif
