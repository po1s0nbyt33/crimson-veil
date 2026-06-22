#include "game.h"
#include <string.h>
#include <stdio.h>

static void mk_weapon_new(UpgradeChoice *c, WeaponType t) {
    memset(c, 0, sizeof(*c));
    c->kind = UPK_WEAPON_NEW;
    c->weapon = t;
    c->cur_level = 0;
    c->next_level = 1;
    snprintf(c->title, sizeof(c->title), "%s", weapon_name(t));
    snprintf(c->desc, sizeof(c->desc), "NOVA - %s", weapon_desc(t));
    c->color = weapon_color(t);
}

static void mk_weapon_lvl(Game *g, UpgradeChoice *c, WeaponType t) {
    int idx = weapon_index(g, t);
    int lv = g->player.weapons[idx].level;
    memset(c, 0, sizeof(*c));
    c->kind = UPK_WEAPON_LEVEL;
    c->weapon = t;
    c->cur_level = lv;
    c->next_level = lv + 1;
    snprintf(c->title, sizeof(c->title), "%s  Niv.%d", weapon_name(t), lv + 1);
    snprintf(c->desc, sizeof(c->desc), "%s", weapon_desc(t));
    c->color = weapon_color(t);
}

static void mk_passive(Game *g, UpgradeChoice *c, PassiveType p) {
    int lv = g->player.passive_lv[p];
    memset(c, 0, sizeof(*c));
    c->kind = UPK_PASSIVE;
    c->passive = p;
    c->cur_level = lv;
    c->next_level = lv + 1;
    snprintf(c->title, sizeof(c->title), "%s  Niv.%d", passive_name(p), lv + 1);
    snprintf(c->desc, sizeof(c->desc), "%s", passive_desc(p));
    c->color = RGBA8(0xC8, 0xC8, 0xD8, 0xFF);
}

static void mk_fusion(UpgradeChoice *c, int fi) {
    const FusionDef *f = fusion_get(fi);
    memset(c, 0, sizeof(*c));
    c->kind = UPK_FUSION;
    c->weapon = f->result;
    c->fusion_idx = fi;
    c->next_level = 1;
    snprintf(c->title, sizeof(c->title), "FUSAO: %s", weapon_name(f->result));
    snprintf(c->desc, sizeof(c->desc), "%s", weapon_desc(f->result));
    c->color = weapon_color(f->result);
}

int upgrade_roll(Game *g, UpgradeChoice *out, int maxn) {
    UpgradeChoice pool[WEAPON_TYPE_COUNT + PASS_COUNT];
    int n = 0;

    for (int t = 0; t < WEAPON_TYPE_COUNT; ++t) {
        int idx = weapon_index(g, (WeaponType)t);
        if (idx >= 0) {
            if (g->player.weapons[idx].level < WEAPON_MAX_LEVEL)
                mk_weapon_lvl(g, &pool[n++], (WeaponType)t);   /* sobe nivel (inclui evos ja obtidas) */
        } else if (t < WEAPON_BASE_COUNT &&
                   g->player.weapon_count < MAX_WEAPONS &&
                   weapon_unlocked(g, (WeaponType)t)) {
            mk_weapon_new(&pool[n++], (WeaponType)t);          /* nova arma base */
        }
    }
    for (int p = 0; p < PASS_COUNT; ++p) {
        if (g->player.passive_lv[p] < PASSIVE_MAX_LEVEL)
            mk_passive(g, &pool[n++], (PassiveType)p);
    }

    /* embaralha (Fisher-Yates) */
    for (int i = n - 1; i > 0; --i) {
        int j = rng_int(&g->rng, 0, i);
        UpgradeChoice tmp = pool[i];
        pool[i] = pool[j];
        pool[j] = tmp;
    }

    int count = 0;
    for (int i = 0; i < n && count < maxn; ++i)
        out[count++] = pool[i];

    /* preenche com curativo/moedas se o pool acabou (fim de jogo) */
    while (count < maxn) {
        UpgradeChoice *c = &out[count];
        memset(c, 0, sizeof(*c));
        if ((count & 1) == 0) {
            c->kind = UPK_HEAL;
            c->color = COL_GREEN;
            snprintf(c->title, sizeof(c->title), "Cura");
            snprintf(c->desc, sizeof(c->desc), "Recupera 50 de vida");
        } else {
            c->kind = UPK_COINS;
            c->color = COL_YELLOW;
            snprintf(c->title, sizeof(c->title), "Moedas");
            snprintf(c->desc, sizeof(c->desc), "+25 moedas");
        }
        count++;
    }

    /* fusao: se houver uma pronta, ha chance de ela substituir o 1o slot */
    int fi = fusion_first_ready(g);
    if (fi >= 0 && rng_f01(&g->rng) < 0.70f)
        mk_fusion(&out[0], fi);

    return count;
}

void upgrade_apply(Game *g, const UpgradeChoice *c) {
    switch (c->kind) {
        case UPK_WEAPON_NEW:
        case UPK_WEAPON_LEVEL:
            weapon_grant(g, c->weapon);
            break;
        case UPK_FUSION:
            weapon_grant(g, c->weapon);
            g->save.fusion_unlocks |= (1u << c->fusion_idx);
            save_write(&g->save);
            fx_floater(g, g->player.pos, "FUSAO!", weapon_color(c->weapon), 1.4f);
            g->flash = maxf(g->flash, 0.3f);
            audio_play_sfx(SFX_FUSION);
            break;
        case UPK_PASSIVE:
            if (g->player.passive_lv[c->passive] < PASSIVE_MAX_LEVEL)
                g->player.passive_lv[c->passive]++;
            g->save.passive_seen |= (1u << c->passive);   /* registra na colecao */
            save_write(&g->save);
            player_recompute(g);
            break;
        case UPK_HEAL:
            player_heal(&g->player, 50.0f);
            break;
        case UPK_COINS:
            g->run_coins += 25;
            break;
        default: break;
    }
}
