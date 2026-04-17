/*
 * sortio.c — Status RNG (xorshift32) inter omnes TU compartitus.
 *
 * Status privatus `state`; solum per functiones publicas mutari
 * potest. Grammatica.c per `sortio_elige` et `sortio_int` in hoc TU
 * vocat; vates.c pro seminatione.
 */
#include "vates.h"

static unsigned state = 2463534242u;

void
sortio_semen(unsigned seed)
{
    state = seed ? seed : 1u;
}

unsigned
sortio_nextu(void)
{
    unsigned x = state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    state = x ? x : 0x1badbeefu;
    return state;
}

int
sortio_int(int modulus)
{
    if (modulus <= 0) return 0;
    return (int)(sortio_nextu() % (unsigned)modulus);
}

const char *
sortio_elige(const char *const *arr, int n)
{
    if (n <= 0) return "";
    return arr[sortio_int(n)];
}
