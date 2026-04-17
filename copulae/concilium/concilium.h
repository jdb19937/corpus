#ifndef CONCILIUM_H
#define CONCILIUM_H

#include <stddef.h>

/*
 * Unaquaeque schola habet signaturam identicam: nomen, motto,
 * respondet. Polymorphismus per tabulam function-pointerorum
 * in quaestor.c dispachitur.
 */

typedef struct {
    const char *(*nomen)(void);
    const char *(*motto)(void);
    void        (*respondet)(const char *quaestio, char *buf, size_t cap);
} Schola;

/* stoici.c */
const char *stoici_nomen(void);
const char *stoici_motto(void);
void        stoici_respondet(const char *q, char *buf, size_t cap);

/* epicurei.c */
const char *epicurei_nomen(void);
const char *epicurei_motto(void);
void        epicurei_respondet(const char *q, char *buf, size_t cap);

/* peripatetici.c */
const char *peripatetici_nomen(void);
const char *peripatetici_motto(void);
void        peripatetici_respondet(const char *q, char *buf, size_t cap);

/* sceptici.c */
const char *sceptici_nomen(void);
const char *sceptici_motto(void);
void        sceptici_respondet(const char *q, char *buf, size_t cap);

/* quaestor.c — dispatcher */
extern const Schola scholae[];
extern const int    N_SCHOLARUM;
void quaestor_dispenda(const char *quaestio);

#endif
