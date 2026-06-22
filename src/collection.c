#include "game.h"

#define LOCK_COLOR RGBA8(0x08, 0x08, 0x0C, 0xFF)

int collection_build(Game *g, CollectionEntry *out, int max) {
    int n = 0;

    /* Armas base */
    for (int t = 0; t < WEAPON_BASE_COUNT && n < max; ++t) {
        CollectionEntry *e = &out[n++];
        e->cat = CC_WEAPON;
        e->id = t;
        e->unlocked = weapon_unlocked(g, (WeaponType)t);
        if (e->unlocked) {
            e->name = weapon_name((WeaponType)t);
            e->desc = weapon_desc((WeaponType)t);
            e->color = weapon_color((WeaponType)t);
        } else { e->name = "???"; e->desc = ""; e->color = LOCK_COLOR; }
    }

    /* Fusoes / evolucoes */
    for (int i = 0; i < fusion_count() && n < max; ++i) {
        const FusionDef *f = fusion_get(i);
        CollectionEntry *e = &out[n++];
        e->cat = CC_FUSION;
        e->id = i;
        e->unlocked = (g->save.fusion_unlocks & (1u << i)) != 0;
        if (e->unlocked) {
            e->name = weapon_name(f->result);
            e->desc = weapon_desc(f->result);
            e->color = weapon_color(f->result);
        } else { e->name = "???"; e->desc = ""; e->color = LOCK_COLOR; }
    }

    /* Itens passivos */
    for (int p = 0; p < PASS_COUNT && n < max; ++p) {
        CollectionEntry *e = &out[n++];
        e->cat = CC_PASSIVE;
        e->id = p;
        e->unlocked = (g->save.passive_seen & (1u << p)) != 0;
        if (e->unlocked) {
            e->name = passive_name((PassiveType)p);
            e->desc = passive_desc((PassiveType)p);
            e->color = RGBA8(0xC8, 0xC8, 0xD8, 0xFF);
        } else { e->name = "???"; e->desc = ""; e->color = LOCK_COLOR; }
    }

    /* Personagens */
    for (int c = 0; c < character_count() && n < max; ++c) {
        const CharacterDef *cd = character_get(c);
        CollectionEntry *e = &out[n++];
        e->cat = CC_CHARACTER;
        e->id = c;
        e->unlocked = character_unlocked(g, c);
        if (e->unlocked) {
            e->name = cd->name;
            e->desc = cd->trait;
            e->color = cd->color;
        } else { e->name = "???"; e->desc = ""; e->color = LOCK_COLOR; }
    }

    return n;
}
