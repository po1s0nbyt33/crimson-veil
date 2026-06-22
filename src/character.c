#include "game.h"

/* name, desc, trait, start_weapon, hp, might, speed, cooldown, area, cost, color */
static const CharacterDef CHARS[] = {
    { "Cacador", "Agil e versatil. Comeca com Facas.", "+15% de velocidade",
      WEAPON_KNIFE, 1.00f, 1.00f, 1.15f, 1.00f, 1.00f,   0, RGBA8(0x5C, 0xD6, 0x6B, 0xFF) },
    { "Maga", "Dano magico alto, porem fragil.", "+30% dano / -20% vida",
      WEAPON_BOLT,  0.80f, 1.30f, 0.98f, 0.95f, 1.00f,   0, RGBA8(0x9C, 0x6C, 0xF0, 0xFF) },
    { "Guerreiro", "Resistente. Comeca com Chicote.", "+45% vida / -10% vel.",
      WEAPON_WHIP,  1.45f, 1.05f, 0.90f, 1.00f, 1.10f, 600, RGBA8(0xE0, 0x9A, 0x4C, 0xFF) },
    { "Paladino", "Comeca com Aura Sagrada.", "+35% area / recarga rapida",
      WEAPON_AURA,  1.20f, 1.00f, 0.95f, 0.82f, 1.35f, 900, RGBA8(0xF2, 0xC8, 0x4B, 0xFF) },
};
#define NCHAR ((int)(sizeof(CHARS) / sizeof(CHARS[0])))

int character_count(void) { return NCHAR; }

const CharacterDef *character_get(int id) {
    if (id < 0 || id >= NCHAR) return &CHARS[0];
    return &CHARS[id];
}

bool character_unlocked(Game *g, int id) {
    if (id < 0 || id >= NCHAR) return false;
    if (CHARS[id].cost == 0) return true;
    return (g->save.char_unlocks & (1u << id)) != 0;
}

bool character_buy(Game *g, int id) {
    if (id < 0 || id >= NCHAR || character_unlocked(g, id)) return false;
    int cost = CHARS[id].cost;
    if (g->save.coins < (uint32_t)cost) return false;
    g->save.coins -= (uint32_t)cost;
    g->save.char_unlocks |= (1u << id);
    save_write(&g->save);
    return true;
}

void character_apply(Game *g, int id) {
    if (id < 0 || id >= NCHAR) id = 0;
    const CharacterDef *c = &CHARS[id];
    Player *p = &g->player;

    p->character        = id;
    p->ch_hp_mult       = c->hp_mult;
    p->ch_might_mult    = c->might_mult;
    p->ch_speed_mult    = c->speed_mult;
    p->ch_cooldown_mult = c->cooldown_mult;
    p->ch_area_mult     = c->area_mult;

    player_recompute(g);        /* recalcula stats com os multiplicadores do personagem */
    p->hp = p->max_hp;
    weapon_grant(g, c->start_weapon);
}
