/*
 * terrenum.c — Generatio grid elevationis pro quinque montibus.
 * Seed fixus per srand in main — hic deterministicus.
 */
#include "montes.h"
#include <stdlib.h>
#include <math.h>

MonsData montes[] = {
    { "Everest",   27.9881,  86.9250, 8848.0, 1, {{{0}}} },
    { "K2",        35.8825,  76.5133, 8611.0, 2, {{{0}}} },
    { "Denali",    63.0695, -151.0074, 6190.0, 3, {{{0}}} },
    { "Kilimanjaro", -3.0674, 37.3556, 5895.0, 4, {{{0}}} },
    { "MonsOlympus", 18.65,  226.2,  21900.0, 5, {{{0}}} },  /* Mars */
};

const int N_MONTIUM = (int)(sizeof montes / sizeof montes[0]);

void
terrenum_init(void)
{
    for (int k = 0; k < N_MONTIUM; k++) {
        float hmax = (float)montes[k].altitudo_max_m;
        for (int i = 0; i < GRID_N; i++) {
            for (int j = 0; j < GRID_N; j++) {
                float u = ((float)i - (GRID_N - 1) * 0.5f) / (float)GRID_N;
                float v = ((float)j - (GRID_N - 1) * 0.5f) / (float)GRID_N;
                float d = sqrtf(u * u + v * v);
                float alt = hmax * (1.0f - d * 1.2f);
                if (alt < 0.0f) alt = 0.0f;
                montes[k].grid[i][j].alt_m = alt;
                montes[k].grid[i][j].slope = d * 45.0f;
                montes[k].grid[i][j].cat   = (int16_t)(alt > hmax * 0.5f ? 2 : 1);
                montes[k].grid[i][j].flux  = (int16_t)(i + j + k);
            }
        }
    }
}
