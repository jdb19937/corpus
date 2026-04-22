/*
 * socrates.c — Automaton Dialecticus Socraticus
 *
 * Automaton qui imitatur morem Socratis Atheniensis in elencho
 * dialectico.  Interrogat, refutat, ducit ad aporiam, tum per
 * maieuticam parit cognitionem occultam in animo interlocutoris.
 *
 * Machina status (STATUS MACHINA):
 *   IRONIA      — primum fingit se ignorare, laudat interlocutorem
 *   ELENCHUS    — excutit sententias, quaerit definitiones
 *   APORIA      — monstrat contradictiones; interlocutor haesitat
 *   MAIEUTICA   — obstetricat veritatem ex animo ipsius discipuli
 *   CONCLUSIO   — claudit sermonem cum modesta recognitione
 *
 * Transitus inter status pendet ex numero conversionum (turnorum)
 * et ex fortitudine assertionum (pondus claim) quas interlocutor
 * profert.  Cum satis refutationes factae sunt, transitur ad
 * aporiam; deinde per maieuticam ad conclusionem.
 *
 * Thesaurus locorum (TOPICA): iustitia, virtus, pietas, fortitudo,
 * amicitia, sapientia, voluptas, mors, anima, pulchritudo, veritas,
 * scientia.  Pro quoque loco habetur catena propositionum paratarum
 * (P et ~P) quae alimentum praebent machinae contradictionis.
 *
 * Generator PRNG: xorshift64 cum semine constanti defaltico; potest
 * tamen per argumentum -s numerus mutari pro experimentis reiteratis.
 *
 * Argumenta lineae imperii:
 *   (sine argumentis)  — colloquium scriptum cum discipulo efferet
 *   -i                 — accipit lineas ex stdin, respondet singulis
 *   -s NUMERUS         — ponit semen generatoris pseudo-fortuiti
 *
 * Omnia verba, omnes commentarii, omnia nomina linguae latinae sunt.
 * Solum nomina bibliothecarum standardium anglice scripta manent,
 * quia sic lingua C postulat.
 *
 * Auctore automato, sub tutela Minervae et Mercurii.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <limits.h>

/* ===================================================================
 * SECTIO PRIMA — Constantes et Macra Universalia
 * =================================================================*/

#define SEMEN_DEFALTUM            0xBADC0FFEE0DDF00DULL
#define LONGITUDO_LINEAE_MAX      1024
#define VOCABULUM_MAX             64
#define VOCABULA_PER_LINEAM_MAX   96
#define RESPONSIO_MAX             2048
#define REFUTATIONES_LIMES        3
#define TURNI_AD_APORIAM          6
#define TURNI_AD_MAIEUTICAM       10
#define TURNI_AD_CONCLUSIONEM     13

/* Macron variadicum pro diagnostica (ad stderr) */
#define DIAGNOSIS(fmt, ...)       fprintf(stderr, "[diagnosis] " fmt "\n", ##__VA_ARGS__)

/* Macron pro numero elementorum in tabula */
#define NUMERUS_ELEMENTORUM(a)    ((int)(sizeof(a) / sizeof((a)[0])))

/* Macron pro minimo et maximo */
#define MINIMUM(a, b)             ((a) < (b) ? (a) : (b))
#define MAXIMUM(a, b)             ((a) > (b) ? (a) : (b))

/* ===================================================================
 * SECTIO SECUNDA — Typi Fundamentales et Enumerationes
 * =================================================================*/

/* Status machinae dialogi: quinque gradus cursus socratici */
typedef enum {
    STATUS_IRONIA    = 0,
    STATUS_ELENCHUS  = 1,
    STATUS_APORIA    = 2,
    STATUS_MAIEUTICA = 3,
    STATUS_CONCLUSIO = 4,
    STATUS_NUMERUS   = 5
} StatusDialogi;

/* Loci (topica) conversationis.  Ordo necessario servandus est
 * quia tabulae quaestionum per hunc indicem accedunt. */
typedef enum {
    TOPICUM_IUSTITIA    =  0,
    TOPICUM_VIRTUS      =  1,
    TOPICUM_PIETAS      =  2,
    TOPICUM_FORTITUDO   =  3,
    TOPICUM_AMICITIA    =  4,
    TOPICUM_SAPIENTIA   =  5,
    TOPICUM_VOLUPTAS    =  6,
    TOPICUM_MORS        =  7,
    TOPICUM_ANIMA       =  8,
    TOPICUM_PULCHRITUDO =  9,
    TOPICUM_VERITAS     = 10,
    TOPICUM_SCIENTIA    = 11,
    TOPICUM_IGNOTUM     = 12,
    TOPICUM_NUMERUS     = 13
} TopicumDialogi;

/* Indicia assertionis: quae verba probant interlocutorem assertionem
 * fortem proferre.  Pondus per bitfield digestum. */
typedef struct {
    unsigned est_praesens        : 1;
    unsigned sunt_praesens       : 1;
    unsigned omnis_praesens      : 1;
    unsigned nemo_praesens       : 1;
    unsigned semper_praesens     : 1;
    unsigned numquam_praesens    : 1;
    unsigned bonum_praesens      : 1;
    unsigned malum_praesens      : 1;
    unsigned iustum_praesens     : 1;
    unsigned iniustum_praesens   : 1;
    unsigned interrogatio        : 1;
    unsigned negatio             : 1;
    unsigned reservatum          : 4;
    uint16_t pondus;   /* summa indiciorum, ponderata */
} AnalysisLineae;

/* Unio pro conversione numerorum (exemplum typi C99 avanti) */
typedef union {
    uint64_t integrum;
    double   fractum;
    uint8_t  octeti[8];
} BinariumConversor;

/* Structura vocabuli extracti ex linea */
typedef struct {
    char     verbum[VOCABULUM_MAX];
    uint8_t  longitudo;
    uint8_t  est_nomen;      /* conjectura: incipit littera capitali? */
    uint8_t  est_claim;      /* est in indice assertionum fortium? */
    uint8_t  reservatum;
} Vocabulum;

/* Structura lineae analysatae */
typedef struct {
    Vocabulum      vocabula[VOCABULA_PER_LINEAM_MAX];
    int            numerus_vocabulorum;
    AnalysisLineae analysis;
    TopicumDialogi topicum;
    char           nomen_extractum[VOCABULUM_MAX];
    char           adiectivum_extractum[VOCABULUM_MAX];
} LineaAnalysata;

/* Status totius dialogi */
typedef struct Dialogus Dialogus;
struct Dialogus {
    StatusDialogi   status;
    int             turnus;
    int             refutationes;
    int             numerus_claim_fortium;
    TopicumDialogi  topicum_currens;
    TopicumDialogi  topicum_antecedens;
    char            ultimum_nomen[VOCABULUM_MAX];
    char            ultimum_adiectivum[VOCABULUM_MAX];
    uint64_t        semen_prng;
    /* Membrum flexibile — catena historiae turnorum */
    int             capacitas_historiae;
    int             longitudo_historiae;
    char          **historia;
};

/* Propositio paria pro machina contradictionis */
typedef struct {
    const char *assertio;        /* quod interlocutor dicit (P) */
    const char *contrarium;      /* quod Socrates insinuare vult (~P) */
    const char *quaestio_ducens; /* quaestio quae ducit ad ~P */
} PropositioParia;

/* Eventus claim: quid interlocutor asseruit? */
typedef struct {
    TopicumDialogi topicum;
    uint8_t        fortitudo;   /* 0..10 */
    const char    *lemma;
} EventusClaim;

/* ===================================================================
 * SECTIO TERTIA — PRNG Xorshift64
 * =================================================================*/

/* Generator pseudo-fortuitus.  Deterministicus, velox, sufficit. */
static inline uint64_t
xorshift64_proxima(uint64_t *restrict status)
{
    uint64_t x = *status;
    if (x == 0ULL) x = 0x9E3779B97F4A7C15ULL;  /* ne stagnet in nihilo */
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *status = x;
    return x;
}

/* Numerus fortuitus in intervallo [0, limes) */
static inline uint64_t
fortuitus_sub(uint64_t *restrict status, uint64_t limes)
{
    if (limes == 0ULL) return 0ULL;
    return xorshift64_proxima(status) % limes;
}

/* Fortuitus inter a et b inclusive */
static inline int
fortuitus_inter(uint64_t *restrict status, int a, int b)
{
    if (b < a) { int t = a; a = b; b = t; }
    return a + (int)fortuitus_sub(status, (uint64_t)(b - a + 1));
}

/* ===================================================================
 * SECTIO QUARTA — Lexicon Claim Markers et Topicorum
 * =================================================================*/

/* Index verborum quae signant assertionem fortem */
static const char *const verba_claim[] = {
    "est", "sunt", "omnis", "omnes", "nemo", "nullus", "semper",
    "numquam", "bonum", "malum", "iustum", "iniustum", "verum",
    "falsum", "pulchrum", "turpe", "sapiens", "stultus", "pium",
    "impium", "fortis", "timidus", "necesse", "oportet", NULL
};

/* X-MACRON: index verborum topicorum.  Expanditur multipliciter
 * in diversis contextibus (generatio enumerationis non, iam habemus;
 * sed pro tabula concordantiae vocabulorum ad topica). */
#define TABULA_TOPICORUM                                                    \
    X(TOPICUM_IUSTITIA,    "iustitia",    "iustus",    "iuste",    "ius")   \
    X(TOPICUM_IUSTITIA,    "iustitiam",   "iniustus",  "iniuste",  "iuris") \
    X(TOPICUM_VIRTUS,      "virtus",      "virtutem",  "virtutis", "arete") \
    X(TOPICUM_VIRTUS,      "virtute",     "virtuti",   "virtutes", "virtutum") \
    X(TOPICUM_PIETAS,      "pietas",      "pietatem",  "pius",     "impius") \
    X(TOPICUM_PIETAS,      "pietate",     "pietatis",  "deos",     "deorum") \
    X(TOPICUM_FORTITUDO,   "fortitudo",   "fortis",    "fortes",   "audax") \
    X(TOPICUM_FORTITUDO,   "fortitudinem","fortitudine","timor",   "metus") \
    X(TOPICUM_AMICITIA,    "amicitia",    "amicus",    "amici",    "amicos") \
    X(TOPICUM_AMICITIA,    "amicorum",    "amicitiam", "sodalis",  "sodales") \
    X(TOPICUM_SAPIENTIA,   "sapientia",   "sapiens",   "sapientes","sophia") \
    X(TOPICUM_SAPIENTIA,   "sapientiam",  "sapientis", "prudens",  "prudentia") \
    X(TOPICUM_VOLUPTAS,    "voluptas",    "voluptatem","voluptati","hedone") \
    X(TOPICUM_VOLUPTAS,    "delectatio",  "dulce",     "suave",    "iucundum") \
    X(TOPICUM_MORS,        "mors",        "mortem",    "mortis",   "thanatos") \
    X(TOPICUM_MORS,        "morte",       "mori",      "mortuus",  "mortui") \
    X(TOPICUM_ANIMA,       "anima",       "animam",    "animae",   "psyche") \
    X(TOPICUM_ANIMA,       "animus",      "animi",     "spiritus", "mens")  \
    X(TOPICUM_PULCHRITUDO, "pulchritudo", "pulchrum",  "pulcher",  "kalon") \
    X(TOPICUM_PULCHRITUDO, "formosus",    "decorus",   "venustas", "venustatem") \
    X(TOPICUM_VERITAS,     "veritas",     "veritatem", "verum",    "aletheia") \
    X(TOPICUM_VERITAS,     "veri",        "mendacium", "falsum",   "fallacia") \
    X(TOPICUM_SCIENTIA,    "scientia",    "scientiam", "scio",     "episteme") \
    X(TOPICUM_SCIENTIA,    "cognitio",    "cognoscere","intellego","nosco")

typedef struct {
    TopicumDialogi topicum;
    const char    *v1, *v2, *v3, *v4;
} EntriumTopici;

static const EntriumTopici tabula_topicorum[] = {
#define X(t, a, b, c, d) { .topicum = (t), .v1 = (a), .v2 = (b), .v3 = (c), .v4 = (d) },
    TABULA_TOPICORUM
#undef X
};

/* Nomina topicorum (pro expressione humana) */
static const char *const nomina_topicorum[TOPICUM_NUMERUS] = {
    [TOPICUM_IUSTITIA]    = "iustitia",
    [TOPICUM_VIRTUS]      = "virtus",
    [TOPICUM_PIETAS]      = "pietas",
    [TOPICUM_FORTITUDO]   = "fortitudo",
    [TOPICUM_AMICITIA]    = "amicitia",
    [TOPICUM_SAPIENTIA]   = "sapientia",
    [TOPICUM_VOLUPTAS]    = "voluptas",
    [TOPICUM_MORS]        = "mors",
    [TOPICUM_ANIMA]       = "anima",
    [TOPICUM_PULCHRITUDO] = "pulchritudo",
    [TOPICUM_VERITAS]     = "veritas",
    [TOPICUM_SCIENTIA]    = "scientia",
    [TOPICUM_IGNOTUM]     = "res incerta"
};

/* Nomina statuum pro diagnosi */
static const char *const nomina_statuum[STATUS_NUMERUS] = {
    [STATUS_IRONIA]    = "IRONIA",
    [STATUS_ELENCHUS]  = "ELENCHUS",
    [STATUS_APORIA]    = "APORIA",
    [STATUS_MAIEUTICA] = "MAIEUTICA",
    [STATUS_CONCLUSIO] = "CONCLUSIO"
};

/* ===================================================================
 * SECTIO QUINTA — Tabulae Templatorum Quaestionum (X-MACRON)
 * =================================================================*/

/*
 * Pro quoque status+topicum, habemus plures templas quaestionum.
 * Signum %N in templa substituitur nomine extracto; signum %A
 * substituitur adiectivo extracto; signum %T nomen topici.
 */

#define TEMPLATA_IRONIA                                                     \
    T(IRONIA, IUSTITIA,    "Dic mihi, amice: quid vero %T esse censes?")  \
    T(IRONIA, IUSTITIA,    "Ego quidem ignoro quid sit %T; tu autem scire videris. Doce me.") \
    T(IRONIA, VIRTUS,      "Felicem te qui de %T loqui audes! Quid ergo est?") \
    T(IRONIA, VIRTUS,      "Utinam tam clare intellegerem %T quam tu, O praeclare!") \
    T(IRONIA, PIETAS,      "Si tu de %T docere potes, venio ad te ut discipulus.") \
    T(IRONIA, FORTITUDO,   "Putasne %T facile definiri posse?")           \
    T(IRONIA, AMICITIA,    "De %T loquimur saepe, at raro scimus quid dicamus.") \
    T(IRONIA, SAPIENTIA,   "Tu sapiens videris; ego vero nihil scio de %T.") \
    T(IRONIA, VOLUPTAS,    "Estne %T bonum summum, ut volunt multi?")     \
    T(IRONIA, MORS,        "Timesne %T, aut cognovisti eam satis?")        \
    T(IRONIA, ANIMA,       "Si %T immortalis est, cur sic de ea dubitamus?") \
    T(IRONIA, PULCHRITUDO, "Quid agit %T in rebus mortalibus?")            \
    T(IRONIA, VERITAS,     "Cognoscisne %T, an tantum opinionem eius?")    \
    T(IRONIA, SCIENTIA,    "Sine %T quid sumus nisi umbrae?")              \
    T(IRONIA, IGNOTUM,     "De qua re agis, amice? Nondum intellego.")

#define TEMPLATA_ELENCHUS                                                   \
    T(ELENCHUS, IUSTITIA,    "Dicis %T esse %A. At nonne etiam in bello %T exigitur? Ibi quoque %A manet?") \
    T(ELENCHUS, IUSTITIA,    "Si %N iustus est, quia reddit debita, quid si debitum est gladius furioso?") \
    T(ELENCHUS, IUSTITIA,    "Nonne interdum iuste mentimur ut servemus innocentem? Ergo quid est %T?") \
    T(ELENCHUS, VIRTUS,      "Dicis virtutem esse %A. Sed eadem virtus in puero et in sene? In viro et femina?") \
    T(ELENCHUS, VIRTUS,      "Si %T doceri potest, cur filii sapientium non semper sapientes fiunt?") \
    T(ELENCHUS, PIETAS,      "Dicis pium esse quod dis placet. At inter deos ipsos discordia; quid tum placet?") \
    T(ELENCHUS, PIETAS,      "Estne pium quia dis placet, an dis placet quia pium?") \
    T(ELENCHUS, FORTITUDO,   "Fortis est qui non timet? At etiam stultus non timet quia nescit pericula.") \
    T(ELENCHUS, FORTITUDO,   "Sic %T est %A? Quid ergo de milite qui fugit prudenter?") \
    T(ELENCHUS, AMICITIA,    "Amicus est similis amico, dicis. Quid ergo de bono et malo? Possuntne esse amici?") \
    T(ELENCHUS, AMICITIA,    "Si amamus propter utilitatem, estne vera %T, an mercatura quaedam?") \
    T(ELENCHUS, SAPIENTIA,   "Si sapiens scit omnia, scit etiam se scire; at nemo omnia novit.") \
    T(ELENCHUS, SAPIENTIA,   "Dicis %N sapientem esse quia multa dicit. Nonne etiam rhetor multa dicit?") \
    T(ELENCHUS, VOLUPTAS,    "Si %T est bonum, estne omnis voluptas bona? Etiam voluptas tyranni?") \
    T(ELENCHUS, VOLUPTAS,    "Quid si %A voluptas perdat animam? Manetne bonum?") \
    T(ELENCHUS, MORS,        "Dicis %T esse malum. At si malum est, cur sapientes eam non fugerunt turpiter?") \
    T(ELENCHUS, MORS,        "Si nihil post %T, quid mali in nihilo?")     \
    T(ELENCHUS, ANIMA,       "Si anima corpori iungitur, cur interdum corpori adversatur?") \
    T(ELENCHUS, ANIMA,       "Dicis animam %A. Potestne res %A mutari, ut mutatur animus noster?") \
    T(ELENCHUS, PULCHRITUDO, "%T in oculo spectantis est? Tum quid de pulchritudine geometrica, quam omnes videmus?") \
    T(ELENCHUS, PULCHRITUDO, "Si %N pulcher est, quia %A, est etiam equus pulcher ob %A?") \
    T(ELENCHUS, VERITAS,     "Dicis veritatem esse id quod omnes credunt. Quid si omnes errant?") \
    T(ELENCHUS, VERITAS,     "Quomodo distinguis %T a firma opinione?")   \
    T(ELENCHUS, SCIENTIA,    "Si %T sensibus innititur, quid de iis quae videre non possumus, velut iustitia?") \
    T(ELENCHUS, SCIENTIA,    "Scirene est meminisse, an discere ex nihilo?") \
    T(ELENCHUS, IGNOTUM,     "Explica clarius: quid %N significat in sermone tuo?")

#define TEMPLATA_APORIA                                                     \
    T(APORIA, IUSTITIA,    "Ergo videmur ignorare quid sit %T; dixeramus %A, at hoc refutatum est.") \
    T(APORIA, VIRTUS,      "Ita res se habet: nec doceri %T posse concedis, nec natura nasci.") \
    T(APORIA, PIETAS,      "Manemus in aporia: pietas neque placere deis definiri potest neque per se.") \
    T(APORIA, FORTITUDO,   "Quid ergo? Nec %T ignorantia est, nec cognitio sola.") \
    T(APORIA, AMICITIA,    "Videtur %T fugere definitionem nostram sicut piscis manus.") \
    T(APORIA, SAPIENTIA,   "Si %T omnium scientia est, nemo sapiens; si unius, quae?") \
    T(APORIA, VOLUPTAS,    "Neque summum bonum %T, neque malum simpliciter. Quid tandem?") \
    T(APORIA, MORS,        "Ne scimus quidem an %T bonum sit an malum; ignoramus.") \
    T(APORIA, ANIMA,       "%T nobis obscurior fit quo magis eam scrutamur.") \
    T(APORIA, PULCHRITUDO, "%T rerum elabitur: non in forma, non in usu, non in opinione.") \
    T(APORIA, VERITAS,     "%T nec consensu hominum, nec sensu, nec ratione sola tenetur.") \
    T(APORIA, SCIENTIA,    "Quid sit %T nescimus; et hoc ipsum, mi amice, scimus.") \
    T(APORIA, IGNOTUM,     "In tenebris versamur; hoc saltem manifestum est.")

#define TEMPLATA_MAIEUTICA                                                  \
    T(MAIEUTICA, IUSTITIA,    "Sed tu ipse, cum de %N cogitas, quid sentis in animo tuo?") \
    T(MAIEUTICA, VIRTUS,      "Respice in te: nonne iam habes partem veritatis de %T?") \
    T(MAIEUTICA, PIETAS,      "Meministi pueritiae tuae: cum precabaris, quid sperabas?") \
    T(MAIEUTICA, FORTITUDO,   "Cum timuisti aliquando, quid te retinuit? Nonne illud ipsum %T fuit?") \
    T(MAIEUTICA, AMICITIA,    "Habesne amicum verum? Cum eo es, quid in te oritur?") \
    T(MAIEUTICA, SAPIENTIA,   "Cum confiteris te nescire, nonne tunc maxime sapis?") \
    T(MAIEUTICA, VOLUPTAS,    "Sentisne voluptatem maiorem in rebus %A an in simplicibus?") \
    T(MAIEUTICA, MORS,        "Si hodie morereris, quid relinqueres quod valeret?") \
    T(MAIEUTICA, ANIMA,       "Cum somnias, quis sentit imagines? Corpus dormit, ergo quid?") \
    T(MAIEUTICA, PULCHRITUDO, "Quid pulcherrimum umquam vidisti? Cur illud pulchrum iudicasti?") \
    T(MAIEUTICA, VERITAS,     "Tune ipse distinguere potes verum a falso in tua vita cotidiana?") \
    T(MAIEUTICA, SCIENTIA,    "Cum aliquid discis, estne sensus tui cognoscendi, an recordandi?") \
    T(MAIEUTICA, IGNOTUM,     "Intra te ipsum respice: quae sententia de %N oritur sponte?")

#define TEMPLATA_CONCLUSIO                                                  \
    T(CONCLUSIO, IUSTITIA,    "Hoc ergo uno scimus de %T: eam quaerendam esse, etiam ignorantes.") \
    T(CONCLUSIO, VIRTUS,      "Virtutis semen in te iam erat; nos tantum parere adiuvimus.") \
    T(CONCLUSIO, PIETAS,      "Pium igitur est ipsum quaerere quid pium sit. Hoc satis in hodiernum.") \
    T(CONCLUSIO, FORTITUDO,   "Fortis est qui fateri audet ignorantiam suam. Sic tu fortis factus es.") \
    T(CONCLUSIO, AMICITIA,    "Amicitia enim nostra hodie aliquid peperit, etsi non completum.") \
    T(CONCLUSIO, SAPIENTIA,   "Hoc unum scio: me nihil scire. Tu iam socius huius sapientiae es.") \
    T(CONCLUSIO, VOLUPTAS,    "Relinquamus voluptates sensuum; maior est voluptas veritatem quaerere.") \
    T(CONCLUSIO, MORS,        "De morte iudicemus cum ad eam venerimus; nunc vivamus philosophantes.") \
    T(CONCLUSIO, ANIMA,       "Animam colamus, ut Apollinis oraculum iubet: 'nosce te ipsum'.") \
    T(CONCLUSIO, PULCHRITUDO, "Pulchritudo rerum nos ducat ad pulchritudinem animi.") \
    T(CONCLUSIO, VERITAS,     "Veritas quaerenda, etsi numquam tota tenenda. Vale, discipule.") \
    T(CONCLUSIO, SCIENTIA,    "Hoc colloquium ipsum scientia fuit; non minor quia sine conclusione firma.") \
    T(CONCLUSIO, IGNOTUM,     "Relinquamus rem in medio; alio die redibimus ad eam.")

/* Structura entrii templatae quaestionis */
typedef struct {
    StatusDialogi  status;
    TopicumDialogi topicum;
    const char    *templa;
} EntriumTemplae;

/* Aggregatio omnium templorum in unam tabulam */
static const EntriumTemplae tabula_templorum[] = {
#define T(s, t, p) { .status = STATUS_##s, .topicum = TOPICUM_##t, .templa = (p) },
    TEMPLATA_IRONIA
    TEMPLATA_ELENCHUS
    TEMPLATA_APORIA
    TEMPLATA_MAIEUTICA
    TEMPLATA_CONCLUSIO
#undef T
};

/* ===================================================================
 * SECTIO SEXTA — Base Scientiae Contradictionum (P et ~P)
 * =================================================================*/

static const PropositioParia contradictiones_iustitia[] = {
    { "iustum est reddere debita",
      "non semper reddere debita iustum est",
      "Si amico gladium commodasti, et ille furiosus factus est, reddesne gladium?" },
    { "iustum est benefacere amicis et nocere inimicis",
      "iusto non convenit ulli nocere",
      "Nonne qui noceat alium facit peiorem? Potestne id opus iustitiae esse?" },
    { "iustitia est utilitas fortioris",
      "iustitia non pendet ex potentia sed ex ratione",
      "Si fortior errat, manetne utilitas eius iustitia?" }
};

static const PropositioParia contradictiones_virtus[] = {
    { "virtus doceri potest",
      "virtus non doceri potest sicut ars",
      "Cur filii virorum magnorum non semper virtutem accipiunt?" },
    { "virtus est cognitio",
      "virtus non est mera cognitio; habitus est",
      "Nonne multi sciunt bonum et tamen malum agunt?" }
};

static const PropositioParia contradictiones_pietas[] = {
    { "pium est quod deis placet",
      "non omne quod deis placet per se pium est",
      "Si inter deos discordia, quid simul placet omnibus?" },
    { "pietas est scientia sacrificandi",
      "pietas non est solum ars ritualis",
      "Si animal pulchrum sacrificas, estne ipsum sacrificium pium, an dispositio animi?" }
};

static const PropositioParia contradictiones_fortitudo[] = {
    { "fortis est qui nihil timet",
      "non qui nihil timet fortis est, sed qui timens bene agit",
      "Estne leo fortis, an mere ferox? Quid distinguit fortitudinem a ferocia?" }
};

static const PropositioParia contradictiones_amicitia[] = {
    { "amicus est similis amico",
      "contraria saepe amantur; non sola similitudo",
      "Nonne medicus aegroto amicus est, quamquam dissimiles?" }
};

static const PropositioParia contradictiones_sapientia[] = {
    { "sapiens est qui multa novit",
      "sapiens est qui novit se nihil scire",
      "Qui omnia se scire putat, potestne adhuc quaerere?" }
};

static const PropositioParia contradictiones_voluptas[] = {
    { "voluptas est bonum summum",
      "voluptas non est bonum summum sed saepe impedimentum",
      "Si bibendo plus delectaris, sed pereas, estne illa voluptas bonum?" }
};

static const PropositioParia contradictiones_mors[] = {
    { "mors est malum terribile",
      "mors incognita est; stultum timere quod nescimus",
      "Si nescimus quid sit mors, cur eam pro malo ducimus?" }
};

static const PropositioParia contradictiones_anima[] = {
    { "anima cum corpore perit",
      "anima immortalis est",
      "Si somniamus corpore immoto, quid tum agit in somnio?" }
};

static const PropositioParia contradictiones_pulchritudo[] = {
    { "pulchritudo est in oculo spectantis",
      "est pulchritudo universalis, ut in proportionibus",
      "Cur omnes homines circulum bene formatum pulchrum iudicant?" }
};

static const PropositioParia contradictiones_veritas[] = {
    { "veritas est consensus multorum",
      "veritas non pendet ex numero credentium",
      "Si omnes Athenienses errant, fitne error veritas?" }
};

static const PropositioParia contradictiones_scientia[] = {
    { "scientia per sensus tantum venit",
      "scientia per rationem venit, sensibus adiuta",
      "Numerum trium potesne tangere, videre, audire? Unde igitur scis eum?" }
};

/* Tabula contradictionum per topicum, cum numero entorum */
typedef struct {
    const PropositioParia *paria;
    int                    numerus;
} CatalogusContradictionum;

static const CatalogusContradictionum catalogus_contradictionum[TOPICUM_NUMERUS] = {
    [TOPICUM_IUSTITIA]    = { contradictiones_iustitia,    NUMERUS_ELEMENTORUM(contradictiones_iustitia) },
    [TOPICUM_VIRTUS]      = { contradictiones_virtus,      NUMERUS_ELEMENTORUM(contradictiones_virtus) },
    [TOPICUM_PIETAS]      = { contradictiones_pietas,      NUMERUS_ELEMENTORUM(contradictiones_pietas) },
    [TOPICUM_FORTITUDO]   = { contradictiones_fortitudo,   NUMERUS_ELEMENTORUM(contradictiones_fortitudo) },
    [TOPICUM_AMICITIA]    = { contradictiones_amicitia,    NUMERUS_ELEMENTORUM(contradictiones_amicitia) },
    [TOPICUM_SAPIENTIA]   = { contradictiones_sapientia,   NUMERUS_ELEMENTORUM(contradictiones_sapientia) },
    [TOPICUM_VOLUPTAS]    = { contradictiones_voluptas,    NUMERUS_ELEMENTORUM(contradictiones_voluptas) },
    [TOPICUM_MORS]        = { contradictiones_mors,        NUMERUS_ELEMENTORUM(contradictiones_mors) },
    [TOPICUM_ANIMA]       = { contradictiones_anima,       NUMERUS_ELEMENTORUM(contradictiones_anima) },
    [TOPICUM_PULCHRITUDO] = { contradictiones_pulchritudo, NUMERUS_ELEMENTORUM(contradictiones_pulchritudo) },
    [TOPICUM_VERITAS]     = { contradictiones_veritas,     NUMERUS_ELEMENTORUM(contradictiones_veritas) },
    [TOPICUM_SCIENTIA]    = { contradictiones_scientia,    NUMERUS_ELEMENTORUM(contradictiones_scientia) },
    [TOPICUM_IGNOTUM]     = { NULL, 0 }
};

/* ===================================================================
 * SECTIO SEPTIMA — Colloquium Scriptum (Defaltum)
 * =================================================================*/

/*
 * Quattuordecim (vel plures) lineae discipuli.  Hae linea per tractum
 * automato ipsi traduntur, ita ut dialogum totum monstret progressus
 * machinae status.  Tria vel quattuor topica tanguntur ut machina
 * transitum clare ostendat.
 */
static const char *const linae_discipuli_scripti[] = {
    "Socrates, iustitia est reddere unicuique quod debetur.",
    "Omnis iustus vir semper debita reddit; numquam fallit.",
    "Sed iustum est etiam amicis benefacere et inimicis nocere.",
    "Non intellego; quid igitur est iustitia vera?",
    "Forsitan ergo virtus ipsa est; dicunt virtutem esse cognitionem.",
    "Virtus doceri potest, sicut geometria aut ars navalis.",
    "At filii Periclis non fuerunt similes patri in virtute. Cur?",
    "Dubito iam; fortasse virtus dono deorum venit, non arte.",
    "Transeamus ad pietatem: pium est id quod deis placet.",
    "At discordia inter deos est; nemo simul placet omnibus.",
    "Nescio iam quid sit pietas; videor nihil firmum tenere.",
    "Fortitudo vero facilior est: fortis est qui nihil timet.",
    "Sed miles stultus nihil timet quia ignorat; estne fortis?",
    "Socrates, confiteor me in aporia esse; doce me, obsecro.",
    "Quid ergo reliquum est nobis? Doce me aliquid certum.",
    "Gratias tibi ago; discedo sapientior, quia scio me nescire."
};

/* ===================================================================
 * SECTIO OCTAVA — Functiones Auxiliares: Stringa et Tokenisatio
 * =================================================================*/

/* Convertit vocabulum ad minusculas in situ */
static void
minusculos_facere(char *s)
{
    for (; *s; ++s) *s = (char)tolower((unsigned char)*s);
}

/* Purgat characteres interpunctionis ab extremis vocabuli */
static void
purga_interpunctionem(char *s)
{
    size_t n = strlen(s);
    while (n > 0 && ispunct((unsigned char)s[n-1])) s[--n] = '\0';
    size_t k = 0;
    while (s[k] && ispunct((unsigned char)s[k])) k++;
    if (k > 0) memmove(s, s + k, n - k + 1);
}

/* Verificat si vocabulum est in indice claim markers */
static bool
est_claim_marker(const char *v)
{
    for (int i = 0; verba_claim[i] != NULL; ++i)
        if (strcmp(verba_claim[i], v) == 0) return true;
    return false;
}

/* Iudicat topicum ex vocabulo (redit TOPICUM_IGNOTUM si nullum apt) */
static TopicumDialogi
topicum_ex_vocabulo(const char *v)
{
    for (int i = 0; i < NUMERUS_ELEMENTORUM(tabula_topicorum); ++i) {
        const EntriumTopici *e = &tabula_topicorum[i];
        if (strcmp(e->v1, v) == 0 || strcmp(e->v2, v) == 0
         || strcmp(e->v3, v) == 0 || strcmp(e->v4, v) == 0)
            return e->topicum;
    }
    return TOPICUM_IGNOTUM;
}

/* Copiat stringam cum cautione (non excessurus limitem) */
static void
copia_tuto(char *restrict dest, const char *restrict orig, size_t maxlen)
{
    if (maxlen == 0) return;
    size_t n = strlen(orig);
    if (n >= maxlen) n = maxlen - 1;
    memcpy(dest, orig, n);
    dest[n] = '\0';
}

/* ===================================================================
 * SECTIO NONA — Analysis Lineae Interlocutoris
 * =================================================================*/

static void
tokenisa_et_analysa(const char *linea, LineaAnalysata *out)
{
    memset(out, 0, sizeof(*out));
    out->topicum = TOPICUM_IGNOTUM;

    /* Vocabulum per spatium separatum */
    const char *p = linea;
    while (*p && out->numerus_vocabulorum < VOCABULA_PER_LINEAM_MAX) {
        while (*p && isspace((unsigned char)*p)) p++;
        if (!*p) break;
        const char *init = p;
        while (*p && !isspace((unsigned char)*p)) p++;
        size_t lg = (size_t)(p - init);
        if (lg >= VOCABULUM_MAX) lg = VOCABULUM_MAX - 1;
        Vocabulum *v = &out->vocabula[out->numerus_vocabulorum];
        memcpy(v->verbum, init, lg);
        v->verbum[lg] = '\0';
        v->longitudo = (uint8_t)strlen(v->verbum);
        v->est_nomen = (v->verbum[0] && isupper((unsigned char)v->verbum[0])) ? 1 : 0;
        purga_interpunctionem(v->verbum);
        minusculos_facere(v->verbum);
        v->est_claim = est_claim_marker(v->verbum) ? 1 : 0;
        out->numerus_vocabulorum++;
    }

    /* Indicia: verifica praesentiam indicum fortium */
    int pondus = 0;
    for (int i = 0; i < out->numerus_vocabulorum; ++i) {
        const char *w = out->vocabula[i].verbum;
        if (strcmp(w, "est")     == 0) { out->analysis.est_praesens     = 1; pondus += 1; }
        if (strcmp(w, "sunt")    == 0) { out->analysis.sunt_praesens    = 1; pondus += 1; }
        if (strcmp(w, "omnis")   == 0
         || strcmp(w, "omnes")   == 0) { out->analysis.omnis_praesens   = 1; pondus += 3; }
        if (strcmp(w, "nemo")    == 0
         || strcmp(w, "nullus")  == 0) { out->analysis.nemo_praesens    = 1; pondus += 3; }
        if (strcmp(w, "semper")  == 0) { out->analysis.semper_praesens  = 1; pondus += 3; }
        if (strcmp(w, "numquam") == 0) { out->analysis.numquam_praesens = 1; pondus += 3; }
        if (strcmp(w, "bonum")   == 0) { out->analysis.bonum_praesens   = 1; pondus += 2; }
        if (strcmp(w, "malum")   == 0) { out->analysis.malum_praesens   = 1; pondus += 2; }
        if (strcmp(w, "iustum")  == 0) { out->analysis.iustum_praesens  = 1; pondus += 2; }
        if (strcmp(w, "iniustum")== 0) { out->analysis.iniustum_praesens= 1; pondus += 2; }
        if (strcmp(w, "non")     == 0
         || strcmp(w, "haud")    == 0) { out->analysis.negatio          = 1; pondus += 1; }

        /* Topicum: primum vocabulum apt inditur */
        if (out->topicum == TOPICUM_IGNOTUM) {
            TopicumDialogi t = topicum_ex_vocabulo(w);
            if (t != TOPICUM_IGNOTUM) out->topicum = t;
        }
    }
    /* Interrogatio? */
    size_t ll = strlen(linea);
    if (ll > 0 && linea[ll - 1] == '?') out->analysis.interrogatio = 1;

    out->analysis.pondus = (uint16_t)pondus;

    /* Extrahit nomen primum non-claim (conjectura pro substitutione) */
    for (int i = 0; i < out->numerus_vocabulorum; ++i) {
        const Vocabulum *v = &out->vocabula[i];
        if (v->est_claim) continue;
        if (v->longitudo < 3) continue;
        if (!out->nomen_extractum[0]) {
            copia_tuto(out->nomen_extractum, v->verbum, VOCABULUM_MAX);
        } else if (!out->adiectivum_extractum[0]) {
            copia_tuto(out->adiectivum_extractum, v->verbum, VOCABULUM_MAX);
            break;
        }
    }
    if (!out->nomen_extractum[0])
        copia_tuto(out->nomen_extractum, "hoc", VOCABULUM_MAX);
    if (!out->adiectivum_extractum[0])
        copia_tuto(out->adiectivum_extractum, "tale", VOCABULUM_MAX);
}

/* ===================================================================
 * SECTIO DECIMA — Gubernator Status Machinae
 * =================================================================*/

/*
 * Transitiones coguntur per regulam simplicem:
 *  - IRONIA manet primum turnum
 *  - ELENCHUS incipit turnus 2
 *  - APORIA si refutationes >= REFUTATIONES_LIMES vel turnus >= TURNI_AD_APORIAM
 *  - MAIEUTICA post APORIAM, turnus >= TURNI_AD_MAIEUTICAM
 *  - CONCLUSIO ultima, turnus >= TURNI_AD_CONCLUSIONEM
 */
static StatusDialogi
proximus_status(const Dialogus *d, const LineaAnalysata *la)
{
    StatusDialogi s = d->status;
    int t = d->turnus;
    int r = d->refutationes;

    /* Assertiones fortes (omnis, semper, numquam) comes pondus */
    bool forte = la->analysis.omnis_praesens
              || la->analysis.nemo_praesens
              || la->analysis.semper_praesens
              || la->analysis.numquam_praesens;

    if (s == STATUS_IRONIA && t >= 1) s = STATUS_ELENCHUS;
    if (s == STATUS_ELENCHUS && (r >= REFUTATIONES_LIMES || t >= TURNI_AD_APORIAM))
        s = STATUS_APORIA;
    if (s == STATUS_APORIA && t >= TURNI_AD_MAIEUTICAM)
        s = STATUS_MAIEUTICA;
    if (s == STATUS_MAIEUTICA && t >= TURNI_AD_CONCLUSIONEM)
        s = STATUS_CONCLUSIO;

    /* Claim fortissimus accelerat transitum ad elenchum */
    if (s == STATUS_IRONIA && forte) s = STATUS_ELENCHUS;

    return s;
}

/* ===================================================================
 * SECTIO UNDECIMA — Substitutio Templatae
 * =================================================================*/

/*
 * Scribit respondsum ex templa, substituens %N, %A, %T.
 * Scribit non plus quam maxlen - 1 characters plus '\0'.
 */
static void
substitue_templam(char *restrict dest, size_t maxlen,
                  const char *restrict templa,
                  const char *restrict nomen,
                  const char *restrict adiectivum,
                  const char *restrict topicum_nomen)
{
    size_t k = 0;
    if (maxlen == 0) return;
    for (const char *p = templa; *p && k + 1 < maxlen; ++p) {
        if (*p == '%' && p[1]) {
            const char *substitutum = NULL;
            switch (p[1]) {
                case 'N': substitutum = nomen; break;
                case 'A': substitutum = adiectivum; break;
                case 'T': substitutum = topicum_nomen; break;
                default:  substitutum = NULL; break;
            }
            if (substitutum) {
                size_t ls = strlen(substitutum);
                size_t copi = MINIMUM(ls, maxlen - 1 - k);
                memcpy(dest + k, substitutum, copi);
                k += copi;
                p++;  /* salire super codicem */
                continue;
            }
        }
        dest[k++] = *p;
    }
    dest[k] = '\0';
}

/* ===================================================================
 * SECTIO DUODECIMA — Elige Templam (Dispatch per Status+Topicum)
 * =================================================================*/

static const char *
elige_templam(uint64_t *restrict semen, StatusDialogi s, TopicumDialogi t)
{
    /* Prima passata: collige entria cum (s == S) && (t == T) */
    int indices[32];
    int n = 0;
    for (int i = 0; i < NUMERUS_ELEMENTORUM(tabula_templorum) && n < 32; ++i) {
        const EntriumTemplae *e = &tabula_templorum[i];
        if (e->status == s && e->topicum == t) indices[n++] = i;
    }
    /* Si nihil, tenta cum topico quocumque pro hoc statu */
    if (n == 0) {
        for (int i = 0; i < NUMERUS_ELEMENTORUM(tabula_templorum) && n < 32; ++i) {
            const EntriumTemplae *e = &tabula_templorum[i];
            if (e->status == s) indices[n++] = i;
        }
    }
    if (n == 0) return "Quid dicis, amice?";
    int k = indices[fortuitus_sub(semen, (uint64_t)n)];
    return tabula_templorum[k].templa;
}

/* ===================================================================
 * SECTIO TERTIA DECIMA — Motor Contradictionis
 * =================================================================*/

/*
 * Si interlocutor profert claim fortem, inspice catalogum
 * contradictionum pro topico currenti; elige unum par et redi
 * quaestionem ducentem ad ~P.
 */
static const char *
elige_contradictionem(uint64_t *restrict semen, TopicumDialogi t)
{
    if (t < 0 || t >= TOPICUM_NUMERUS) return NULL;
    const CatalogusContradictionum *c = &catalogus_contradictionum[t];
    if (c->numerus == 0 || c->paria == NULL) return NULL;
    int k = (int)fortuitus_sub(semen, (uint64_t)c->numerus);
    return c->paria[k].quaestio_ducens;
}

/* ===================================================================
 * SECTIO QUARTA DECIMA — Generator Responsi Socratici
 * =================================================================*/

/*
 * Compage totius automati: accepta linea discipuli, producit
 * responsum Socratis, updatque statum dialogi.
 */
static void
genera_responsum(Dialogus *d, const char *linea_in, char *restrict out, size_t maxlen)
{
    LineaAnalysata la;
    tokenisa_et_analysa(linea_in, &la);

    /* Updata topicum currens */
    if (la.topicum != TOPICUM_IGNOTUM) {
        d->topicum_antecedens = d->topicum_currens;
        d->topicum_currens = la.topicum;
    }
    /* Memoriza nomen et adiectivum pro substitutione */
    if (la.nomen_extractum[0])
        copia_tuto(d->ultimum_nomen, la.nomen_extractum, VOCABULUM_MAX);
    if (la.adiectivum_extractum[0])
        copia_tuto(d->ultimum_adiectivum, la.adiectivum_extractum, VOCABULUM_MAX);

    /* Accumula refutationes si claim fortis */
    bool claim_fortis = la.analysis.omnis_praesens
                     || la.analysis.nemo_praesens
                     || la.analysis.semper_praesens
                     || la.analysis.numquam_praesens
                     || (la.analysis.pondus >= 4);
    if (claim_fortis) {
        d->numerus_claim_fortium++;
        d->refutationes++;
    }

    /* Deliberatur proximus status */
    d->status = proximus_status(d, &la);

    /* Interdum — in statu ELENCHUS cum claim fortis — utere motore
     * contradictionis pro quaestione directe ducente ad ~P. */
    const char *templa = NULL;
    if (d->status == STATUS_ELENCHUS && claim_fortis
        && d->topicum_currens != TOPICUM_IGNOTUM
        && (fortuitus_sub(&d->semen_prng, 3) != 0))
    {
        templa = elige_contradictionem(&d->semen_prng, d->topicum_currens);
    }
    if (!templa) {
        templa = elige_templam(&d->semen_prng, d->status, d->topicum_currens);
    }

    substitue_templam(out, maxlen, templa,
                      d->ultimum_nomen,
                      d->ultimum_adiectivum,
                      nomina_topicorum[d->topicum_currens]);
    d->turnus++;
}

/* ===================================================================
 * SECTIO QUINTA DECIMA — Administratio Historiae (Membrum Flexibile)
 * =================================================================*/

/*
 * Historia turnorum conservatur in catena dynamica;
 * utitur reallocatione cum capacitas augenda est.
 */
static void
historia_adde(Dialogus *d, const char *linea)
{
    if (d->longitudo_historiae >= d->capacitas_historiae) {
        int nc = d->capacitas_historiae ? d->capacitas_historiae * 2 : 16;
        char **novum = realloc(d->historia, (size_t)nc * sizeof(char *));
        if (!novum) return;
        d->historia = novum;
        d->capacitas_historiae = nc;
    }
    size_t lg = strlen(linea);
    char *c = malloc(lg + 1);
    if (!c) return;
    memcpy(c, linea, lg + 1);
    d->historia[d->longitudo_historiae++] = c;
}

static void
historia_libera(Dialogus *d)
{
    for (int i = 0; i < d->longitudo_historiae; ++i) free(d->historia[i]);
    free(d->historia);
    d->historia = NULL;
    d->longitudo_historiae = 0;
    d->capacitas_historiae = 0;
}

/* ===================================================================
 * SECTIO SEXTA DECIMA — Tabulae Dispatch per Functiones-Pointer
 * =================================================================*/

/*
 * Exemplum tabulae dispatch: una functio pro quoque statu,
 * si vis tractare transitum specifice.  Hic minimaliter
 * implementatur ad demonstrandum stylum.
 */
typedef void (*PraefatioStatus)(const Dialogus *d, FILE *fp);

static void praefa_ironia(const Dialogus *d, FILE *fp) {
    (void)d; (void)fp;
}
static void praefa_elenchus(const Dialogus *d, FILE *fp) {
    (void)d; (void)fp;
}
static void praefa_aporia(const Dialogus *d, FILE *fp) {
    (void)d; (void)fp;
}
static void praefa_maieutica(const Dialogus *d, FILE *fp) {
    (void)d; (void)fp;
}
static void praefa_conclusio(const Dialogus *d, FILE *fp) {
    (void)d; (void)fp;
}

static const PraefatioStatus tabula_praefationum[STATUS_NUMERUS] = {
    [STATUS_IRONIA]    = praefa_ironia,
    [STATUS_ELENCHUS]  = praefa_elenchus,
    [STATUS_APORIA]    = praefa_aporia,
    [STATUS_MAIEUTICA] = praefa_maieutica,
    [STATUS_CONCLUSIO] = praefa_conclusio
};

/* ===================================================================
 * SECTIO SEPTIMA DECIMA — Initializatio Dialogi
 * =================================================================*/

static void
dialogus_init(Dialogus *d, uint64_t semen)
{
    memset(d, 0, sizeof(*d));
    d->status = STATUS_IRONIA;
    d->turnus = 0;
    d->refutationes = 0;
    d->numerus_claim_fortium = 0;
    d->topicum_currens = TOPICUM_IGNOTUM;
    d->topicum_antecedens = TOPICUM_IGNOTUM;
    d->semen_prng = semen ? semen : SEMEN_DEFALTUM;
    copia_tuto(d->ultimum_nomen, "hoc", VOCABULUM_MAX);
    copia_tuto(d->ultimum_adiectivum, "tale", VOCABULUM_MAX);
}

/* ===================================================================
 * SECTIO OCTAVA DECIMA — Modus Scriptus (Colloquium Internum)
 * =================================================================*/

static int
exequi_colloquium_scriptum(uint64_t semen)
{
    Dialogus d;
    dialogus_init(&d, semen);

    /* Minimus titulus ornatus: numerus fortuitus inter 1 et 99 */
    int ornatus = fortuitus_inter(&d.semen_prng, 1, 99);
    printf("=== Colloquium Socraticum (numerus %d) ===\n", ornatus);
    printf("(semen generatoris: %" PRIu64 ")\n\n", d.semen_prng);

    char responsum[RESPONSIO_MAX];
    int n_linearum = NUMERUS_ELEMENTORUM(linae_discipuli_scripti);

    for (int i = 0; i < n_linearum; ++i) {
        const char *linea = linae_discipuli_scripti[i];
        printf("Discipulus: %s\n", linea);
        genera_responsum(&d, linea, responsum, sizeof(responsum));
        tabula_praefationum[d.status](&d, stdout);
        printf("Socrates [%s/%s]: %s\n\n",
               nomina_statuum[d.status],
               nomina_topicorum[d.topicum_currens],
               responsum);
        historia_adde(&d, linea);
        historia_adde(&d, responsum);
    }

    printf("=== Finis Colloquii ===\n");
    printf("(turni peracti: %d; refutationes: %d; status ultimus: %s)\n",
           d.turnus, d.refutationes, nomina_statuum[d.status]);

    historia_libera(&d);
    return 0;
}

/* ===================================================================
 * SECTIO NONA DECIMA — Modus Interactivus (stdin)
 * =================================================================*/

static int
exequi_modum_interactivum(uint64_t semen)
{
    Dialogus d;
    dialogus_init(&d, semen);
    char linea[LONGITUDO_LINEAE_MAX];
    char responsum[RESPONSIO_MAX];

    while (fgets(linea, (int)sizeof(linea), stdin) != NULL) {
        /* Purga newline trail */
        size_t n = strlen(linea);
        while (n > 0 && (linea[n-1] == '\n' || linea[n-1] == '\r'))
            linea[--n] = '\0';
        if (n == 0) continue;
        genera_responsum(&d, linea, responsum, sizeof(responsum));
        printf("%s\n", responsum);
        fflush(stdout);
        historia_adde(&d, linea);
        historia_adde(&d, responsum);
    }
    historia_libera(&d);
    return 0;
}

/* ===================================================================
 * SECTIO VICESIMA — Auxilia Argumentorum
 * =================================================================*/

static void
usus_breviter(FILE *fp, const char *nomen_prog)
{
    fprintf(fp, "Usus: %s [-i] [-s N]\n", nomen_prog);
    fprintf(fp, "  (sine argumentis) colloquium internum exhibet\n");
    fprintf(fp, "  -i         modus interactivus (linea per lineam ex stdin)\n");
    fprintf(fp, "  -s N       semen generatoris pseudo-fortuiti\n");
}

/* Parsa numerum ex stringa; redit true si successit */
static bool
parsa_numerum_u64(const char *s, uint64_t *out)
{
    if (!s || !*s) return false;
    char *fin = NULL;
    /* Nota: strtoull accipit praefixa 0x pro hexadecimali */
    unsigned long long v = strtoull(s, &fin, 0);
    if (fin == s || (fin && *fin != '\0')) return false;
    *out = (uint64_t)v;
    return true;
}

/* ===================================================================
 * SECTIO VICESIMA PRIMA — Porta Principalis
 * =================================================================*/

int
main(int argc, char **argv)
{
    bool modus_interact = false;
    uint64_t semen = SEMEN_DEFALTUM;

    for (int i = 1; i < argc; ++i) {
        const char *a = argv[i];
        if (strcmp(a, "-i") == 0) {
            modus_interact = true;
        } else if (strcmp(a, "-s") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "socrates: argumentum '-s' exigit numerum\n");
                usus_breviter(stderr, argv[0]);
                return 2;
            }
            if (!parsa_numerum_u64(argv[++i], &semen)) {
                fprintf(stderr, "socrates: numerus invalidus pro seminte: '%s'\n", argv[i]);
                return 2;
            }
        } else {
            fprintf(stderr, "socrates: argumentum ignotum: '%s'\n", a);
            usus_breviter(stderr, argv[0]);
            return 2;
        }
    }

    if (modus_interact) return exequi_modum_interactivum(semen);
    return exequi_colloquium_scriptum(semen);
}

/* ===================================================================
 * SECTIO VICESIMA SECUNDA — Appendix: Notae Criticae et Meditationes
 * =================================================================*/

/*
 * Nota I — De electione generatoris xorshift64:
 *   Velocissimus est et sufficit pro choice templorum; non
 *   pro cryptographia usurpandus, sed hic de cryptographia
 *   non agitur.  Periodus ~2^64 - 1 satis ampla.
 *
 * Nota II — De tabulis designatis:
 *   C99 permittit inicializatores designatos, qui perpulchre
 *   clarificant relationem inter indicem enumerationis et valorem
 *   tabulae.  Vide 'nomina_topicorum', 'nomina_statuum',
 *   'catalogus_contradictionum', 'tabula_praefationum'.
 *
 * Nota III — De machina status:
 *   Transitiones non sunt pure deterministicae in turnis: claim
 *   fortissimus potest transitum accelerare.  Sic machina
 *   'reagit' ad inputum discipuli, non mere tempus sequitur.
 *
 * Nota IV — De motore contradictionis:
 *   Utitur tabula manualiter composita, non algorithmo
 *   inferentiae.  Sic responsum proprio modo socraticum est:
 *   sciens paradoxum quaerit, non casu invenit.
 *
 * Nota V — De topico IGNOTO:
 *   Si nullum vocabulum topicum confirmatum invenitur, automaton
 *   ad topicum ignotum recurrit et quaestionem clarificationis
 *   proferit.  Hoc pro robustness contra inputum inexpectatum.
 *
 * Nota VI — De substitutione templatae:
 *   Signum %N pro nomine extracto, %A pro adiectivo, %T pro
 *   nomine topici.  Si nihil extractum est, defaltum ('hoc',
 *   'tale') usurpatur.  Sic nulla stringa vacua apparet.
 *
 * Nota VII — De memoria:
 *   Historia turnorum in heap allocatur et crescit per duplicationem.
 *   Ad finem liberatur per historia_libera().  Nulla memoria
 *   amittitur, etiam si automaton longissime currit.
 *
 * Nota VIII — De linguis mixtis:
 *   Omnia latine; exceptio sola sunt nomina bibliothecarum
 *   standardium, quae per linguam C anglice scripta manent.
 *   Hoc non est pollutio sed necessitas technica.
 *
 * Nota IX — De methodologia Socratis:
 *   Vera methodus Socratis (Platonis testimonio) non admittit
 *   conclusionem firmam in pluribus dialogis: manet in aporia
 *   ut discipulus sponte quaerat.  Automaton nostrum hoc imitatur
 *   per statum APORIA et MAIEUTICA, ubi responsa verteunt
 *   ad interiorem inspectionem.
 *
 * Nota X — De extensibilitate:
 *   Addere topicum novum: addita entria in TABULA_TOPICORUM
 *   (X-macron), addita templata in quinque TEMPLATA_*, addita
 *   contradictiones, addita nomen in 'nomina_topicorum'.
 *   Nulla functio mutanda, tantum tabulae.  Hoc est vis
 *   initializatorum designatorum et X-macronis.
 *
 * Nota XI — De securitate stringarum:
 *   Nusquam strcpy sine limite, nusquam sprintf sine n-variante.
 *   Semper longitudo prodest.  copia_tuto() adiutrix centralis.
 *
 * Nota XII — De typis integris:
 *   Utimur uint64_t pro prng, uint16_t pro pondere, uint8_t
 *   pro flagis brevibus.  Specificitas typorum augeat claritatem
 *   et minuit surprises in diversis architecturis.
 *
 * Nota XIII — De function-pointer dispatch:
 *   Tabula 'tabula_praefationum' monstrat stylum dispatch per
 *   function pointer, etiamsi hic corpus functionum vacuum
 *   relictum sit.  Extensio facile per substituerem corpora.
 *
 * Nota XIV — Finis commentarii.
 *   Valeas, lector; et nunquam obliviscaris: unum hoc scio,
 *   me nihil scire.  — Socrates
 */

/* FINIS LIBELLI */
