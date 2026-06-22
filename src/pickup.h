#ifndef PICKUP_H
#define PICKUP_H

#include "types.h"
#include "config.h"

typedef struct Game Game;

typedef enum {
    PICK_XP,      /* gema de experiencia */
    PICK_COIN,    /* moeda (progressao permanente) */
    PICK_HEAL,    /* recupera vida */
    PICK_MAGNET,  /* atrai todo o XP da tela */
    PICK_BOMB,    /* dano em todos os inimigos visiveis */
    PICK_TYPE_COUNT
} PickupType;

typedef struct {
    bool       active;
    PickupType type;
    Vec2       pos, vel;
    int        value;
    float      radius;
    bool       homing;   /* sendo puxado para o jogador */
    float      bob;      /* animacao */
} Pickup;

int  pickup_spawn(Game *g, PickupType t, Vec2 pos, int value);
void pickups_update(Game *g, float dt);
void pickups_draw(Game *g);
void pickups_magnet_all(Game *g);   /* item ima: puxa todo o XP */

#endif /* PICKUP_H */
