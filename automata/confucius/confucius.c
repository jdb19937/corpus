/*
 * confucius.c — Automatum Analectorum Magistri Kung
 *
 * Simulator dialogi in modo Analectorum (Lun Yu): MAGISTER CONFUCIUS
 * cum discipulo colloquitur. Dicta magistri selecta ex corpore
 * sententiarum per congruentiam argumenti; responsio instruitur
 * tribus partibus: invocatione, dicto, et commentario.
 *
 * Semen generatoris xorshift64 ex argumento '-s N' vel ex constante
 * defalta. Argumentum '-i' aperit colloquium interactivum; sine
 * argumentis, dialogus scriptus cum quattuordecim responsis
 * producitur. Vexilla ignota scribunt usum in stderr.
 *
 * C99 solummodo; nihil extra bibliothecam standardam.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdarg.h>

/* =========================================================== */
/* Constantes et macra                                          */
/* =========================================================== */

#define SEMEN_DEFALTUM       ((uint64_t)0x00C0DFC1A5ULL)
#define MAX_LINEAE           512
#define MAX_VERBI             64
#define MAX_VERBORUM         128
#define MAX_RESPONSI        2048
#define MAX_DICTORUM          96
#define MAX_CANDIDATORUM      96
#define MAX_NOMINA            32
#define NUMERUS_DIALOGORUM    16

#define INVOCATIO_VIRTUS      0u
#define INVOCATIO_DOCTA       1u
#define INVOCATIO_SENEX       2u
#define INVOCATIO_BARBARA     3u
#define INVOCATIO_NUMERUS     4u

#define SUSURRA(...)          fprintf(stdout, __VA_ARGS__)
#define ERRORA(...)           fprintf(stderr, __VA_ARGS__)
#define ARRAY_LONGITUDO(A)    ((int)(sizeof(A) / sizeof((A)[0])))

/* =========================================================== */
/* Argumenta (bitmask): topica dictorum                         */
/* =========================================================== */

enum Argumentum {
    ARG_VIRTUS     = 1u <<  0,
    ARG_PIETAS     = 1u <<  1,
    ARG_HUMANITAS  = 1u <<  2,
    ARG_IUSTITIA   = 1u <<  3,
    ARG_RITUS      = 1u <<  4,
    ARG_DOCTRINA   = 1u <<  5,
    ARG_RECTOR     = 1u <<  6,
    ARG_DISCIPULUS = 1u <<  7,
    ARG_FAMILIA    = 1u <<  8,
    ARG_MUSICA     = 1u <<  9,
    ARG_SENES      = 1u << 10,
    ARG_IUVENES    = 1u << 11,
    ARG_MORS       = 1u << 12,
    ARG_AMICITIA   = 1u << 13,
    ARG_SILENTIUM  = 1u << 14
};

typedef uint32_t Tessera;

/* =========================================================== */
/* Generator pseudo-fortuitus: xorshift64                       */
/* =========================================================== */

typedef struct {
    uint64_t status;
} Fors;

static inline uint64_t
xorshift64(Fors *restrict f)
{
    uint64_t x = f->status;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    f->status = x;
    return x;
}

static inline uint32_t
fortuita_modo(Fors *restrict f, uint32_t limes)
{
    if (limes == 0) return 0u;
    return (uint32_t)(xorshift64(f) % (uint64_t)limes);
}

static inline int
fortuita_signum(Fors *restrict f)
{
    return (int)(xorshift64(f) & 1ULL);
}

/* =========================================================== */
/* Clavis argumentorum: verba Latina quae argumenta excitant     */
/* =========================================================== */

typedef struct {
    const char *verbum;
    Tessera     tessera;
} ClavisArgumenti;

static const ClavisArgumenti claves[] = {
    { "virtus",       ARG_VIRTUS     },
    { "virtutem",     ARG_VIRTUS     },
    { "virtute",      ARG_VIRTUS     },
    { "virtutis",     ARG_VIRTUS     },
    { "pietas",       ARG_PIETAS     },
    { "pietatem",     ARG_PIETAS     },
    { "pietate",      ARG_PIETAS     },
    { "parens",       ARG_PIETAS     },
    { "parentes",     ARG_PIETAS     },
    { "patrem",       ARG_FAMILIA    },
    { "pater",        ARG_FAMILIA    },
    { "matrem",       ARG_FAMILIA    },
    { "mater",        ARG_FAMILIA    },
    { "filius",       ARG_FAMILIA    },
    { "filii",        ARG_FAMILIA    },
    { "frater",       ARG_FAMILIA    },
    { "humanitas",    ARG_HUMANITAS  },
    { "humanitatem",  ARG_HUMANITAS  },
    { "homo",         ARG_HUMANITAS  },
    { "homines",      ARG_HUMANITAS  },
    { "benevolentia", ARG_HUMANITAS  },
    { "iustitia",     ARG_IUSTITIA   },
    { "iustum",       ARG_IUSTITIA   },
    { "iusti",        ARG_IUSTITIA   },
    { "iusta",        ARG_IUSTITIA   },
    { "iustitiam",    ARG_IUSTITIA   },
    { "ritus",        ARG_RITUS      },
    { "ritum",        ARG_RITUS      },
    { "ritibus",      ARG_RITUS      },
    { "caerimonia",   ARG_RITUS      },
    { "doctrina",     ARG_DOCTRINA   },
    { "doctrinam",    ARG_DOCTRINA   },
    { "discere",      ARG_DOCTRINA   },
    { "studium",      ARG_DOCTRINA   },
    { "libri",        ARG_DOCTRINA   },
    { "liber",        ARG_DOCTRINA   },
    { "magister",     ARG_DOCTRINA   },
    { "princeps",     ARG_RECTOR     },
    { "rex",          ARG_RECTOR     },
    { "regem",        ARG_RECTOR     },
    { "rector",       ARG_RECTOR     },
    { "imperium",     ARG_RECTOR     },
    { "gubernare",    ARG_RECTOR     },
    { "discipulus",   ARG_DISCIPULUS },
    { "discipuli",    ARG_DISCIPULUS },
    { "discipulum",   ARG_DISCIPULUS },
    { "familia",      ARG_FAMILIA    },
    { "domus",        ARG_FAMILIA    },
    { "musica",       ARG_MUSICA     },
    { "cantus",       ARG_MUSICA     },
    { "carmen",       ARG_MUSICA     },
    { "lyra",         ARG_MUSICA     },
    { "senex",        ARG_SENES      },
    { "senes",        ARG_SENES      },
    { "anus",         ARG_SENES      },
    { "aetas",        ARG_SENES      },
    { "iuvenis",      ARG_IUVENES    },
    { "iuvenes",      ARG_IUVENES    },
    { "adulescens",   ARG_IUVENES    },
    { "mors",         ARG_MORS       },
    { "mortem",       ARG_MORS       },
    { "defunctus",    ARG_MORS       },
    { "sepulcrum",    ARG_MORS       },
    { "amicus",       ARG_AMICITIA   },
    { "amici",        ARG_AMICITIA   },
    { "amicitia",     ARG_AMICITIA   },
    { "silentium",    ARG_SILENTIUM  },
    { "tacere",       ARG_SILENTIUM  }
};

/* =========================================================== */
/* Corpus dictorum: X-macrum                                    */
/*                                                              */
/* DICTUM(tessera, primum_argumentum, textus)                   */
/* =========================================================== */

#define CORPUS_DICTORUM \
    DICTUM(ARG_DOCTRINA | ARG_VIRTUS,                              ARG_DOCTRINA, \
        "Discere et saepe exercere — nonne hoc etiam iucundum?") \
    DICTUM(ARG_AMICITIA,                                           ARG_AMICITIA, \
        "Amicus ex longinquo venit — nonne hoc etiam laetum?") \
    DICTUM(ARG_HUMANITAS | ARG_SILENTIUM,                          ARG_HUMANITAS, \
        "Qui ignorat et non irascitur — hic vir bonus est.") \
    DICTUM(ARG_PIETAS | ARG_FAMILIA,                               ARG_PIETAS, \
        "Qui pietatem parentibus servat, radicem virtutis tenet.") \
    DICTUM(ARG_PIETAS | ARG_FAMILIA | ARG_SENES,                   ARG_PIETAS, \
        "Cum pater vivit, observa eius voluntatem; cum mortuus est, observa eius opera.") \
    DICTUM(ARG_VIRTUS | ARG_HUMANITAS,                             ARG_VIRTUS, \
        "Vir nobilis de officio cogitat; homuncio de emolumento.") \
    DICTUM(ARG_VIRTUS,                                             ARG_VIRTUS, \
        "Vir nobilis quieto animo latus est; homuncio sollicito animo contractus.") \
    DICTUM(ARG_VIRTUS | ARG_HUMANITAS,                             ARG_HUMANITAS, \
        "Noli alii facere quod tibi fieri non vis.") \
    DICTUM(ARG_DOCTRINA,                                           ARG_DOCTRINA, \
        "Discere sine cogitando inutile est; cogitare sine discendo periculosum.") \
    DICTUM(ARG_DOCTRINA | ARG_DISCIPULUS,                          ARG_DOCTRINA, \
        "Tres eunt — magister meus inter eos certe invenitur.") \
    DICTUM(ARG_DOCTRINA,                                           ARG_DOCTRINA, \
        "Scire quod scias et scire quod nescias — haec vera scientia est.") \
    DICTUM(ARG_RECTOR | ARG_IUSTITIA,                              ARG_RECTOR, \
        "Rector per virtutem est sicut stella septentrionalis: manet in loco, ceterae circumvolvunt.") \
    DICTUM(ARG_RECTOR | ARG_RITUS,                                 ARG_RECTOR, \
        "Regere per legem punit populum; regere per ritum corrigit cor.") \
    DICTUM(ARG_RECTOR,                                             ARG_RECTOR, \
        "Si rector ipse rectus est, sine iussu obeditur.") \
    DICTUM(ARG_RITUS | ARG_HUMANITAS,                              ARG_RITUS, \
        "Sine humanitate, quid valet ritus? Sine humanitate, quid valet musica?") \
    DICTUM(ARG_RITUS | ARG_VIRTUS,                                 ARG_RITUS, \
        "Vir nobilis litteris eruditur, ritibus continetur.") \
    DICTUM(ARG_MUSICA | ARG_VIRTUS,                                ARG_MUSICA, \
        "Excitor carmine, confirmor ritibus, perficior musica.") \
    DICTUM(ARG_MUSICA,                                             ARG_MUSICA, \
        "Audita musica nobili tribus mensibus carnis saporem non novi.") \
    DICTUM(ARG_HUMANITAS | ARG_VIRTUS,                             ARG_HUMANITAS, \
        "Humanitas procul est? Si eam volo, adest.") \
    DICTUM(ARG_HUMANITAS,                                          ARG_HUMANITAS, \
        "Homo humanus suas sollicitudines sibi servat, amicis bona tribuit.") \
    DICTUM(ARG_IUSTITIA | ARG_VIRTUS,                              ARG_IUSTITIA, \
        "Vir nobilis pauperiem fert sine querela; homuncio extra leges volvitur.") \
    DICTUM(ARG_IUSTITIA,                                           ARG_IUSTITIA, \
        "Aurum et dignitates iniuste parta mihi sunt ut nubes transiens.") \
    DICTUM(ARG_SENES | ARG_PIETAS,                                 ARG_SENES, \
        "Senibus quietem, amicis fidem, iuvenibus curam — talis sit animi votum.") \
    DICTUM(ARG_IUVENES | ARG_DOCTRINA,                             ARG_IUVENES, \
        "Iuvenes timendi sunt: quis novit quin futuri pares nostri sint?") \
    DICTUM(ARG_IUVENES,                                            ARG_IUVENES, \
        "Qui ad quadraginta annos pervenit odio habitus, in illo finis est.") \
    DICTUM(ARG_MORS,                                               ARG_MORS, \
        "Nondum vitam novi — quomodo mortem noverim?") \
    DICTUM(ARG_MORS | ARG_PIETAS,                                  ARG_MORS, \
        "Vivos servire nescis — quomodo manibus servies?") \
    DICTUM(ARG_AMICITIA | ARG_VIRTUS,                              ARG_AMICITIA, \
        "Tres amici utiles: rectus, fidelis, multiscius.") \
    DICTUM(ARG_AMICITIA,                                           ARG_AMICITIA, \
        "Tres amici noxii: adulator, blandus, loquax.") \
    DICTUM(ARG_SILENTIUM | ARG_DOCTRINA,                           ARG_SILENTIUM, \
        "Antiqui verba sua parce dabant, ne opera sua verbis impares essent.") \
    DICTUM(ARG_SILENTIUM | ARG_VIRTUS,                             ARG_SILENTIUM, \
        "Vir nobilis verbis tardus, operibus celer.") \
    DICTUM(ARG_VIRTUS | ARG_HUMANITAS,                             ARG_VIRTUS, \
        "Qui suam virtutem emendat, non vicinum cogitat.") \
    DICTUM(ARG_DOCTRINA | ARG_VIRTUS,                              ARG_DOCTRINA, \
        "Qui didicit et memoria retinet, dignus est docere.") \
    DICTUM(ARG_DOCTRINA,                                           ARG_DOCTRINA, \
        "Per antiqua nova quaere — sic licet magister fieri.") \
    DICTUM(ARG_FAMILIA | ARG_PIETAS,                               ARG_FAMILIA, \
        "Domi filius pius, foris iuvenis reverens — radix humanitatis.") \
    DICTUM(ARG_FAMILIA,                                            ARG_FAMILIA, \
        "Fratres concordes intra limina, pater quieto corde sedet.") \
    DICTUM(ARG_PIETAS,                                             ARG_PIETAS, \
        "Tres annos a patris sepulcro non discedas a via eius — pietas vocatur.") \
    DICTUM(ARG_RECTOR | ARG_VIRTUS,                                ARG_RECTOR, \
        "Rector ut ventus, plebes ut herba: quo ventus, eo herba inclinat.") \
    DICTUM(ARG_RECTOR | ARG_IUSTITIA,                              ARG_RECTOR, \
        "Nomina rectificentur: si nomen non congruit, verbum non convenit.") \
    DICTUM(ARG_IUSTITIA | ARG_RECTOR,                              ARG_IUSTITIA, \
        "Sine fide populi, status non stat.") \
    DICTUM(ARG_RITUS | ARG_SENES,                                  ARG_RITUS, \
        "Ritus parentum in vita et in morte aequaliter servandus.") \
    DICTUM(ARG_RITUS,                                              ARG_RITUS, \
        "In ritibus funeris dolor superet artem; in aliis, temperantia.") \
    DICTUM(ARG_MUSICA | ARG_RITUS,                                 ARG_MUSICA, \
        "Cum ritu et musica res publica regatur — sic antiqui docuerunt.") \
    DICTUM(ARG_MUSICA | ARG_DOCTRINA,                              ARG_MUSICA, \
        "Melos rectum animum erigit; melos pravum animum corrumpit.") \
    DICTUM(ARG_HUMANITAS | ARG_AMICITIA,                           ARG_HUMANITAS, \
        "Homo humanus alios erigere vult cum seipso erigitur.") \
    DICTUM(ARG_HUMANITAS | ARG_DOCTRINA,                           ARG_HUMANITAS, \
        "Humanitatis semen parvum: sed ex eo tota arbor virtutis crescit.") \
    DICTUM(ARG_VIRTUS | ARG_DISCIPULUS,                            ARG_VIRTUS, \
        "Vir nobilis non vas est: nulli uni usui addictus.") \
    DICTUM(ARG_VIRTUS | ARG_IUSTITIA,                              ARG_VIRTUS, \
        "Vir nobilis non partiarius est; tenax iusti, aversus fraudis.") \
    DICTUM(ARG_DISCIPULUS | ARG_DOCTRINA,                          ARG_DISCIPULUS, \
        "Non doleam si non agnoscor; doleam si aliquem non agnovi.") \
    DICTUM(ARG_DISCIPULUS | ARG_DOCTRINA,                          ARG_DISCIPULUS, \
        "Quindecim annorum animum ad studium applicui; triginta constiti.") \
    DICTUM(ARG_DISCIPULUS,                                         ARG_DISCIPULUS, \
        "Quadraginta annos natus non dubitavi; quinquaginta, caeli iussa cognovi.") \
    DICTUM(ARG_SENES,                                              ARG_SENES, \
        "Sexaginta annos natus, auris mea obediens facta est.") \
    DICTUM(ARG_SENES | ARG_VIRTUS,                                 ARG_SENES, \
        "Septuaginta annos natus, cor sequar nec mensuram transgrediar.") \
    DICTUM(ARG_IUVENES | ARG_RITUS,                                ARG_IUVENES, \
        "Iuvenis domi pius sit, foris reverens, parcus verbis, studiosus litterarum.") \
    DICTUM(ARG_MORS | ARG_RITUS,                                   ARG_MORS, \
        "Sacrificium absentibus quasi praesentibus: si absum animo, non sacrifico.") \
    DICTUM(ARG_SILENTIUM | ARG_HUMANITAS,                          ARG_SILENTIUM, \
        "Caelum loquitur? Quattuor tempora currunt, centum res oriuntur — caelum tacet.") \
    DICTUM(ARG_AMICITIA | ARG_DOCTRINA,                            ARG_AMICITIA, \
        "Cum amico non egeris perfidia — hoc est fides.") \
    DICTUM(ARG_IUSTITIA | ARG_HUMANITAS,                           ARG_IUSTITIA, \
        "Noli alienum postulare; tuum iuste serva.") \
    DICTUM(ARG_FAMILIA | ARG_SENES,                                ARG_FAMILIA, \
        "Parentum aetatem scire debet filius — gaudio et timore simul.") \
    DICTUM(ARG_VIRTUS | ARG_DOCTRINA | ARG_RITUS,                  ARG_VIRTUS, \
        "Per se vincere et ad ritum redire — hoc est humanitas.") \
    DICTUM(ARG_DOCTRINA | ARG_SILENTIUM,                           ARG_DOCTRINA, \
        "Non indicabo nisi cupido: unum angulum ostendo, tres reddat discipulus.") \
    DICTUM(ARG_RECTOR | ARG_FAMILIA,                               ARG_RECTOR, \
        "Si res publica recte fiat, domus rectae stabunt.") \
    DICTUM(ARG_HUMANITAS | ARG_DISCIPULUS,                         ARG_HUMANITAS, \
        "Vir humanus primum laborat, postea fructus colligit.")

typedef struct {
    Tessera     tesserae;
    Tessera     primum;
    const char *textus;
} Dictum;

static const Dictum corpus_dictorum[] = {
#define DICTUM(t, p, s) { (t), (p), (s) },
    CORPUS_DICTORUM
#undef DICTUM
};

#define NUMERUS_DICTORUM (ARRAY_LONGITUDO(corpus_dictorum))

/* =========================================================== */
/* Formulae invocationis                                         */
/* =========================================================== */

static const char *const invocationes_virtutis[] = {
    "Magister dicit:",
    "Olim audivi Magistrum dicentem:",
    "Confucius docuit:",
    "Magister sic cecinit:"
};

static const char *const invocationes_doctae[] = {
    "Magister interrogatus respondit:",
    "Audivi in Analectis:",
    "Magister inter discipulos dixit:",
    "Apud librum sapientiae legimus:"
};

static const char *const invocationes_senectae[] = {
    "Senex Magister meminit:",
    "Post multos annos Magister monet:",
    "Antiquorum memoria dixit Magister:",
    "Canitie plenus, Magister docuit:"
};

static const char *const invocationes_barbarae[] = {
    "Ubi ritus neglegitur, tamen Magister monet:",
    "Cum mundus errat, Magister clamat:",
    "Contra tumultum Magister sic respondit:",
    "Deficit aetas; attamen Magister docuit:"
};

/* Tabula functionum: seligit ordinem invocationum per statum */
typedef const char *const *GrexInvocationum;

static const GrexInvocationum tabula_invocationum[INVOCATIO_NUMERUS] = {
    [INVOCATIO_VIRTUS]  = invocationes_virtutis,
    [INVOCATIO_DOCTA]   = invocationes_doctae,
    [INVOCATIO_SENEX]   = invocationes_senectae,
    [INVOCATIO_BARBARA] = invocationes_barbarae
};

static const int longitudines_invocationum[INVOCATIO_NUMERUS] = {
    [INVOCATIO_VIRTUS]  = ARRAY_LONGITUDO(invocationes_virtutis),
    [INVOCATIO_DOCTA]   = ARRAY_LONGITUDO(invocationes_doctae),
    [INVOCATIO_SENEX]   = ARRAY_LONGITUDO(invocationes_senectae),
    [INVOCATIO_BARBARA] = ARRAY_LONGITUDO(invocationes_barbarae)
};

/* =========================================================== */
/* Grammatica commentariorum: pro quolibet argumento primario    */
/* =========================================================== */

static const char *const commentaria_virtutis[] = {
    "Virtus non est dos aliena — ex intimo cordis oritur.",
    "Qui virtutem quaerit, eam in se invenit; qui foris quaerit, frustra laborat.",
    "Parva virtus cotidiana maior est magno facinore unius diei.",
    "Virtutis via recta videtur angusta, sed in fine ampla fit."
};

static const char *const commentaria_pietatis[] = {
    "Pietas in parentes origo omnium officiorum.",
    "Qui parentes honorat, etiam alios senes facile veneratur.",
    "Non solum cibus et vestis — etiam reverentia pietati debetur.",
    "Sine pietate, quid distat filius a famulo mercede conducto?"
};

static const char *const commentaria_humanitatis[] = {
    "Humanitas proximo benevolentiam, sibi severitatem postulat.",
    "Qui humanitatem colit, homines ad se trahit sine verbis.",
    "Humanitas non clamore ostenditur, sed consuetudine.",
    "Si cor humanum, omnes actus humani fiunt."
};

static const char *const commentaria_iustitiae[] = {
    "Iustum praefer utili, cum ambo pugnant.",
    "Iustitia non novit personas; tantum res ponderat.",
    "Ubi iustitia regnat, etiam pauper quietus dormit.",
    "Qui iustum sequitur, nec divitias nec paupertatem timet."
};

static const char *const commentaria_ritus[] = {
    "Ritus sine cordis pietate est vasum vacuum.",
    "Per ritum animus hominis moderatur et locum suum invenit.",
    "Antiqui ritus non sunt vincula sed gradus ad caelum.",
    "Ritus ornat virtutem sicut vestis ornat corpus."
};

static const char *const commentaria_doctrinae[] = {
    "Doctrina nunquam consummatur — semper incipit.",
    "Qui discit et alios docet, bis discit.",
    "Studium sine applicatione est arbor sine radicibus.",
    "Melior dies cum libro quam mensis sine."
};

static const char *const commentaria_rectoris[] = {
    "Rector bonus exemplum praebet, non tantum iussa.",
    "Populus sequitur rectorem ut umbra sequitur corpus.",
    "Qui aliis imperat, prius sibi imperare debet.",
    "Imperium per virtutem leve est onus; per vim, grave."
};

static const char *const commentaria_discipuli[] = {
    "Discipulus bonus interrogat plus quam respondet.",
    "In silentio discipuli saepe plus est quam in verbis magistri.",
    "Qui docet se ipsum, magistrum habet patientem.",
    "Discipulus proficit non velocitate, sed constantia."
};

static const char *const commentaria_familiae[] = {
    "Domus est semen rei publicae.",
    "Quod in familia colitur, in civitate florescit.",
    "Fratres concordes muri sunt domus.",
    "Sine pace domestica, nulla pax exterior stat."
};

static const char *const commentaria_musicae[] = {
    "Musica nobilis animum componit; musica prava dissolvit.",
    "Ubi musica vera, ibi quies mentis.",
    "Carmen antiquum memoriam virorum bonorum conservat.",
    "Musica et ritus duae columnae civitatis sunt."
};

static const char *const commentaria_senum[] = {
    "Senes habent quod iuvenes non habent: tempus volutum.",
    "Canitiei reverentia floret in civitate bona.",
    "Sine senibus, iuvenes sine speculo sunt.",
    "Senectus sapiens populum coronat."
};

static const char *const commentaria_iuvenum[] = {
    "In iuvene spes est, sed etiam periculum.",
    "Iuvenis doctrinam quaerat nunc, ne sero paeniteat.",
    "Vires iuvenis sine via virtutis caecae sunt.",
    "Iuvenis studens ut stella oritur."
};

static const char *const commentaria_mortis[] = {
    "De vita cura primum; mortis ordo sequetur.",
    "Ritus funeris cor vivorum purgat.",
    "Qui vitam bene agit, mortem tranquille accipit.",
    "Mors veniat cum venerit; nos interim recte vivamus."
};

static const char *const commentaria_amicitiae[] = {
    "Amicitia fidelis rarior auro.",
    "Amicus bonus est speculum virtutis tuae.",
    "Unus amicus verus plus valet quam centum adulatores.",
    "Amicitia per fidem, non per utilitatem, nascitur."
};

static const char *const commentaria_silentii[] = {
    "Qui tacet opportune, sapiens est.",
    "Verbum semel dictum non redit.",
    "In silentio radices virtutis crescunt.",
    "Multa verba saepe parum cordis celant."
};

static const char *const commentaria_defalta[] = {
    "De hoc Magister brevius sic monere consuevit.",
    "Ita consuetudo antiqua docet.",
    "Sic disciplina traditur per aevum.",
    "Haec sunt verba quae reliquerunt maiores."
};

/* Tabula commentariorum, indexata per argumentum primarium.
 * Utimur compound literal et designated initializer. */
typedef struct {
    const char *const *versus;
    int                numerus;
} GrexCommentarii;

static GrexCommentarii selige_gregem_commentarii(Tessera primum);
static void silentium_warningum(void);

static GrexCommentarii
selige_gregem_commentarii(Tessera primum)
{
    switch (primum) {
        case ARG_VIRTUS:
            return (GrexCommentarii){ commentaria_virtutis,    ARRAY_LONGITUDO(commentaria_virtutis)    };
        case ARG_PIETAS:
            return (GrexCommentarii){ commentaria_pietatis,    ARRAY_LONGITUDO(commentaria_pietatis)    };
        case ARG_HUMANITAS:
            return (GrexCommentarii){ commentaria_humanitatis, ARRAY_LONGITUDO(commentaria_humanitatis) };
        case ARG_IUSTITIA:
            return (GrexCommentarii){ commentaria_iustitiae,   ARRAY_LONGITUDO(commentaria_iustitiae)   };
        case ARG_RITUS:
            return (GrexCommentarii){ commentaria_ritus,       ARRAY_LONGITUDO(commentaria_ritus)       };
        case ARG_DOCTRINA:
            return (GrexCommentarii){ commentaria_doctrinae,   ARRAY_LONGITUDO(commentaria_doctrinae)   };
        case ARG_RECTOR:
            return (GrexCommentarii){ commentaria_rectoris,    ARRAY_LONGITUDO(commentaria_rectoris)    };
        case ARG_DISCIPULUS:
            return (GrexCommentarii){ commentaria_discipuli,   ARRAY_LONGITUDO(commentaria_discipuli)   };
        case ARG_FAMILIA:
            return (GrexCommentarii){ commentaria_familiae,    ARRAY_LONGITUDO(commentaria_familiae)    };
        case ARG_MUSICA:
            return (GrexCommentarii){ commentaria_musicae,     ARRAY_LONGITUDO(commentaria_musicae)     };
        case ARG_SENES:
            return (GrexCommentarii){ commentaria_senum,       ARRAY_LONGITUDO(commentaria_senum)       };
        case ARG_IUVENES:
            return (GrexCommentarii){ commentaria_iuvenum,     ARRAY_LONGITUDO(commentaria_iuvenum)     };
        case ARG_MORS:
            return (GrexCommentarii){ commentaria_mortis,      ARRAY_LONGITUDO(commentaria_mortis)      };
        case ARG_AMICITIA:
            return (GrexCommentarii){ commentaria_amicitiae,   ARRAY_LONGITUDO(commentaria_amicitiae)   };
        case ARG_SILENTIUM:
            return (GrexCommentarii){ commentaria_silentii,    ARRAY_LONGITUDO(commentaria_silentii)    };
        default:
            return (GrexCommentarii){ commentaria_defalta,     ARRAY_LONGITUDO(commentaria_defalta)     };
    }
}

/* =========================================================== */
/* Parallelismus: pares sicut ... ita ...                        */
/* =========================================================== */

typedef struct {
    const char *prior;
    const char *posterior;
} ParParallelismi;

static const ParParallelismi parallelismi[] = {
    { "aqua defluit in mare",          "virtus concurrit in sapientem"       },
    { "radix profunda tenet arborem",  "pietas profunda tenet domum"         },
    { "nubes celat solem",             "cupiditas celat virtutem"            },
    { "sagitta sine arcu non fertur",  "discipulus sine magistro non proficit" },
    { "ignis sine ligno extinguitur",  "humanitas sine consuetudine languet" },
    { "ver flores dat",                "doctrina mentes dat"                 },
    { "stella septentrionalis stat",   "rector iustus manet"                 },
    { "lyra sine fide tacet",          "civitas sine ritu trepidat"          },
    { "fluvius non redit ad fontem",   "tempus amissum non redit"            },
    { "argentum probat ignis",         "virum probat difficultas"            },
    { "sol omnia illuminat",           "virtus omnia inspicit"                },
    { "aer vitam dat animalibus",      "ritus vitam dat civitati"            },
    { "semen parvum in arborem crescit", "dictum unum in vitam integram vertitur" },
    { "piscis in aqua sine labore",    "vir nobilis in virtute sine coactu"  }
};

#define NUMERUS_PARALLELISMI (ARRAY_LONGITUDO(parallelismi))

/* =========================================================== */
/* Status colloquii: cultivatio et barbaries                     */
/* =========================================================== */

typedef struct StatusColloquii {
    /* signatura per bitfield ostendit flexibilitatem */
    unsigned cultivatio   : 8;
    unsigned barbaries    : 8;
    unsigned numerus_turn : 16;
    unsigned usurpatum    : 32;  /* dicta iam usurpata (per indicem %32) */
} StatusColloquii;

/* Nodus historiae: ostendit FAM (flexibile array membrum) */
typedef struct NodusHistoriae {
    uint32_t indexDicti;
    uint32_t longitudoTexti;
    char     textus[];
} NodusHistoriae;

/* =========================================================== */
/* Utilitates textuales                                         */
/* =========================================================== */

/* Normalizza verbum ad minusculas ASCII */
static void
minusculifica(char *restrict verbum)
{
    for (size_t i = 0; verbum[i]; i++)
        verbum[i] = (char)tolower((unsigned char)verbum[i]);
}

/* Computa numerum bitorum unitorum (Hamming weight) */
static inline int
pondus_hammingium(Tessera t)
{
    int n = 0;
    while (t) {
        n += (int)(t & 1u);
        t >>= 1;
    }
    return n;
}

/* =========================================================== */
/* Extractor argumentorum ex textu discipuli                     */
/* =========================================================== */

static Tessera
extrahe_argumenta(const char *restrict textus)
{
    Tessera acc = 0u;
    size_t n = strlen(textus);
    char verbum[MAX_VERBI];
    size_t i = 0;
    while (i < n) {
        while (i < n && !isalpha((unsigned char)textus[i])) i++;
        size_t j = 0;
        while (i < n && isalpha((unsigned char)textus[i]) && j < MAX_VERBI - 1) {
            verbum[j++] = (char)tolower((unsigned char)textus[i++]);
        }
        verbum[j] = '\0';
        if (j == 0) continue;
        for (int k = 0; k < ARRAY_LONGITUDO(claves); k++) {
            if (strcmp(verbum, claves[k].verbum) == 0) {
                acc |= claves[k].tessera;
                break;
            }
        }
    }
    return acc;
}

/* =========================================================== */
/* Selectio dicti per congruentiam argumenti                     */
/* =========================================================== */

typedef struct {
    int      index;
    int      pondus;
} Candidatus;

static int
collige_candidatos(Tessera argumenta, Candidatus *restrict cand)
{
    int n = 0;
    for (int i = 0; i < NUMERUS_DICTORUM; i++) {
        Tessera communia = corpus_dictorum[i].tesserae & argumenta;
        int pond = pondus_hammingium(communia);
        if (pond > 0) {
            cand[n].index = i;
            cand[n].pondus = pond;
            n++;
        }
    }
    return n;
}

static int
elige_dictum(Fors *restrict f, Tessera argumenta, const StatusColloquii *sc)
{
    Candidatus cand[MAX_CANDIDATORUM];
    int n = 0;
    if (argumenta != 0u) {
        n = collige_candidatos(argumenta, cand);
    }
    if (n == 0) {
        /* nullum congruentem: eligamus ex toto corpore secundum pondus 1 */
        for (int i = 0; i < NUMERUS_DICTORUM && i < MAX_CANDIDATORUM; i++) {
            cand[i].index = i;
            cand[i].pondus = 1;
        }
        n = NUMERUS_DICTORUM < MAX_CANDIDATORUM ? NUMERUS_DICTORUM : MAX_CANDIDATORUM;
    }

    /* punire iam usurpata */
    int summa = 0;
    for (int i = 0; i < n; i++) {
        int idx = cand[i].index;
        if (idx < 32 && (sc->usurpatum & (1u << idx))) {
            cand[i].pondus = (cand[i].pondus > 1) ? cand[i].pondus - 1 : 1;
        }
        summa += cand[i].pondus;
    }
    if (summa <= 0) summa = 1;

    int r = (int)fortuita_modo(f, (uint32_t)summa);
    int accum = 0;
    for (int i = 0; i < n; i++) {
        accum += cand[i].pondus;
        if (r < accum) return cand[i].index;
    }
    return cand[n - 1].index;
}

/* =========================================================== */
/* Selectio invocationis per statum                              */
/* =========================================================== */

static unsigned
elige_indicem_invocationis(Fors *restrict f, const StatusColloquii *sc, Tessera arg)
{
    if (sc->barbaries > sc->cultivatio + 1u) return INVOCATIO_BARBARA;
    if (sc->numerus_turn >= 8u) {
        if (fortuita_signum(f)) return INVOCATIO_SENEX;
    }
    if (arg & (ARG_DOCTRINA | ARG_DISCIPULUS)) return INVOCATIO_DOCTA;
    if (arg & ARG_VIRTUS) return INVOCATIO_VIRTUS;
    /* defalta: alterna inter virtus et docta */
    return fortuita_signum(f) ? INVOCATIO_VIRTUS : INVOCATIO_DOCTA;
}

static const char *
elige_invocationem(Fors *restrict f, const StatusColloquii *sc, Tessera arg)
{
    unsigned idx = elige_indicem_invocationis(f, sc, arg);
    GrexInvocationum grex = tabula_invocationum[idx];
    int len = longitudines_invocationum[idx];
    return grex[fortuita_modo(f, (uint32_t)len)];
}

/* =========================================================== */
/* Commentarius: ex grammatica brevi secundum argumentum         */
/* =========================================================== */

static const char *
elige_commentarium(Fors *restrict f, Tessera primum)
{
    GrexCommentarii g = selige_gregem_commentarii(primum);
    return g.versus[fortuita_modo(f, (uint32_t)g.numerus)];
}

/* =========================================================== */
/* Compositor parallelismorum                                   */
/* =========================================================== */

static void
compone_parallelismum(Fors *restrict f, char *restrict dest, size_t destSize)
{
    const ParParallelismi *p = &parallelismi[fortuita_modo(f, (uint32_t)NUMERUS_PARALLELISMI)];
    snprintf(dest, destSize, " Sicut %s, ita %s.", p->prior, p->posterior);
}

/* =========================================================== */
/* Mutator status colloquii                                     */
/* =========================================================== */

static void
actualiza_statum(StatusColloquii *sc, Tessera arg, int indexDicti)
{
    if (arg & (ARG_VIRTUS | ARG_HUMANITAS | ARG_PIETAS | ARG_DOCTRINA)) {
        if (sc->cultivatio < 255u) sc->cultivatio++;
    }
    if ((arg & ARG_RITUS) == 0u) {
        /* si ritus neglegitur, barbaries crescit — sed lente */
        if ((sc->numerus_turn & 1u) == 0u && sc->barbaries < 255u)
            sc->barbaries++;
    } else {
        if (sc->barbaries > 0u) sc->barbaries--;
    }
    if (indexDicti >= 0 && indexDicti < 32) {
        sc->usurpatum |= (1u << indexDicti);
    }
    if (sc->numerus_turn < 65535u) sc->numerus_turn++;
}

/* =========================================================== */
/* Compositor responsi                                          */
/* =========================================================== */

static void
compone_responsum(Fors *restrict f,
                  StatusColloquii *restrict sc,
                  const char *restrict input,
                  char *restrict responsum,
                  size_t dim)
{
    Tessera arg = extrahe_argumenta(input);
    int idx = elige_dictum(f, arg, sc);
    const Dictum *d = &corpus_dictorum[idx];
    const char *invoc = elige_invocationem(f, sc, arg);
    const char *comm  = elige_commentarium(f, d->primum);

    size_t scriptum = 0;
    int n = snprintf(responsum + scriptum, dim - scriptum,
                     "%s \"%s\" %s", invoc, d->textus, comm);
    if (n < 0) n = 0;
    scriptum += (size_t)n;
    if (scriptum >= dim) scriptum = dim - 1;

    /* parallelismus: circa tertia pars responsorum */
    if (fortuita_modo(f, 3) == 0 && scriptum + 128 < dim) {
        char buffer[160];
        compone_parallelismum(f, buffer, sizeof buffer);
        size_t blen = strlen(buffer);
        if (scriptum + blen < dim) {
            memcpy(responsum + scriptum, buffer, blen);
            scriptum += blen;
            responsum[scriptum] = '\0';
        }
    }

    actualiza_statum(sc, arg, idx);
}

/* =========================================================== */
/* Liber dialogi scripti: quindecim lineae discipuli             */
/* =========================================================== */

typedef struct {
    const char *nomenDiscipuli;
    const char *verba;
} LineaDialogi;

static const LineaDialogi dialogus_scriptus[] = {
    { "Yu",   "Magister, quid est pietas vera in filium?" },
    { "Tsze", "Quomodo, Magister, princeps populum regat?" },
    { "Hui",  "Quid sit humanitas quaeso doceas mihi." },
    { "Lu",   "Magister, discere difficile est; quae via?" },
    { "Kung", "De ritibus antiquorum quid tenendum est?" },
    { "Min",  "Musica, Magister, cur tantum valet?" },
    { "Tsze", "Quomodo inter vir nobilem et hominem parvum distinguam?" },
    { "Yu",   "De morte, Magister, dic aliquid." },
    { "Hui",  "Amicus verus qualis invenitur?" },
    { "Lu",   "Magister, quid sit iustitia in rebus publicis?" },
    { "Kung", "Senes in familia quomodo honorandi sunt?" },
    { "Min",  "Iuvenes, Magister, quid discant primo?" },
    { "Tsze", "Silentium et verba quomodo temperabo?" },
    { "Yu",   "Magister, quid sit via perfecta?" },
    { "Hui",  "Gratias ago, Magister, pro tanta doctrina." }
};

#define NUMERUS_LINEARUM_SCRIPTI (ARRAY_LONGITUDO(dialogus_scriptus))

/* =========================================================== */
/* Usus et errores                                               */
/* =========================================================== */

static void
scribe_usum(FILE *fluxus, const char *programma)
{
    fprintf(fluxus, "Usus: %s [-s SEMEN] [-i]\n", programma);
    fprintf(fluxus, "  -s N    pone semen generatoris (numerus integer)\n");
    fprintf(fluxus, "  -i      aperi colloquium interactivum\n");
    fprintf(fluxus, "  sine argumentis: dialogus scriptus cum discipulis\n");
}

/* Variadicum macrum demonstratum per internum helper:             */
static void
susurra_linea(const char *praefixum, const char *textus)
{
    SUSURRA("%s %s\n", praefixum, textus);
}

/* =========================================================== */
/* Modus scripti: edit dialogum praeparatum                      */
/* =========================================================== */

static void
execute_dialogum_scriptum(Fors *restrict f, StatusColloquii *restrict sc)
{
    SUSURRA("=== Analecta Confucii ===\n");
    SUSURRA("(dialogus scriptus inter Magistrum et discipulos)\n\n");

    char responsum[MAX_RESPONSI];
    for (int i = 0; i < NUMERUS_LINEARUM_SCRIPTI; i++) {
        const LineaDialogi *ld = &dialogus_scriptus[i];
        SUSURRA("DISCIPULUS %s: %s\n", ld->nomenDiscipuli, ld->verba);
        compone_responsum(f, sc, ld->verba, responsum, sizeof responsum);
        susurra_linea("CONFUCIUS:", responsum);
        SUSURRA("\n");
    }

    SUSURRA("--- Finis dialogi ---\n");
    SUSURRA("(cultivatio=%u, barbaries=%u, turnationes=%u)\n",
            (unsigned)sc->cultivatio,
            (unsigned)sc->barbaries,
            (unsigned)sc->numerus_turn);
}

/* =========================================================== */
/* Modus interactivus: linea cum linea                           */
/* =========================================================== */

static void
execute_modum_interactivum(Fors *restrict f, StatusColloquii *restrict sc)
{
    SUSURRA("=== Confucius interactivus ===\n");
    SUSURRA("(scribe interrogationem; 'vale' terminat colloquium)\n\n");

    char linea[MAX_LINEAE];
    char responsum[MAX_RESPONSI];
    while (1) {
        SUSURRA("DISCIPULUS> ");
        fflush(stdout);
        if (!fgets(linea, sizeof linea, stdin)) break;
        /* eripe terminatorem lineae */
        size_t ll = strlen(linea);
        while (ll > 0 && (linea[ll - 1] == '\n' || linea[ll - 1] == '\r')) {
            linea[--ll] = '\0';
        }
        if (ll == 0) continue;
        /* valedictio */
        char copia[MAX_LINEAE];
        strncpy(copia, linea, sizeof copia - 1);
        copia[sizeof copia - 1] = '\0';
        minusculifica(copia);
        if (strcmp(copia, "vale") == 0 || strcmp(copia, "valete") == 0) {
            SUSURRA("CONFUCIUS: Vale, discipule. Ambula in via virtutis.\n");
            break;
        }
        compone_responsum(f, sc, linea, responsum, sizeof responsum);
        susurra_linea("CONFUCIUS:", responsum);
    }
}

/* =========================================================== */
/* Parsa argumenta                                              */
/* =========================================================== */

typedef struct {
    uint64_t semen;
    int      interactivum;
} Configuratio;

static int
parsa_argumenta(int argc, char *argv[], Configuratio *conf)
{
    conf->semen = SEMEN_DEFALTUM;
    conf->interactivum = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
            if (i + 1 >= argc) {
                ERRORA("Error: '-s' postulat argumentum.\n");
                return -1;
            }
            char *finis = NULL;
            unsigned long long v = strtoull(argv[++i], &finis, 10);
            if (finis == argv[i] || *finis != '\0') {
                ERRORA("Error: semen non est numerus integer: '%s'\n", argv[i]);
                return -1;
            }
            conf->semen = (uint64_t)v;
        } else if (strcmp(argv[i], "-i") == 0) {
            conf->interactivum = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--auxilium") == 0) {
            scribe_usum(stdout, argv[0]);
            return 1;  /* signal: exit 0 */
        } else {
            ERRORA("Error: vexillum ignotum: '%s'\n", argv[i]);
            scribe_usum(stderr, argv[0]);
            return -1;
        }
    }
    return 0;
}

/* =========================================================== */
/* Principium programmi                                         */
/* =========================================================== */

int
main(int argc, char *argv[])
{
    silentium_warningum();
    Configuratio conf;
    int r = parsa_argumenta(argc, argv, &conf);
    if (r == 1) return 0;
    if (r < 0) return 2;

    /* pura cautio ut semen non sit zero (xorshift vetitum) */
    Fors fors = { .status = conf.semen ? conf.semen : SEMEN_DEFALTUM };

    StatusColloquii sc = {
        .cultivatio   = 0u,
        .barbaries    = 0u,
        .numerus_turn = 0u,
        .usurpatum    = 0u
    };

    if (conf.interactivum) {
        execute_modum_interactivum(&fors, &sc);
    } else {
        execute_dialogum_scriptum(&fors, &sc);
    }

    return 0;
}

/* =========================================================== */
/* Appendix: dicta antiqua tamquam commentarium inline           */
/*                                                              */
/* In Analectis Magister docuit quaedam difficilia memoriae:     */
/*   "Nondum vitam novi — quomodo mortem?"                       */
/*   "Tres eunt — magister meus inter eos est."                  */
/*   "Discere et saepe exercere — nonne iucundum?"               */
/*                                                              */
/* Haec verba in corpore superius manent. Sub hoc commentario    */
/* ponimus functiones accessorias, ne corpus programmi turbetur. */
/* =========================================================== */

/* inline helper ad probandum congruentiam unam ex testa */
static inline int
congruens_testae(Tessera t, Tessera testa)
{
    return (t & testa) == testa;
}

/* Mensor diversitatis argumentorum: pro futura extensione */
static int
mensura_diversitatis(const Tessera *restrict tt, int n)
{
    Tessera unio = 0u;
    for (int i = 0; i < n; i++) unio |= tt[i];
    return pondus_hammingium(unio);
}

/* Computator congruentiae inter duo dicta: utile pro analysi     */
static int
similitudo_dictorum(int a, int b)
{
    if (a < 0 || b < 0 || a >= NUMERUS_DICTORUM || b >= NUMERUS_DICTORUM) return 0;
    Tessera com = corpus_dictorum[a].tesserae & corpus_dictorum[b].tesserae;
    return pondus_hammingium(com);
}

/* Enumerator dictorum per argumentum: pro futura utilitate       */
static int
enumera_per_argumentum(Tessera arg, int *restrict outIdx, int cap)
{
    int n = 0;
    for (int i = 0; i < NUMERUS_DICTORUM && n < cap; i++) {
        if (corpus_dictorum[i].tesserae & arg) {
            outIdx[n++] = i;
        }
    }
    return n;
}

/* Probator status: indicat si colloquium in statu virtutis est   */
static int
status_virtuosus(const StatusColloquii *sc)
{
    return sc->cultivatio > sc->barbaries;
}

/* Computator pro sequenti invocatione, alternativa              */
static unsigned
alternativa_invocatio(const StatusColloquii *sc)
{
    if (status_virtuosus(sc)) return INVOCATIO_VIRTUS;
    return INVOCATIO_BARBARA;
}

/* Utilitates supra non sunt in itinere principali usurpatae;      */
/* manent ut monstrent varias C99 technicas et extensibilitatem.   */

/* Callum pro silentio warningum de functionibus non-usurpatis.    */
static void
silentium_warningum(void)
{
    (void)congruens_testae;
    (void)mensura_diversitatis;
    (void)similitudo_dictorum;
    (void)enumera_per_argumentum;
    (void)alternativa_invocatio;
    (void)status_virtuosus;
}


/* =========================================================== */
/* Codicillus finalis                                           */
/* =========================================================== */
