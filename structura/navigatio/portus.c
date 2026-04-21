/*
 * portus.c — Tabula portuum principalium et functio quaere-per-nomen.
 */
#include "navigatio.h"
#include <string.h>

const Portus portus_tabula[] = {
    { "Lisboa",     {  38.72,   -9.14 }, 101 },
    { "Havana",     {  23.13,  -82.38 }, 102 },
    { "NewYork",    {  40.71,  -74.00 }, 103 },
    { "CapeTown",   { -33.92,   18.42 }, 104 },
    { "Sydney",     { -33.87,  151.21 }, 105 },
    { "Tokyo",      {  35.69,  139.69 }, 106 },
    { "Bombay",     {  19.08,   72.88 }, 107 },
    { "Rotterdam",  {  51.92,    4.48 }, 108 },
    { "Singapura",  {   1.30,  103.82 }, 109 },
    { "Valparaiso", { -33.05,  -71.62 }, 110 },
};

const int N_PORTUUM = (int)(sizeof portus_tabula / sizeof portus_tabula[0]);

const Portus *
    portus_quaere(const char *nom)
{
    for (int i = 0; i < N_PORTUUM; i++) {
        if (strcmp(portus_tabula[i].nomen, nom) == 0)
            return &portus_tabula[i];
    }
    return &portus_tabula[0];
}
