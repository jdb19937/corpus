/*
 * resolutor.c — SLD resolution (backward-chaining cum backtracking).
 *
 * Pro quoque clausula, renovat variabiles (per pool + offset fresh),
 * unificat caput cum goal, et recursive novas sub-goals resolvit.
 * Pool et subst ad checkpoint restaurantur post omnis fallere
 * aut post solutionem inventam (ut sequens clausula puram paginam
 * habeat).
 */
#include "rationator.h"
#include <stdio.h>

static int solutiones_inventae;
static int solutiones_max;
static int n_qv;
static const int         *qv_ids;
static const char *const *qv_names;

static void
solutio_imprime(const Subst *s)
{
    solutiones_inventae++;
    printf("  solutio %d: ", solutiones_inventae);
    if (n_qv == 0) {
        printf("verum.");
    } else {
        for (int i = 0; i < n_qv; i++) {
            if (i > 0) printf(", ");
            Terminus dummy;
            dummy.genus  = TT_VAR;
            dummy.sym    = -1;
            dummy.var_id = qv_ids[i];
            dummy.n_arg  = 0;
            for (int k = 0; k < MAX_ARGUMENTA; k++) dummy.arg[k] = NULL;
            printf("%s = ", qv_names[i]);
            Terminus *v = walk(&dummy, s);
            if (v && v->genus == TT_VAR && v->var_id == dummy.var_id)
                printf("_");
            else
                term_imprime_sub(v, s);
        }
    }
    printf("\n");
}

static int
resolve(Terminus **goals, int n_goals, Subst *s, int depth)
{
    if (solutiones_inventae >= solutiones_max) return 1;
    if (depth > MAX_PROFUNDITAS) return 0;
    if (n_goals == 0) {
        solutio_imprime(s);
        return 1;
    }

    for (int c = 0; c < baselas_numerus(); c++) {
        const Clausula *cl = clausula_at(c);
        int offset     = (depth + 1) * MAX_SYMBOLA;
        int pool_chk   = pool_top();
        int subst_chk  = s->n;

        Terminus *ren_caput = term_renova(cl->caput, offset);
        if (!ren_caput) { pool_restore(pool_chk); continue; }

        if (unify(goals[0], ren_caput, s)) {
            Terminus *newgoals[MAX_GOALS];
            int m = 0;
            int ok = 1;
            for (int i = 0; i < cl->n_corpus && m < MAX_GOALS; i++) {
                Terminus *r = term_renova(cl->corpus[i], offset);
                if (!r) { ok = 0; break; }
                newgoals[m++] = r;
            }
            for (int i = 1; i < n_goals && m < MAX_GOALS; i++)
                newgoals[m++] = goals[i];

            if (ok) resolve(newgoals, m, s, depth + 1);
        }

        s->n = subst_chk;
        pool_restore(pool_chk);

        if (solutiones_inventae >= solutiones_max) return 1;
    }
    return solutiones_inventae > 0;
}

int
resolutor_quaere(Terminus *quaestio,
                 int n_query_vars,
                 const int *query_var_ids,
                 const char *const *query_var_names,
                 int max_sol)
{
    solutiones_inventae = 0;
    solutiones_max = (max_sol > 0 && max_sol <= MAX_SOLUTIONUM)
                     ? max_sol : MAX_SOLUTIONUM;
    n_qv     = n_query_vars;
    qv_ids   = query_var_ids;
    qv_names = query_var_names;

    Subst s;
    subst_init(&s);

    Terminus *goals[1] = { quaestio };
    int pool_chk = pool_top();
    resolve(goals, 1, &s, 0);
    pool_restore(pool_chk);

    if (solutiones_inventae == 0)
        printf("  (nulla solutio)\n");
    return solutiones_inventae;
}
