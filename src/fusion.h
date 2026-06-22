#ifndef FUSION_H
#define FUSION_H

#include "types.h"
#include "weapon.h"
#include "player.h"

typedef struct Game Game;

/* Receita de fusao: arma base (nivel alto) + passiva (nivel alto) -> arma evoluida. */
typedef struct {
    WeaponType  result;         /* arma evoluida resultante */
    WeaponType  base_weapon;    /* arma base necessaria */
    PassiveType req_passive;    /* passiva necessaria */
    int         passive_level;  /* nivel minimo da passiva */
} FusionDef;

#define FUSION_WEAPON_LEVEL 6   /* nivel minimo da arma base para liberar a fusao */

int              fusion_count(void);
const FusionDef *fusion_get(int i);
int              fusion_index_by_result(WeaponType result);  /* ou -1 */
bool             fusion_is_ready(Game *g, int i);   /* prereqs cumpridos e ainda nao obtida */
int              fusion_first_ready(Game *g);       /* indice de uma fusao pronta, ou -1 */

#endif /* FUSION_H */
