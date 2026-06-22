#ifndef WEAPON_H
#define WEAPON_H

#include "types.h"
#include "config.h"

typedef struct Game Game;

/* Armas de ataque automatico (auto-attack, estilo bullet-heaven / horde survival). */
typedef enum {
    WEAPON_BOLT,    /* projetil magico que mira no inimigo mais proximo */
    WEAPON_KNIFE,   /* facas rapidas na direcao do movimento, perfuram pouco */
    WEAPON_AXE,     /* machado em arco (gravidade), dano alto, perfura muito */
    WEAPON_AURA,    /* alho: dano continuo em area ao redor do jogador */
    WEAPON_WHIP,    /* chicote: golpe horizontal na direcao encarada */

    /* ---- armas evoluidas (fusoes) ---- */
    WEAPON_BOLT_EVO,    /* Tempestade Arcana */
    WEAPON_KNIFE_EVO,   /* Mil Laminas */
    WEAPON_AXE_EVO,     /* Machado Lunar */
    WEAPON_AURA_EVO,    /* Coroa Sagrada */
    WEAPON_WHIP_EVO,    /* Ceifa Sombria */

    WEAPON_TYPE_COUNT,
    WEAPON_BASE_COUNT = WEAPON_BOLT_EVO   /* numero de armas base (nao-evoluidas) */
} WeaponType;

#define WEAPON_MAX_LEVEL 8

typedef struct {
    WeaponType type;
    int   level;      /* 1..WEAPON_MAX_LEVEL */
    float cooldown;   /* tempo ate o proximo disparo */
    float tick;       /* uso auxiliar (ex.: cadencia de dano da aura) */
} Weapon;

typedef struct {
    bool     active;
    WeaponType src;
    Vec2     pos, vel;
    float    radius;
    float    damage;
    float    life, maxlife;
    int      pierce;     /* quantos inimigos ainda pode atingir */
    float    grav;       /* gravidade (machado) */
    bool     hits;       /* false = apenas visual (chicote) */
    float    hit_cd;     /* atraso entre acertos (evita derreter 1 alvo) */
    float    spin;       /* rotacao visual */
    Vec2     aux;        /* vetor auxiliar (ex.: alcance/direcao do chicote) */
    uint32_t color;
} Projectile;

const char *weapon_name(WeaponType t);
const char *weapon_desc(WeaponType t);
uint32_t    weapon_color(WeaponType t);
WeaponType  weapon_base(WeaponType t);   /* arma base de uma evolucao (ou ela mesma) */
bool        weapon_is_evo(WeaponType t); /* true se for arma evoluida (fusao) */

void weapons_update(Game *g, float dt);
void weapons_draw(Game *g);          /* desenha auras (areas continuas) */
void projectiles_update(Game *g, float dt);
void projectiles_draw(Game *g);

int  weapon_index(Game *g, WeaponType t);   /* indice no inventario ou -1 */
bool weapon_grant(Game *g, WeaponType t);   /* adiciona, ou sobe nivel se ja tem */
bool weapon_is_maxed(Game *g, WeaponType t);
bool weapon_unlocked(Game *g, WeaponType t); /* disponivel para escolha/equipar */

#endif /* WEAPON_H */
