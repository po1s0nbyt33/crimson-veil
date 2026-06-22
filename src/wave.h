#ifndef WAVE_H
#define WAVE_H

#include "types.h"

typedef struct Game Game;

/* Diretor de ondas: controla spawn continuo, escalonamento de dificuldade
 * por tempo e disparo de chefes em marcos de tempo. */
typedef struct {
    float time;            /* tempo total decorrido na partida (s) */
    float spawn_acc;       /* acumulador para taxa de spawn */
    int   kills;
    int   next_boss_at;    /* segundo em que o proximo chefe surge */
    int   bosses_spawned;
    float pack_acc;        /* acumulador para ondas em grupo ("packs") */
} WaveDirector;

void  wave_init(WaveDirector *w);
void  wave_update(Game *g, float dt);
float wave_difficulty(Game *g);   /* multiplicador de stats dos inimigos */
int   wave_minute(Game *g);       /* "round" atual = minutos decorridos */

#endif /* WAVE_H */
