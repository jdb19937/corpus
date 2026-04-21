/*
 * catalogus.c — Data stellarum clararum (HD, magnitudo, coordinatae,
 * parallaxis, B-V). Initializatio aggregatorum cum nested Coord
 * probat ABI pro struct literalibus.
 */
#include "stellae.h"

const Stella catalogus[] = {
    { 48915LL,  -1.46, { 6.75, -16.7 }, 379.2,  0.01 },  /* Sirius   */
    { 60179LL,   1.58, { 7.58,  31.9 }, 63.27,  0.03 },  /* Castor   */
    { 62509LL,   1.14, { 7.75,  28.0 }, 96.54,  0.99 },  /* Pollux   */
    { 87901LL,   1.36, { 10.1,  11.9 }, 41.13, -0.11 },  /* Regulus  */
    { 124897LL, -0.05, { 14.2,  19.1 }, 88.83,  1.23 },  /* Arcturus */
    { 172167LL,  0.03, { 18.6,  38.7 }, 130.2,  0.00 },  /* Vega     */
    { 186791LL,  2.72, { 19.7,   8.8 }, 42.74,  1.52 },  /* Altair   */
    { 197345LL,  1.25, { 20.6,  45.2 }, 1.949, 0.09 },  /* Deneb    */
    { 39801LL,   0.42, { 5.91,   7.4 }, 6.550, 1.85 },  /* Betelgeuse */
    { 34085LL,   0.13, { 5.24,  -8.2 }, 6.560, -0.04 }, /* Rigel    */
    { 80007LL,   0.74, { 9.22, -69.7 }, 10.43,  1.44 },  /* Miaplacidus */
    { 45348LL,  -0.72, { 6.40, -52.6 }, 10.55,  0.15 },  /* Canopus  */
};

const int N_STELLARUM = (int)(sizeof catalogus / sizeof catalogus[0]);
