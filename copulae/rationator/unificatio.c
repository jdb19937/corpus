/*
 * unificatio.c — Algorithmus unificationis Robinsoni.
 *
 * Subst est lista (var_id, valor) parium, constructa incrementaliter
 * per unificationem. `walk` variabilem sequitur per chain ligaminum.
 * Nullus occurs-check (sicut in Prologis classicis).
 */
#include "rationator.h"
#include <stdio.h>

void
subst_init(Subst *s)
{
    s->n = 0;
}

Terminus *
walk(Terminus *t, const Subst *s)
{
    int guard = MAX_SUBST + 1;
    while (t && t->genus == TT_VAR && guard-- > 0) {
        int found = 0;
        for (int i = 0; i < s->n; i++) {
            if (s->par[i].var_id == t->var_id) {
                t = s->par[i].valor;
                found = 1;
                break;
            }
        }
        if (!found) break;
    }
    return t;
}

static int
bind(int var_id, Terminus *valor, Subst *s)
{
    if (s->n >= MAX_SUBST) return 0;
    s->par[s->n].var_id = var_id;
    s->par[s->n].valor = valor;
    s->n++;
    return 1;
}

int
unify(Terminus *a, Terminus *b, Subst *s)
{
    a = walk(a, s);
    b = walk(b, s);
    if (!a || !b) return 0;
    if (a == b) return 1;

    if (a->genus == TT_VAR && b->genus == TT_VAR && a->var_id == b->var_id)
        return 1;
    if (a->genus == TT_VAR) return bind(a->var_id, b, s);
    if (b->genus == TT_VAR) return bind(b->var_id, a, s);

    if (a->genus != b->genus) return 0;
    if (a->sym   != b->sym)   return 0;
    if (a->n_arg != b->n_arg) return 0;
    for (int i = 0; i < a->n_arg; i++)
        if (!unify(a->arg[i], b->arg[i], s)) return 0;
    return 1;
}

void
term_imprime_sub(const Terminus *t, const Subst *s)
{
    Terminus *w = walk((Terminus *)t, s);
    if (!w) { printf("?"); return; }
    if (w->genus == TT_VAR) {
        printf("_%d", w->var_id);
        return;
    }
    printf("%s", sym_nomen(w->sym));
    if (w->n_arg > 0) {
        printf("(");
        for (int i = 0; i < w->n_arg; i++) {
            if (i > 0) printf(", ");
            term_imprime_sub(w->arg[i], s);
        }
        printf(")");
    }
}
