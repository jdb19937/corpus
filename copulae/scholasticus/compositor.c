/*
 * compositor.c — Assemblator partium articuli Scholastici.
 *
 * Assemblat quinque partes classicas: titulus, obiectiones, sed
 * contra, respondeo, et ad primum. Output formatus in stilo Summae.
 */
#include "scholasticus.h"
#include <stdio.h>

void
articulus_compone(const char *thesis, const char *thema)
{
    char obi_buf[2048];
    char sc_buf[512];
    char resp_buf[1024];
    char ad1_buf[512];

    printf("\n═══════════════════════════════════════\n");
    printf(" ARTICULUS: %s\n", thesis);
    printf("═══════════════════════════════════════\n");

    /* 1. Obiectiones (videtur quod non) */
    printf("\n─── Obiectiones ───\n");
    obiectio_compone(thema, obi_buf, sizeof obi_buf, MAX_OBIECTIONUM);
    printf("%s", obi_buf);

    /* 2. Sed contra */
    printf("\n─── Sed contra ───\n\n");
    sedcontra_compone(thema, sc_buf, sizeof sc_buf);
    printf("%s\n", sc_buf);

    /* 3. Respondeo dicendum */
    printf("\n─── Respondeo ───\n\n");
    responsio_compone(thesis, thema, resp_buf, sizeof resp_buf);
    printf("%s\n", resp_buf);

    /* 4. Ad primum */
    printf("\n─── Ad obiectiones ───\n\n");
    adprimum_compone(thema, ad1_buf, sizeof ad1_buf);
    printf("%s\n", ad1_buf);
}
