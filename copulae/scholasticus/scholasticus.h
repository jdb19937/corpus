#ifndef SCHOLASTICUS_H
#define SCHOLASTICUS_H

#include <stddef.h>

#define MAX_THEMAE      6   /* per auctoritas: up to 6 tags */
#define MAX_OBIECTIONUM 3

typedef struct {
    const char *auctor;       /* "Aristoteles" */
    const char *opus;         /* "Physica II 8" */
    const char *citatio;      /* "Natura nihil facit frustra." */
    const char *themae[MAX_THEMAE];  /* NULL-terminated */
    int         polaritas;    /* +1 affirmat thesim; -1 negat; 0 neuter */
} Auctoritas;

/* auctoritates.c */
extern const Auctoritas auctoritates[];
extern const int        N_AUCTORITATUM;
int auctoritas_inveni(const char *thema, int polaritas, int skip,
                      const Auctoritas **out);

/* obiectio.c */
int obiectio_compone(const char *thesis_thema,
                     char *buf, size_t cap,
                     int max_obiectionum);

/* sedcontra.c */
int sedcontra_compone(const char *thesis_thema, char *buf, size_t cap);

/* responsio.c */
void responsio_compone(const char *thesis, const char *thesis_thema,
                       char *buf, size_t cap);
void adprimum_compone(const char *thesis_thema, char *buf, size_t cap);

/* compositor.c */
void articulus_compone(const char *thesis, const char *thesis_thema);

#endif
