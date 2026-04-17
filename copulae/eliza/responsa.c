/*
 * responsa.c — Tabulae responsorum per bucket; electio per semen.
 *
 * Aggregatum `responsa_generica[]` est externe visibile; ceterae
 * tabulae sunt static et solum per functionem `responsum_elige`
 * attingibiles.
 */
#include "eliza.h"
#include <stddef.h>

static const char *const familiaria[] = {
    "Dic mihi plus de familia tua.",
    "Quam diu ita fuit inter vos?",
    "Cogita de necessitudinibus tuis familiaribus.",
};

static const char *const sensus[] = {
    "Cur sentis ita?",
    "Quando illud sentire coepisti?",
    "Semperne sic sentis, an interdum?",
};

static const char *const credulitas[] = {
    "Cur id credis?",
    "Quid te ad hanc opinionem movit?",
    "Si id non crederes, quid accideret?",
};

static const char *const timor[] = {
    "Quae vere timere videris?",
    "Quam graviter te afficit hic timor?",
    "Timor saepe umbra cupiditatis est: quid cupis?",
};

static const char *const amor[] = {
    "Quid amare tibi significat?",
    "Dic mihi plura de eo quod amas.",
    "Amor et dolor saepe gemini sunt.",
};

static const char *const odium[] = {
    "Odium quid in te excitat?",
    "Quomodo se habet haec aversio?",
    "Odisti vere, an dolor loquitur?",
};

static const char *const cupiditas[] = {
    "Quid impedit ne hoc consequaris?",
    "Quid acciderit si voluntatem tuam obtineas?",
    "Cupido saepe veritatem celat: age, age.",
};

static const char *const somnia[] = {
    "Quid in somniis vides?",
    "Freud dixit somnia viam regiam in inconscium esse.",
    "Somnia sunt epistulae animae: lege tuum.",
};

static const char *const apparentiae[] = {
    "Videri est scire?",
    "Quam certus es de hac apparentia?",
    "Verum an falsum distinguere potes?",
};

static const char *const modalia[] = {
    "Difficile est rem ita describere.",
    "Quomodo id ad te pertinet?",
    "Quis modus tibi maxime placet?",
};

static const char *const causae[] = {
    "Quid si causam non invenires?",
    "Semper causam quaeris?",
    "Non omnia causas habent: quid tum?",
};

static const char *const quaestiones[] = {
    "Responsum ipse iam scis, puto.",
    "Quid tu respondebis?",
    "Quaestio tua me interrogat: quid tibi?",
};

static const char *const absoluta[] = {
    "Vere nemo? Nullusne omnino?",
    "Semper? Usque nunc?",
    "Absoluta verba rarum veritatem tenent.",
};

static const char *const privationes[] = {
    "Nihil prorsus? Narra mihi plus.",
    "Nihil omnino? Quomodo te hoc afficit?",
    "Inter nihil et aliquid quid iacet?",
};

const char *const responsa_generica[] = {
    "Narra mihi plus.",
    "Intellego. Perge.",
    "Quid ad hanc rem sentis?",
    "Prosequere.",
    "Dic maiora.",
    "Ita.",
};
const int N_GENERICORUM =
    (int)(sizeof responsa_generica / sizeof responsa_generica[0]);

typedef struct {
    const char *const *arr;
    int n;
} Bucket;

static const Bucket buckets[] = {
    { familiaria,  (int)(sizeof familiaria  / sizeof familiaria[0])  },
    { sensus,      (int)(sizeof sensus      / sizeof sensus[0])      },
    { credulitas,  (int)(sizeof credulitas  / sizeof credulitas[0])  },
    { timor,       (int)(sizeof timor       / sizeof timor[0])       },
    { amor,        (int)(sizeof amor        / sizeof amor[0])        },
    { odium,       (int)(sizeof odium       / sizeof odium[0])       },
    { cupiditas,   (int)(sizeof cupiditas   / sizeof cupiditas[0])   },
    { somnia,      (int)(sizeof somnia      / sizeof somnia[0])      },
    { apparentiae, (int)(sizeof apparentiae / sizeof apparentiae[0]) },
    { modalia,     (int)(sizeof modalia     / sizeof modalia[0])     },
    { causae,      (int)(sizeof causae      / sizeof causae[0])      },
    { quaestiones, (int)(sizeof quaestiones / sizeof quaestiones[0]) },
    { absoluta,    (int)(sizeof absoluta    / sizeof absoluta[0])    },
    { absoluta,    (int)(sizeof absoluta    / sizeof absoluta[0])    },
    { privationes, (int)(sizeof privationes / sizeof privationes[0]) },
    { responsa_generica, 0 /* patched at runtime */ },
};

const char *
responsum_elige(int bucket, unsigned seed)
{
    int nb = (int)(sizeof buckets / sizeof buckets[0]);
    if (bucket < 0 || bucket >= nb) bucket = nb - 1;
    const Bucket *b = &buckets[bucket];
    int n = b->n ? b->n : N_GENERICORUM;
    int i = (int)(seed % (unsigned)n);
    return b->arr[i];
}
