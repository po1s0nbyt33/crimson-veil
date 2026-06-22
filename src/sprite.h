#ifndef SPRITE_H
#define SPRITE_H

#include "types.h"
#include "weapon.h"
#include <vita2d.h>

/* Sprites pixel-art gerados proceduralmente em textura no init.
 * Tudo embutido no codigo (sem arquivos externos): baixo uso de memoria,
 * carregamento instantaneo, filtro POINT para pixels nitidos. */
typedef enum {
    SPR_CHAR0, SPR_CHAR1, SPR_CHAR2, SPR_CHAR3,
    SPR_E_SWARM, SPR_E_RUNNER, SPR_E_BRUTE, SPR_E_SHOOTER, SPR_E_MAGE,
    SPR_BOSS0, SPR_BOSS1, SPR_BOSS2, SPR_BOSS3, SPR_BOSS4,
    SPR_WEAPON_FIRST,
    SPR_COUNT = SPR_WEAPON_FIRST + WEAPON_TYPE_COUNT
} SpriteId;

void sprite_init(void);   /* gera todas as texturas (chamar depois de vita2d_init) */
void sprite_fini(void);   /* libera texturas (chamar antes de vita2d_fini) */

void sprite_draw(SpriteId id, float cx, float cy, float target_px);

SpriteId sprite_for_char(int char_id);
SpriteId sprite_for_enemy(int enemy_type);
SpriteId sprite_for_boss(int boss_id);
SpriteId sprite_for_weapon(WeaponType t);

#endif /* SPRITE_H */
