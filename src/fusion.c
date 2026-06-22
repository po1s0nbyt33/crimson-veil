#include "game.h"

/* result, base_weapon, req_passive, passive_level */
static const FusionDef FUSIONS[] = {
    { WEAPON_BOLT_EVO,  WEAPON_BOLT,  PASS_MIGHT,    4 },
    { WEAPON_KNIFE_EVO, WEAPON_KNIFE, PASS_AMOUNT,   4 },
    { WEAPON_AXE_EVO,   WEAPON_AXE,   PASS_AREA,     4 },
    { WEAPON_AURA_EVO,  WEAPON_AURA,  PASS_COOLDOWN, 4 },
    { WEAPON_WHIP_EVO,  WEAPON_WHIP,  PASS_SPEED,    4 },
};
#define NFUSE ((int)(sizeof(FUSIONS) / sizeof(FUSIONS[0])))

int fusion_count(void) { return NFUSE; }

const FusionDef *fusion_get(int i) {
    if (i < 0 || i >= NFUSE) return &FUSIONS[0];
    return &FUSIONS[i];
}

int fusion_index_by_result(WeaponType result) {
    for (int i = 0; i < NFUSE; ++i)
        if (FUSIONS[i].result == result) return i;
    return -1;
}

bool fusion_is_ready(Game *g, int i) {
    if (i < 0 || i >= NFUSE) return false;
    const FusionDef *f = &FUSIONS[i];
    if (weapon_index(g, f->result) >= 0) return false;            /* ja possui a evolucao */
    int wi = weapon_index(g, f->base_weapon);
    if (wi < 0 || g->player.weapons[wi].level < FUSION_WEAPON_LEVEL) return false;
    if (g->player.passive_lv[f->req_passive] < f->passive_level) return false;
    return true;
}

int fusion_first_ready(Game *g) {
    for (int i = 0; i < NFUSE; ++i)
        if (fusion_is_ready(g, i)) return i;
    return -1;
}
