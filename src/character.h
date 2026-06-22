#ifndef CHARACTER_H
#define CHARACTER_H

#include "types.h"
#include "weapon.h"

typedef struct Game Game;

/* Personagem jogavel: arma inicial propria + multiplicadores de atributo. */
typedef struct {
    const char *name;
    const char *desc;
    const char *trait;        /* texto curto da habilidade/estilo */
    WeaponType  start_weapon; /* concedida no inicio (independe de desbloqueio) */
    float       hp_mult;
    float       might_mult;
    float       speed_mult;
    float       cooldown_mult; /* <1 = recarga mais rapida */
    float       area_mult;
    int         cost;          /* moedas para desbloquear (0 = ja disponivel) */
    uint32_t    color;
} CharacterDef;

int                 character_count(void);
const CharacterDef *character_get(int id);
bool                character_unlocked(Game *g, int id);
bool                character_buy(Game *g, int id);   /* gasta moedas, marca, salva */
void                character_apply(Game *g, int id); /* arma inicial + multiplicadores */

#endif /* CHARACTER_H */
