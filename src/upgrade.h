#ifndef UPGRADE_H
#define UPGRADE_H

#include "types.h"
#include "player.h"
#include "weapon.h"

typedef struct Game Game;

typedef enum {
    UPK_WEAPON_NEW,    /* nova arma */
    UPK_WEAPON_LEVEL,  /* subir nivel de arma existente */
    UPK_PASSIVE,       /* atributo passivo */
    UPK_FUSION,        /* fusao: cria arma evoluida */
    UPK_HEAL,          /* recuperar vida (fallback) */
    UPK_COINS          /* moedas (fallback) */
} UpgradeKind;

typedef struct {
    UpgradeKind kind;
    WeaponType  weapon;
    PassiveType passive;
    int         fusion_idx;   /* indice da fusao (UPK_FUSION) */
    int         cur_level;
    int         next_level;
    char        title[44];
    char        desc[80];
    uint32_t    color;
} UpgradeChoice;

/* Sorteia ate maxn opcoes validas; retorna a quantidade preenchida. */
int  upgrade_roll(Game *g, UpgradeChoice *out, int maxn);
void upgrade_apply(Game *g, const UpgradeChoice *c);

#endif /* UPGRADE_H */
