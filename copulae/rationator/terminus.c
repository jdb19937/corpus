/*
 * terminus.c — Tabula symbolorum, pool terminorum, constructores.
 *
 * Terminus est vel atomus ("anchises"), vel variabilis ("X"), vel
 * compositus ("parens(X, Y)"). Omnes termini ex pool communi
 * allocantur; pool_top/pool_restore permittunt backtracking sine
 * leaks memoriae.
 */
#include "rationator.h"
#include <stdio.h>
#include <string.h>

static char     symbola[MAX_SYMBOLA][32];
static int      n_symbola = 0;
static Terminus pool[MAX_TERMINI];
static int      n_pool = 0;

void
terminus_init(void)
{
    n_symbola = 0;
    n_pool = 0;
}

int
sym_id(const char *nomen)
{
    for (int i = 0; i < n_symbola; i++)
        if (strcmp(symbola[i], nomen) == 0) return i;
    if (n_symbola >= MAX_SYMBOLA) return -1;
    size_t cap = sizeof symbola[0] - 1;
    strncpy(symbola[n_symbola], nomen, cap);
    symbola[n_symbola][cap] = '\0';
    return n_symbola++;
}

const char *
sym_nomen(int id)
{
    return (id >= 0 && id < n_symbola) ? symbola[id] : "?";
}

static Terminus *
novum(GenusTerm g)
{
    if (n_pool >= MAX_TERMINI) return NULL;
    Terminus *t = &pool[n_pool++];
    t->genus = g;
    t->sym = -1;
    t->var_id = -1;
    t->n_arg = 0;
    for (int i = 0; i < MAX_ARGUMENTA; i++) t->arg[i] = NULL;
    return t;
}

Terminus *
term_atom(const char *n)
{
    Terminus *t = novum(TT_ATOM);
    if (t) t->sym = sym_id(n);
    return t;
}

Terminus *
term_var(const char *n)
{
    Terminus *t = novum(TT_VAR);
    if (t) {
        t->sym = sym_id(n);
        t->var_id = t->sym;  /* initial id; offset applied on renova */
    }
    return t;
}

Terminus *
term_cmpd(const char *n, int n_arg,
          Terminus *a1, Terminus *a2, Terminus *a3, Terminus *a4)
{
    Terminus *t = novum(TT_COMPOUND);
    if (!t) return NULL;
    t->sym = sym_id(n);
    if (n_arg > MAX_ARGUMENTA) n_arg = MAX_ARGUMENTA;
    t->n_arg = n_arg;
    Terminus *aa[MAX_ARGUMENTA] = { a1, a2, a3, a4 };
    for (int i = 0; i < n_arg; i++) t->arg[i] = aa[i];
    return t;
}

Terminus *
term_renova(Terminus *t, int offset)
{
    if (!t) return NULL;
    if (t->genus == TT_ATOM) return t;     /* atoms are shareable */
    Terminus *c = novum(t->genus);
    if (!c) return NULL;
    c->sym = t->sym;
    c->n_arg = t->n_arg;
    if (t->genus == TT_VAR) {
        c->var_id = t->var_id + offset;
    } else {
        for (int i = 0; i < t->n_arg; i++) {
            c->arg[i] = term_renova(t->arg[i], offset);
            if (!c->arg[i]) return NULL;
        }
    }
    return c;
}

int  pool_top(void)        { return n_pool; }
void pool_restore(int top) { if (top >= 0 && top <= n_pool) n_pool = top; }

void
term_imprime(const Terminus *t)
{
    if (!t) { printf("?"); return; }
    if (t->genus == TT_VAR) {
        printf("%s", sym_nomen(t->sym));
        return;
    }
    printf("%s", sym_nomen(t->sym));
    if (t->n_arg > 0) {
        printf("(");
        for (int i = 0; i < t->n_arg; i++) {
            if (i > 0) printf(", ");
            term_imprime(t->arg[i]);
        }
        printf(")");
    }
}
