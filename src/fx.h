#ifndef FX_H
#define FX_H

#include "types.h"
#include "config.h"

typedef struct Game Game;

/* Texto flutuante (numeros de dano, "+XP", "LEVEL UP!" etc.). */
typedef struct {
    bool     active;
    Vec2     pos, vel;
    float    life, maxlife;
    uint32_t color;
    float    scale;
    char     text[16];
} Floater;

/* Particula simples (poeira de morte, faiscas). */
typedef struct {
    bool     active;
    Vec2     pos, vel;
    float    life, maxlife;
    float    radius;
    uint32_t color;
} Particle;

void fx_reset(Game *g);
void fx_floater(Game *g, Vec2 worldpos, const char *text, uint32_t color, float scale);
void fx_number(Game *g, Vec2 worldpos, int amount);
void fx_burst(Game *g, Vec2 worldpos, uint32_t color, int count, float speed);
void fx_shake(Game *g, float amount);
void fx_update(Game *g, float dt);
void fx_draw(Game *g);    /* desenha em coordenadas de mundo (usa a camera) */

#endif /* FX_H */
