/*
 * regiones.c — Data regionum geographicarum (vertices approximati
 * in WGS-84 longitude/latitude).
 */
#include "provinciae.h"

const Regio regiones[] = {
    {
        "Sicilia", 5,
        {
            {12.4, 38.1}, {15.6, 38.3}, {15.1, 37.0},
            {13.5, 36.6}, {12.4, 37.8}
        },
        5.0, 100001LL,
    },
    {
        "Islandia", 6,
        {
            {-24.5, 63.4}, {-21.2, 65.5}, {-16.1, 66.5},
            {-13.5, 65.0}, {-14.7, 63.9}, {-21.8, 63.3}
        },
        0.38, 100002LL,
    },
    {
        "Cuba", 5,
        {
            {-84.9, 21.8}, {-82.6, 23.2}, {-78.6, 22.4},
            {-74.1, 20.2}, {-77.1, 19.8}
        },
        11.3, 100003LL,
    },
    {
        "Sardinia", 4,
        {
            {8.2, 41.2}, {9.8, 41.2}, {9.6, 39.0}, {8.4, 39.2}
        },
        1.6, 100004LL,
    },
    {
        "Taiwan", 4,
        {
            {120.1, 25.3}, {122.0, 24.6}, {121.0, 22.0}, {120.0, 22.6}
        },
        23.6, 100005LL,
    },
};

const int N_REGIONUM = (int)(sizeof regiones / sizeof regiones[0]);
