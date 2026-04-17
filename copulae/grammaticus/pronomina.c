/*
 * pronomina.c — Formae pronominum fixae (non ex regula derivatae).
 *
 * Pronomina personalia (ego/tu/nos/vos), demonstrativa (hic/ille/is),
 * et relativum qui. Forma stricta requiritur (non suffix-matching).
 */
#include "grammaticus.h"

const Pronomen pronomina[] = {
    { "ego",    "ego",    "1p sg nom"                   },
    { "mei",    "ego",    "1p sg gen"                   },
    { "mihi",   "ego",    "1p sg dat"                   },
    { "me",     "ego",    "1p sg acc/abl"               },
    { "nos",    "nos",    "1p pl nom/acc"               },
    { "nostri", "nos",    "1p pl gen"                   },
    { "nobis",  "nos",    "1p pl dat/abl"               },
    { "tu",     "tu",     "2p sg nom"                   },
    { "tui",    "tu",     "2p sg gen"                   },
    { "tibi",   "tu",     "2p sg dat"                   },
    { "te",     "tu",     "2p sg acc/abl"               },
    { "vos",    "vos",    "2p pl nom/acc"               },
    { "vestri", "vos",    "2p pl gen"                   },
    { "vobis",  "vos",    "2p pl dat/abl"               },

    { "hic",    "hic",    "dem M nom sg"                },
    { "haec",   "hic",    "dem F nom sg; N nom/acc pl"  },
    { "hoc",    "hic",    "dem N nom/acc sg; M/N abl sg"},
    { "huius",  "hic",    "dem gen sg omnis generis"    },
    { "huic",   "hic",    "dem dat sg omnis generis"    },
    { "hunc",   "hic",    "dem M acc sg"                },
    { "hanc",   "hic",    "dem F acc sg"                },
    { "his",    "hic",    "dem dat/abl pl"              },

    { "ille",   "ille",   "dem M nom sg"                },
    { "illa",   "ille",   "dem F nom/abl sg; N nom/acc pl"},
    { "illud",  "ille",   "dem N nom/acc sg"            },
    { "illum",  "ille",   "dem M acc sg"                },
    { "illi",   "ille",   "dem gen/dat sg; M nom pl"    },

    { "is",     "is",     "dem M nom sg"                },
    { "ea",     "is",     "dem F nom/abl sg; N nom/acc pl"},
    { "id",     "is",     "dem N nom/acc sg"            },
    { "eum",    "is",     "dem M acc sg"                },
    { "eius",   "is",     "dem gen sg omnis generis"    },
    { "ei",     "is",     "dem dat sg; M nom pl"        },

    { "qui",    "qui",    "rel M nom sg; M nom pl"      },
    { "quae",   "qui",    "rel F nom sg; N nom/acc pl; F nom pl"},
    { "quod",   "qui",    "rel N nom/acc sg"            },
    { "quem",   "qui",    "rel M acc sg"                },
    { "cuius",  "qui",    "rel gen sg omnis generis"    },
    { "cui",    "qui",    "rel dat sg omnis generis"    },
};

const int N_PRONOMINA = (int)(sizeof pronomina / sizeof pronomina[0]);
