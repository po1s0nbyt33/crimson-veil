#ifndef COLLECTION_H
#define COLLECTION_H

#include "types.h"

typedef struct Game Game;

typedef enum { CC_WEAPON, CC_FUSION, CC_PASSIVE, CC_CHARACTER } ColCat;

typedef struct {
    ColCat      cat;
    int         id;
    bool        unlocked;
    const char *name;       /* "???" se bloqueado */
    const char *desc;       /* "" se bloqueado */
    uint32_t    color;      /* preto se bloqueado */
} CollectionEntry;

/* Monta a lista completa de itens colecionaveis; retorna a quantidade. */
int collection_build(Game *g, CollectionEntry *out, int max);

#endif /* COLLECTION_H */
