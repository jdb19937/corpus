/*
 * coniugatio_tab.c — Tabulae coniugationum 1ae et 2ae.
 *
 * Praesens activum indicativum + perfectum + imperfectum + infinitivus
 * pro utraque coniugatione. Suffixi longiores prius.
 */
#include "grammaticus.h"

const Finitio coniugatio1[] = {
    { CAT_CONJ1, "abantur","3p pl imperf pass ind (1a)"    },
    { CAT_CONJ1, "averunt","3p pl perf act ind (1a)"       },
    { CAT_CONJ1, "aberunt","3p pl fut act ind (1a)"        },
    { CAT_CONJ1, "abamus", "1p pl imperf act ind (1a)"     },
    { CAT_CONJ1, "abatis", "2p pl imperf act ind (1a)"     },
    { CAT_CONJ1, "abunt",  "3p pl fut act ind (1a)"        },
    { CAT_CONJ1, "abant",  "3p pl imperf act ind (1a)"     },
    { CAT_CONJ1, "avit",   "3p sg perf act ind (1a)"       },
    { CAT_CONJ1, "avi",    "1p sg perf act ind (1a)"       },
    { CAT_CONJ1, "abam",   "1p sg imperf act ind (1a)"     },
    { CAT_CONJ1, "abas",   "2p sg imperf act ind (1a)"     },
    { CAT_CONJ1, "abat",   "3p sg imperf act ind (1a)"     },
    { CAT_CONJ1, "are",    "praes inf act (1a)"            },
    { CAT_CONJ1, "amus",   "1p pl praes act ind (1a)"      },
    { CAT_CONJ1, "atis",   "2p pl praes act ind (1a)"      },
    { CAT_CONJ1, "ant",    "3p pl praes act ind (1a)"      },
    { CAT_CONJ1, "as",     "2p sg praes act ind (1a)"      },
    { CAT_CONJ1, "at",     "3p sg praes act ind (1a)"      },
    { CAT_CONJ1, "o",      "1p sg praes act ind (1a)"      },
};
const int N_CONJ1 = (int)(sizeof coniugatio1 / sizeof coniugatio1[0]);

const Finitio coniugatio2[] = {
    { CAT_CONJ2, "ebamus", "1p pl imperf act ind (2a)"     },
    { CAT_CONJ2, "ebant",  "3p pl imperf act ind (2a)"     },
    { CAT_CONJ2, "ebam",   "1p sg imperf act ind (2a)"     },
    { CAT_CONJ2, "ebat",   "3p sg imperf act ind (2a)"     },
    { CAT_CONJ2, "emus",   "1p pl praes act ind (2a)"      },
    { CAT_CONJ2, "etis",   "2p pl praes act ind (2a)"      },
    { CAT_CONJ2, "ent",    "3p pl praes act ind (2a)"      },
    { CAT_CONJ2, "ere",    "praes inf act (2a)"            },
    { CAT_CONJ2, "es",     "2p sg praes act ind (2a)"      },
    { CAT_CONJ2, "et",     "3p sg praes act ind (2a)"      },
    { CAT_CONJ2, "eo",     "1p sg praes act ind (2a)"      },
};
const int N_CONJ2 = (int)(sizeof coniugatio2 / sizeof coniugatio2[0]);
