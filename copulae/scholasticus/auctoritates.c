/*
 * auctoritates.c — Basis citationum ex auctoribus scholasticae
 * traditionis (Aristoteles, Augustinus, Thomas, Averroes, Boethius,
 * Dionysius Areopagita, Avicenna, Anselmus).
 *
 * Unaquaeque auctoritas tagged est cum themis ad quas pertinet
 * (virtu, anima, deu, mort, etc.). Polaritas indicat an thesim
 * affirmare an negare tendit.
 */
#include "scholasticus.h"
#include <string.h>

const Auctoritas auctoritates[] = {
    { "Aristoteles", "Ethica Nicomachea II.1",
      "Virtus est habitus electivus in medio consistens.",
      { "virtu", "habit", NULL }, +1 },

    { "Aristoteles", "Physica II.8",
      "Natura nihil facit frustra.",
      { "natur", "finis", NULL }, +1 },

    { "Aristoteles", "De Anima III.5",
      "Intellectus agens est separabilis et immortalis.",
      { "anim", "immortal", "intell", NULL }, +1 },

    { "Aristoteles", "Metaphysica XII.7",
      "Primum movens immobile; actus purus.",
      { "deu", "motu", NULL }, +1 },

    { "Augustinus", "Confessiones I.1",
      "Fecisti nos ad te, Domine, et inquietum est cor nostrum donec requiescat in te.",
      { "deu", "beat", "anim", NULL }, +1 },

    { "Augustinus", "De Civitate Dei XIX.1",
      "Omnis homo naturaliter beatitudinem appetit.",
      { "beat", "appetit", NULL }, +1 },

    { "Augustinus", "De Trinitate X.11",
      "Anima memoriam sui, intelligentiam et voluntatem habet.",
      { "anim", "intell", NULL }, +1 },

    { "Thomas Aquinas", "Summa Theologica I q.2 a.3",
      "Ex motu, ex causa efficiente, ex possibili, ex gradibus, ex fine "
      "quinque viae demonstrant Deum esse.",
      { "deu", "motu", "caus", NULL }, +1 },

    { "Thomas Aquinas", "Summa Theologica I-II q.55 a.1",
      "Virtus est bonus habitus mentis, quo recte vivitur, quo nullus male utitur.",
      { "virtu", "habit", NULL }, +1 },

    { "Thomas Aquinas", "Summa Theologica I q.75 a.6",
      "Anima humana incorruptibilis est, quia per se subsistit.",
      { "anim", "immortal", NULL }, +1 },

    { "Averroes", "Commentarium Magnum in De Anima",
      "Intellectus agens unus est in omnibus hominibus, separatus et aeternus.",
      { "anim", "intell", NULL }, 0 },

    { "Boethius", "De Consolatione Philosophiae III",
      "Beatitudo est status omnium bonorum aggregatione perfectus.",
      { "beat", "fortun", NULL }, +1 },

    { "Dionysius", "De Divinis Nominibus IV",
      "Bonum diffusivum sui.",
      { "deu", "bon", NULL }, +1 },

    { "Avicenna", "Liber de Anima V.4",
      "Homo suspensus in aere cognoscit se esse etiam sine sensibus.",
      { "anim", "cognit", NULL }, +1 },

    { "Anselmus", "Proslogion II",
      "Id quo maius cogitari non potest: in re esse oportet, non solum in intellectu.",
      { "deu", "maius", NULL }, +1 },

    { "Protagoras", "apud Sextum Empiricum",
      "De diis neque ut sint neque ut non sint habeo dicere.",
      { "deu", "scept", NULL }, -1 },

    { "Cicero", "De Finibus V.16",
      "Virtus est animi habitus naturae modo atque rationi consentaneus.",
      { "virtu", "habit", NULL }, +1 },

    { "Seneca", "Epistulae 41",
      "Animus rectus, bonus, magnus — hic est deus in homine hospitatus.",
      { "anim", "deu", NULL }, +1 },
};

const int N_AUCTORITATUM =
    (int)(sizeof auctoritates / sizeof auctoritates[0]);

static int
thema_matches(const Auctoritas *a, const char *thema)
{
    for (int i = 0; i < MAX_THEMAE && a->themae[i]; i++)
        if (strstr(thema, a->themae[i]) || strstr(a->themae[i], thema))
            return 1;
    return 0;
}

int
auctoritas_inveni(const char *thema, int polaritas, int skip,
                  const Auctoritas **out)
{
    int seen = 0;
    for (int i = 0; i < N_AUCTORITATUM; i++) {
        const Auctoritas *a = &auctoritates[i];
        if (!thema_matches(a, thema)) continue;
        if (polaritas != 0 && a->polaritas != polaritas &&
            a->polaritas != 0) continue;
        if (seen == skip) { *out = a; return i; }
        seen++;
    }
    return -1;
}
