#ifndef STELLAE_H
#define STELLAE_H

#include <stdint.h>

/*
 * Coord — par coordinatarum caelestium; 2 doubles, fit in duo XMM /
 * FPR; probat parvum HFA passing.
 */
typedef struct {
    double ra;   /* ascensio recta (h)     */
    double dec;  /* declinatio (gradus)    */
} Coord;

/*
 * Stella — miscet int64_t et double; classificationi ABI difficilior.
 * Magnitudo = 8 + 8 + 16 + 8 + 8 = 48 bytes.
 */
typedef struct {
    int64_t   hd;           /* Henry Draper id                 */
    double    mag;          /* magnitudo visualis apparens     */
    Coord     pos;
    double    parallax_mas; /* milliarcsec                     */
    double    pmag_b_v;     /* B-V color index                 */
} Stella;

/* catalogus.c */
extern const Stella catalogus[];
extern const int    N_STELLARUM;

/* magnitudo.c */
double mag_absoluta(double m_app, double parallax_mas);
double luminositas_relativa(double m_abs);      /* solar units */
double flux_ratio(double m1, double m2);

/* coordinatae.c */
Coord  ad_galacticas(Coord eq);                 /* ex aequatorialibus */
double angulus_inter(Coord a, Coord b);         /* gradus             */
Coord  coord_medium(Coord a, Coord b);

/* qsort callback per function pointer — ABI pro struct* args */
int    cmp_mag(const void *a, const void *b);
int    cmp_hd(const void *a, const void *b);

#endif
