#ifndef RATIONATOR_H
#define RATIONATOR_H

#define MAX_SYMBOLA      64
#define MAX_TERMINI    4096
#define MAX_ARGUMENTA     4
#define MAX_CLAUSULAE    32
#define MAX_CORPORIS      4
#define MAX_SUBST       256
#define MAX_GOALS        16
#define MAX_PROFUNDITAS  24
#define MAX_SOLUTIONUM   16

typedef enum { TT_ATOM, TT_VAR, TT_COMPOUND } GenusTerm;

typedef struct Terminus {
    GenusTerm genus;
    int       sym;       /* symbol id (functor or variable name) */
    int       var_id;    /* runtime variable id (used when TT_VAR) */
    int       n_arg;
    struct Terminus *arg[MAX_ARGUMENTA];
} Terminus;

typedef struct {
    Terminus *caput;
    Terminus *corpus[MAX_CORPORIS];
    int       n_corpus;
} Clausula;

/* terminus.c — symbol table, term pool, builders */
void        terminus_init(void);
int         sym_id(const char *nomen);
const char *sym_nomen(int id);
Terminus   *term_atom(const char *n);
Terminus   *term_var(const char *n);
Terminus   *term_cmpd(const char *n, int n_arg,
                      Terminus *a1, Terminus *a2,
                      Terminus *a3, Terminus *a4);
Terminus   *term_renova(Terminus *t, int offset);
int         pool_top(void);
void        pool_restore(int top);
void        term_imprime(const Terminus *t);

/* baselas.c — clause database */
int             clausula_fact(Terminus *caput);
int             clausula_rule(Terminus *caput, int n_body,
                              Terminus *b1, Terminus *b2,
                              Terminus *b3, Terminus *b4);
int             baselas_numerus(void);
const Clausula *clausula_at(int i);
void            baselas_imprime(void);

/* unificatio.c */
typedef struct { int var_id; Terminus *valor; } SubstPar;

typedef struct {
    SubstPar par[MAX_SUBST];
    int n;
} Subst;

void      subst_init(Subst *s);
Terminus *walk(Terminus *t, const Subst *s);
int       unify(Terminus *a, Terminus *b, Subst *s);
void      term_imprime_sub(const Terminus *t, const Subst *s);

/* resolutor.c */
int resolutor_quaere(Terminus *quaestio,
                     int n_query_vars,
                     const int *query_var_ids,
                     const char *const *query_var_names,
                     int max_solutionum);

#endif
