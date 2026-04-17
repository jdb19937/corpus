/*
 * declinatio_tab.c — Tabulae declinationum 1ae et 2ae.
 *
 * Aggregata extern usurpata ex agnosco.c. Suffixi longiores prius
 * locantur ut correcte praeferantur in matching (e.g. "arum" ante "a").
 */
#include "grammaticus.h"

const Finitio declinatio1f[] = {
    { CAT_DECL1F, "arum", "gen pl F"                },
    { CAT_DECL1F, "abus", "dat/abl pl F (rarum)"    },
    { CAT_DECL1F, "am",   "acc sg F"                },
    { CAT_DECL1F, "ae",   "gen/dat sg F; nom/voc pl F" },
    { CAT_DECL1F, "as",   "acc pl F"                },
    { CAT_DECL1F, "is",   "dat/abl pl F"            },
    { CAT_DECL1F, "a",    "nom/abl/voc sg F"        },
};
const int N_DECL1F = (int)(sizeof declinatio1f / sizeof declinatio1f[0]);

const Finitio declinatio2m[] = {
    { CAT_DECL2M, "orum", "gen pl M"                },
    { CAT_DECL2M, "um",   "acc sg M"                },
    { CAT_DECL2M, "us",   "nom sg M"                },
    { CAT_DECL2M, "os",   "acc pl M"                },
    { CAT_DECL2M, "is",   "dat/abl pl M"            },
    { CAT_DECL2M, "o",    "dat/abl sg M"            },
    { CAT_DECL2M, "i",    "gen sg M; nom/voc pl M"  },
    { CAT_DECL2M, "e",    "voc sg M"                },
};
const int N_DECL2M = (int)(sizeof declinatio2m / sizeof declinatio2m[0]);

const Finitio declinatio2n[] = {
    { CAT_DECL2N, "orum", "gen pl N"                },
    { CAT_DECL2N, "um",   "nom/acc/voc sg N"        },
    { CAT_DECL2N, "is",   "dat/abl pl N"            },
    { CAT_DECL2N, "o",    "dat/abl sg N"            },
    { CAT_DECL2N, "i",    "gen sg N"                },
    { CAT_DECL2N, "a",    "nom/acc/voc pl N"        },
};
const int N_DECL2N = (int)(sizeof declinatio2n / sizeof declinatio2n[0]);
