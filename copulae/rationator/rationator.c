/*
 * rationator.c — Mini-Prolog: SLD backward-chaining cum unificatione.
 *
 * Speculum inversum ad `peritus` (forward ↔ backward chaining).
 * Basis clausularum: genealogia Aeneidis (Anchises, Venus, Aeneas,
 * Ascanius, Silvius, Latinus, Lavinia) et regulae derivativae
 * (`avus/2`, `ortus/2` per recursionem).
 *
 * Officia linker: recursio mutua trans TU (walk in unificatio.c,
 * term_renova in terminus.c, resolve in resolutor.c); tabula
 * clausularum in baselas.c definita et legata ex resolutor.c.
 */
#include "rationator.h"
#include <stdio.h>
#include <string.h>

static void
construe_basim(void)
{
    /* Facta: genealogia ex Aeneide */
    clausula_fact(term_cmpd("parens", 2,
                            term_atom("anchises"), term_atom("aeneas"),
                            NULL, NULL));
    clausula_fact(term_cmpd("parens", 2,
                            term_atom("venus"), term_atom("aeneas"),
                            NULL, NULL));
    clausula_fact(term_cmpd("parens", 2,
                            term_atom("aeneas"), term_atom("ascanius"),
                            NULL, NULL));
    clausula_fact(term_cmpd("parens", 2,
                            term_atom("aeneas"), term_atom("silvius"),
                            NULL, NULL));
    clausula_fact(term_cmpd("parens", 2,
                            term_atom("ascanius"), term_atom("iulus"),
                            NULL, NULL));
    clausula_fact(term_cmpd("parens", 2,
                            term_atom("latinus"), term_atom("lavinia"),
                            NULL, NULL));

    clausula_fact(term_cmpd("mas", 1, term_atom("anchises"), NULL, NULL, NULL));
    clausula_fact(term_cmpd("mas", 1, term_atom("aeneas"),   NULL, NULL, NULL));
    clausula_fact(term_cmpd("mas", 1, term_atom("ascanius"), NULL, NULL, NULL));
    clausula_fact(term_cmpd("mas", 1, term_atom("silvius"),  NULL, NULL, NULL));
    clausula_fact(term_cmpd("mas", 1, term_atom("iulus"),    NULL, NULL, NULL));
    clausula_fact(term_cmpd("mas", 1, term_atom("latinus"),  NULL, NULL, NULL));
    clausula_fact(term_cmpd("femina", 1, term_atom("venus"),   NULL, NULL, NULL));
    clausula_fact(term_cmpd("femina", 1, term_atom("lavinia"), NULL, NULL, NULL));

    /* avus(X, Z) :- parens(X, Y), parens(Y, Z), mas(X). */
    {
        Terminus *X = term_var("X");
        Terminus *Y = term_var("Y");
        Terminus *Z = term_var("Z");
        clausula_rule(
            term_cmpd("avus", 2, X, Z, NULL, NULL), 3,
            term_cmpd("parens", 2, X, Y, NULL, NULL),
            term_cmpd("parens", 2, Y, Z, NULL, NULL),
            term_cmpd("mas", 1, X, NULL, NULL, NULL),
            NULL);
    }

    /* ortus(X, Y) :- parens(X, Y). */
    {
        Terminus *X = term_var("X");
        Terminus *Y = term_var("Y");
        clausula_rule(
            term_cmpd("ortus", 2, X, Y, NULL, NULL), 1,
            term_cmpd("parens", 2, X, Y, NULL, NULL),
            NULL, NULL, NULL);
    }

    /* ortus(X, Z) :- parens(X, Y), ortus(Y, Z). */
    {
        Terminus *X = term_var("X");
        Terminus *Y = term_var("Y");
        Terminus *Z = term_var("Z");
        clausula_rule(
            term_cmpd("ortus", 2, X, Z, NULL, NULL), 2,
            term_cmpd("parens", 2, X, Y, NULL, NULL),
            term_cmpd("ortus", 2, Y, Z, NULL, NULL),
            NULL, NULL);
    }
}

static void
age_quaestionem(const char *descr, Terminus *q,
                int nqv, const int *ids, const char *const *names)
{
    printf("\n?- ");
    term_imprime(q);
    printf(".   (%s)\n", descr);
    resolutor_quaere(q, nqv, ids, names, MAX_SOLUTIONUM);
}

int
main(void)
{
    terminus_init();
    construe_basim();

    printf("=== Rationator (mini-Prolog) ===\n\n");
    baselas_imprime();

    /* ?- parens(X, aeneas). */
    {
        Terminus *X = term_var("X");
        int ids[1] = { X->var_id };
        const char *names[1] = { "X" };
        age_quaestionem("quis parens Aeneae?",
                        term_cmpd("parens", 2, X, term_atom("aeneas"), NULL, NULL),
                        1, ids, names);
    }

    /* ?- parens(aeneas, Y). */
    {
        Terminus *Y = term_var("Y");
        int ids[1] = { Y->var_id };
        const char *names[1] = { "Y" };
        age_quaestionem("cuius parens Aeneas?",
                        term_cmpd("parens", 2, term_atom("aeneas"), Y, NULL, NULL),
                        1, ids, names);
    }

    /* ?- avus(X, iulus). */
    {
        Terminus *X = term_var("X");
        int ids[1] = { X->var_id };
        const char *names[1] = { "X" };
        age_quaestionem("quis avus Iuli?",
                        term_cmpd("avus", 2, X, term_atom("iulus"), NULL, NULL),
                        1, ids, names);
    }

    /* ?- ortus(anchises, Y). */
    {
        Terminus *Y = term_var("Y");
        int ids[1] = { Y->var_id };
        const char *names[1] = { "Y" };
        age_quaestionem("qui ex Anchise orti sunt?",
                        term_cmpd("ortus", 2, term_atom("anchises"), Y, NULL, NULL),
                        1, ids, names);
    }

    /* ?- parens(X, Y), femina(X). */
    {
        Terminus *X = term_var("X");
        Terminus *Y = term_var("Y");
        int ids[2] = { X->var_id, Y->var_id };
        const char *names[2] = { "X", "Y" };
        /* Construimus conjunctionem per regulam implicitam: */
        /* Simpliciter vocamus parens cum tunc check femina in resolutor?
           NON: ad hoc adhibemus clausulam specialem. */
        clausula_rule(
            term_cmpd("parensMater", 2, X, Y, NULL, NULL), 2,
            term_cmpd("parens", 2, X, Y, NULL, NULL),
            term_cmpd("femina", 1, X, NULL, NULL, NULL),
            NULL, NULL);
        age_quaestionem("quae mater cuius?",
                        term_cmpd("parensMater", 2, X, Y, NULL, NULL),
                        2, ids, names);
    }

    printf("\n");
    return 0;
}
