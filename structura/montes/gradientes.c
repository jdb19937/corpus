/*
 * gradientes.c — Computatio gradientium. Gradiens (8 B = 2 floats)
 * per valorem: probat ABI pro structa quae unum register implet.
 */
#include "montes.h"

Gradiens
grad_in(const MonsData *m, int i, int j)
{
    int i0 = i > 0 ? i - 1 : i;
    int i1 = i < GRID_N - 1 ? i + 1 : i;
    int j0 = j > 0 ? j - 1 : j;
    int j1 = j < GRID_N - 1 ? j + 1 : j;
    Gradiens g;
    g .dx = 0.5f * (m->grid[i1][j].alt_m - m->grid[i0][j].alt_m);
    g .dy = 0.5f * (m->grid[i][j1].alt_m - m->grid[i][j0].alt_m);
    return g;
}

double
slope_max(const MonsData *m)
{
    double s = 0.0;
    for (int i = 0; i < GRID_N; i++)
        for (int j = 0; j < GRID_N; j++)
            if ((double)m->grid[i][j].slope > s)
                s = m->grid[i][j].slope;
    return s;
}

Gradiens
grad_medianum(const MonsData *m)
{
    return grad_in(m, GRID_N / 2, GRID_N / 2);
}
