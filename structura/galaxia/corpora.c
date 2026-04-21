/*
 * corpora.c — Data corporum galacticorum. Nested Vec3d initializatio
 * probat ABI pro aggregate literalibus.
 */
#include "galaxia.h"

Corpus corpora[] = {
    { "VialLactea", {  0.0,  0.0, 0.0 }, {  0.0,  0.0, 0.0 }, 1.000,   1 },
    { "Andromeda",  { 780.0, 0.0, 0.0 }, { -110.0, 10.0, 0.0 }, 1.500,  2 },
    { "Triangulum", { 900.0, 450.0, 0.0 }, { -70.0, -20.0, 0.0 }, 0.050, 3 },
    { "LMC",        {  50.0, -40.0, 0.0 }, { 40.0,  80.0, 0.0 }, 0.010, 4 },
    { "SMC",        {  60.0, -60.0, 0.0 }, { 30.0,  60.0, 0.0 }, 0.007, 5 },
    { "M32",        { 770.0,   5.0, 0.0 }, { -100.0, 5.0, 0.0 }, 0.003, 6 },
};

const int N_CORPORUM = (int)(sizeof corpora / sizeof corpora[0]);
