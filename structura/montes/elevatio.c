/*
 * elevatio.c — Interpolatio bilinearis, cella mediana, summa
 * altitudinum.
 *
 * cella_mediana reddit Cella (12 B) per valorem — probat ABI pro
 * parva mixta struct (2 floats + 2 int16).
 */
#include "montes.h"

float
interp_bilin(const MonsData *m, float u, float v)
{
    if (u < 0.0f) u = 0.0f; if (u > 1.0f) u = 1.0f;
    if (v < 0.0f) v = 0.0f; if (v > 1.0f) v = 1.0f;
    float fx = u * (GRID_N - 1);
    float fy = v * (GRID_N - 1);
    int   i0 = (int)fx, j0 = (int)fy;
    int   i1 = i0 + 1, j1 = j0 + 1;
    if (i1 >= GRID_N) i1 = GRID_N - 1;
    if (j1 >= GRID_N) j1 = GRID_N - 1;
    float tx = fx - i0, ty = fy - j0;
    float h00 = m->grid[i0][j0].alt_m;
    float h10 = m->grid[i1][j0].alt_m;
    float h01 = m->grid[i0][j1].alt_m;
    float h11 = m->grid[i1][j1].alt_m;
    float a   = h00 * (1.0f - tx) + h10 * tx;
    float b   = h01 * (1.0f - tx) + h11 * tx;
    return a * (1.0f - ty) + b * ty;
}

Cella
cella_mediana(const MonsData *m)
{
    int ci = GRID_N / 2;
    int cj = GRID_N / 2;
    return m->grid[ci][cj];
}

double
altitudo_summa(const MonsData *m)
{
    double s = 0.0;
    for (int i = 0; i < GRID_N; i++)
        for (int j = 0; j < GRID_N; j++)
            s += (double)m->grid[i][j].alt_m;
    return s;
}
