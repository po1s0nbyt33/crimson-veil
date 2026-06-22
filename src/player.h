#ifndef PLAYER_H
#define PLAYER_H

#include "types.h"
#include "config.h"
#include "weapon.h"

typedef struct Game Game;

/* Atributos passivos (melhorados via upgrades de nivel). */
typedef enum {
    PASS_MAXHP,
    PASS_REGEN,
    PASS_MIGHT,      /* dano */
    PASS_AREA,       /* tamanho de area/projetil */
    PASS_COOLDOWN,   /* recarga das armas */
    PASS_SPEED,      /* velocidade de movimento */
    PASS_PICKUP,     /* raio de coleta */
    PASS_PROJSPEED,  /* velocidade dos projeteis */
    PASS_AMOUNT,     /* projeteis extras */
    PASS_ARMOR,      /* reducao de dano */
    PASS_GROWTH,     /* ganho de XP */
    PASS_COUNT
} PassiveType;

#define PASSIVE_MAX_LEVEL 5

typedef struct {
    Vec2  pos, vel;
    Vec2  facing;          /* ultima direcao encarada (mira de facas/chicote) */
    float hp, max_hp;
    float speed;
    float regen;
    float pickup_radius;
    float iframe;          /* invulnerabilidade restante */

    int   level, xp, xp_to_next;

    /* multiplicadores derivados (base + meta + passivas) */
    float might;
    float area;
    float cooldown_mult;
    float proj_speed;
    int   amount_bonus;
    float armor;
    float growth;
    float greed;

    Weapon  weapons[MAX_WEAPONS];
    int     weapon_count;
    uint8_t passive_lv[PASS_COUNT];

    /* personagem escolhido + seus multiplicadores fixos */
    int   character;
    float ch_hp_mult, ch_might_mult, ch_speed_mult, ch_cooldown_mult, ch_area_mult;

    int   pending_levelups;  /* nivels acumulados aguardando tela de upgrade */
    float hit_flash;
    float anim;
} Player;

const char *passive_name(PassiveType p);
const char *passive_desc(PassiveType p);

void player_init(Game *g);               /* zera e aplica bonus de meta */
void player_recompute(Game *g);          /* recalcula stats derivados */
void player_update(Game *g, float dt);
void player_take_damage(Game *g, float dmg);
void player_gain_xp(Game *g, int amount);
void player_heal(Player *p, float amount);
void player_draw(Game *g);

#endif /* PLAYER_H */
