/*
 * planetae.c — Tabula planetarum systematis solaris cum elementis
 * orbitalibus approximatis (epocha J2000).
 */
#include "orbes.h"

const Planeta planetae[] = {
    { "Mercurius", { 0.387,  0.2056, 0.1222, 0.5083, 0.8435, 3.0506, 1 }, 0.055 },
    { "Venus",     { 0.723,  0.0068, 0.0593, 0.9577, 1.3384, 0.8747, 2 }, 0.815 },
    { "Terra",     { 1.000,  0.0167, 0.0000, 1.7967, 0.0000, 6.2401, 3 }, 1.000 },
    { "Mars",      { 1.524,  0.0934, 0.0323, 5.0006, 0.8649, 0.3381, 4 }, 0.107 },
    { "Iuppiter",  { 5.203,  0.0484, 0.0227, 4.7790, 1.7535, 0.3489, 5 }, 317.8 },
    { "Saturnus",  { 9.537,  0.0542, 0.0434, 5.9235, 1.9838, 5.5331, 6 }, 95.16 },
    { "Uranus",    { 19.19,  0.0472, 0.0135, 1.6917, 1.2916, 2.4818, 7 }, 14.54 },
    { "Neptunus",  { 30.07,  0.0086, 0.0309, 4.7631, 2.3001, 4.4731, 8 }, 17.15 },
};

const int N_PLANETARUM = (int)(sizeof planetae / sizeof planetae[0]);
