/*
 * placae.c — Data placarum tectonicarum principalium.
 * Initializatio designata cum bitfields probat ABI pro
 * aggregatis heterogeneis.
 */
#include "tectonica.h"

const Placa placae[] = {
    { "Pacifica",        1, 0x0001, 1, 1, 0, { -6.0,  5.0 }, 103300000.0 },
    { "NorthAmericana",  2, 0x0002, 0, 1, 0, { -2.0,  0.5 },  75900000.0 },
    { "Eurasiatica",     3, 0x0004, 0, 1, 0, {  1.0,  0.5 },  67800000.0 },
    { "Africana",        4, 0x0008, 0, 1, 0, {  2.0,  1.5 },  61300000.0 },
    { "Antarctica",      5, 0x0010, 0, 1, 0, { -1.0,  0.0 },  60900000.0 },
    { "Australiana",     6, 0x0020, 0, 1, 0, {  7.0,  3.0 },  58900000.0 },
    { "SouthAmericana",  7, 0x0040, 0, 1, 0, {  1.5, -1.0 },  43600000.0 },
    { "Nazca",           8, 0x0080, 1, 1, 0, {  7.0, -2.0 },  15600000.0 },
    { "Indica",          9, 0x0100, 0, 1, 0, {  5.0,  5.0 },  11900000.0 },
    { "Philippinensis", 10, 0x0200, 1, 1, 0, { -8.0,  0.5 },   5500000.0 },
};

const int N_PLACARUM = (int)(sizeof placae / sizeof placae[0]);
