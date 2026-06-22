#ifndef ENEMY_H
#define ENEMY_H

#include "types.h"
#include "config.h"

typedef struct Game Game;

typedef enum {
    ENEMY_SWARM,    /* basico, lento, fraco — aparece em grande quantidade */
    ENEMY_RUNNER,   /* rapido, pouca vida */
    ENEMY_BRUTE,    /* lento, muita vida, dano de contato alto */
    ENEMY_SHOOTER,  /* mantem distancia e atira projetil reto */
    ENEMY_MAGE,     /* lento, atira magia teleguiada */
    ENEMY_BOSS,     /* chefe (configurado por boss.c) */
    ENEMY_TYPE_COUNT
} EnemyType;

/* Projetil disparado por inimigos/bosses — atinge o JOGADOR. */
typedef struct {
    bool     active;
    Vec2     pos, vel;
    float    radius;
    float    damage;
    float    life;
    bool     homing;       /* true = persegue lentamente o jogador (magia) */
    bool     destructible; /* true = pode ser destruido por tiros do jogador (inimigos comuns) */
    float    spin;
    uint32_t color;
} EnemyProjectile;

typedef struct {
    bool     active;
    EnemyType type;
    Vec2     pos, vel;
    float    hp, max_hp;
    float    speed;
    float    radius;
    float    damage;       /* dano de contato */
    int      xp;           /* XP derrubado */
    int      coins;        /* moedas derrubadas (boss/brute) */
    uint32_t color;
    float    hit_flash;
    float    knockback;    /* timer de recuo apos ser atingido */

    bool     is_boss;
    int      boss_id;
    float    attack_cd;    /* timer de habilidade do boss */
    float    phase_t;      /* timer de estado do boss */
    int      phase;
} Enemy;

int  enemy_spawn(Game *g, EnemyType t, Vec2 pos);   /* indice ou -1 */
void enemies_update(Game *g, float dt);
void enemies_draw(Game *g);
void enemy_hurt(Game *g, int idx, float dmg);
void enemy_kill(Game *g, int idx);                  /* derruba drops + fx */
int  enemy_nearest(Game *g, Vec2 from, float maxdist); /* indice ou -1 */
int  enemies_alive(Game *g);

/* Projeteis inimigos */
void eproj_spawn(Game *g, Vec2 pos, Vec2 vel, float dmg, float radius,
                 float life, bool homing, bool destructible, uint32_t color);
void eprojectiles_update(Game *g, float dt);
void eprojectiles_draw(Game *g);

#endif /* ENEMY_H */
