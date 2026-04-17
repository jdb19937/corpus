/*
 * motor.c — Motor inferentiae forward-chaining.
 *
 * Dum aliqua regula novum factum addit, motor iterum per omnes
 * regulas percurrit. Regulae firing sequentiam per `explicatio_adde`
 * notat ut explicatio postea tracked possit.
 */
#include "peritus.h"

int
motor_currit(void)
{
    int total = 0;
    int changed = 1;
    while (changed) {
        changed = 0;
        for (int r = 0; r < N_REGULARUM; r++) {
            const Regula *R = &regulae[r];
            if (factum_habet(R->consequens)) continue;

            int omnia_adsunt = 1;
            for (int a = 0; a < R->n_ante; a++) {
                if (!factum_habet(R->ante[a])) {
                    omnia_adsunt = 0;
                    break;
                }
            }
            if (!omnia_adsunt) continue;

            int idx = factum_adde(R->consequens);
            if (idx >= 0) {
                explicatio_adde(r, idx);
                total++;
                changed = 1;
            }
        }
    }
    return total;
}
