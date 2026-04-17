/*
 * memoria.c — Circulus memoriae recentium themarum aegrotantis.
 *
 * Status privatus (static): solum per API publica accessibilis,
 * linker non videt tabulas sed tantum functiones publicae.
 */
#include "eliza.h"
#include <string.h>

#define MAX_MEMORIAE  6
#define LONG_MEMORIAE 128

static char memoriae[MAX_MEMORIAE][LONG_MEMORIAE];
static int  n_mem = 0;
static int  idx_mem = 0;

void
memoria_adde(const char *text)
{
    if (!text || !*text) return;
    strncpy(memoriae[idx_mem], text, LONG_MEMORIAE - 1);
    memoriae[idx_mem][LONG_MEMORIAE - 1] = '\0';
    idx_mem = (idx_mem + 1) % MAX_MEMORIAE;
    if (n_mem < MAX_MEMORIAE) n_mem++;
}

const char *
memoria_extrahe(void)
{
    if (n_mem == 0) return NULL;
    int j = (idx_mem - 1 + MAX_MEMORIAE) % MAX_MEMORIAE;
    return memoriae[j];
}

int
memoria_plena(void)
{
    return n_mem > 0;
}
