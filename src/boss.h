#ifndef BOSS_H
#define BOSS_H

#include "types.h"

typedef struct Game Game;

/* Definicao base de um chefe (antes do escalonamento por dificuldade). */
typedef struct {
    const char *name;
    float    hp;
    float    speed;
    float    radius;
    float    damage;
    uint32_t color;
    int      xp;
    int      coins;
} BossDef;

int         boss_count(void);
const char *boss_name(int boss_id);

/* Invoca o chefe correspondente ao indice de onda (cicla pela lista) e
 * registra g->boss_idx. */
void boss_spawn(Game *g, int wave_index);

/* Comportamento especial do chefe (chamado por enemies_update). */
void boss_update(Game *g, int enemy_idx, float dt);

#endif /* BOSS_H */
