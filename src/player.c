#include "game.h"
#include <string.h>
#include <stdio.h>

const char *passive_name(PassiveType p) {
    switch (p) {
        case PASS_MAXHP:     return "Vitalidade";
        case PASS_REGEN:     return "Regeneracao";
        case PASS_MIGHT:     return "Forca";
        case PASS_AREA:      return "Area";
        case PASS_COOLDOWN:  return "Recarga";
        case PASS_SPEED:     return "Velocidade";
        case PASS_PICKUP:    return "Magnetismo";
        case PASS_PROJSPEED: return "Impulso";
        case PASS_AMOUNT:    return "Multiplicador";
        case PASS_ARMOR:     return "Armadura";
        case PASS_GROWTH:    return "Sabedoria";
        default:             return "?";
    }
}

const char *passive_desc(PassiveType p) {
    switch (p) {
        case PASS_MAXHP:     return "+20 de vida maxima";
        case PASS_REGEN:     return "+0.5 vida por segundo";
        case PASS_MIGHT:     return "+12% de dano";
        case PASS_AREA:      return "+12% de area de efeito";
        case PASS_COOLDOWN:  return "-6% de recarga";
        case PASS_SPEED:     return "+7% de velocidade";
        case PASS_PICKUP:    return "+15% de raio de coleta";
        case PASS_PROJSPEED: return "+12% vel. de projetil";
        case PASS_AMOUNT:    return "+1 projetil por arma";
        case PASS_ARMOR:     return "+1 de armadura";
        case PASS_GROWTH:    return "+8% de XP ganho";
        default:             return "";
    }
}

void player_recompute(Game *g) {
    Player *p = &g->player;
    SaveData *s = &g->save;

    /* multiplicadores do personagem (defensivo: 0 -> 1.0) */
    float chhp = p->ch_hp_mult       > 0.0f ? p->ch_hp_mult       : 1.0f;
    float chmi = p->ch_might_mult    > 0.0f ? p->ch_might_mult    : 1.0f;
    float chsp = p->ch_speed_mult    > 0.0f ? p->ch_speed_mult    : 1.0f;
    float chcd = p->ch_cooldown_mult > 0.0f ? p->ch_cooldown_mult : 1.0f;
    float char_a = p->ch_area_mult   > 0.0f ? p->ch_area_mult     : 1.0f;

    float new_max = (PLAYER_BASE_HP
                  + s->meta[META_MAXHP] * 20.0f
                  + p->passive_lv[PASS_MAXHP] * 20.0f) * chhp;
    float delta = new_max - p->max_hp;
    p->max_hp = new_max;
    if (delta > 0.0f) p->hp = minf(p->max_hp, p->hp + delta);
    if (p->hp > p->max_hp) p->hp = p->max_hp;

    p->might         = (1.0f + s->meta[META_MIGHT] * 0.10f + p->passive_lv[PASS_MIGHT] * 0.12f) * chmi;
    p->area          = (1.0f + p->passive_lv[PASS_AREA] * 0.12f) * char_a;
    p->cooldown_mult = clampf((1.0f - s->meta[META_COOLDOWN] * 0.04f - p->passive_lv[PASS_COOLDOWN] * 0.06f) * chcd, 0.35f, 1.0f);
    p->speed         = PLAYER_BASE_SPEED * (1.0f + s->meta[META_SPEED] * 0.05f + p->passive_lv[PASS_SPEED] * 0.07f) * chsp;
    p->regen         = s->meta[META_REGEN] * 0.40f + p->passive_lv[PASS_REGEN] * 0.50f;
    p->pickup_radius = PICKUP_BASE_RADIUS * (1.0f + s->meta[META_PICKUP] * 0.12f + p->passive_lv[PASS_PICKUP] * 0.15f);
    p->proj_speed    = 1.0f + p->passive_lv[PASS_PROJSPEED] * 0.12f;
    p->amount_bonus  = s->meta[META_AMOUNT] + p->passive_lv[PASS_AMOUNT];
    p->armor         = s->meta[META_ARMOR] * 1.0f + p->passive_lv[PASS_ARMOR] * 1.0f;
    p->growth        = 1.0f + s->meta[META_GROWTH] * 0.05f + p->passive_lv[PASS_GROWTH] * 0.08f;
    p->greed         = 1.0f + s->meta[META_GREED] * 0.06f;
}

void player_init(Game *g) {
    Player *p = &g->player;
    memset(p, 0, sizeof(*p));
    p->pos = v2(0.0f, 0.0f);
    p->facing = v2(0.0f, 1.0f);
    p->level = 1;
    p->xp = 0;
    p->xp_to_next = XP_BASE + 1 * XP_PER_LEVEL;
    p->ch_hp_mult = p->ch_might_mult = p->ch_speed_mult = p->ch_cooldown_mult = p->ch_area_mult = 1.0f;
    p->max_hp = 0.0f;
    p->hp = 0.0f;

    /* aplica personagem: define multiplicadores, recalcula stats e da a arma inicial */
    character_apply(g, g->save.selected_char);
}

void player_heal(Player *p, float amount) {
    p->hp = minf(p->max_hp, p->hp + amount);
}

void player_take_damage(Game *g, float dmg) {
    Player *p = &g->player;
    if (p->iframe > 0.0f) return;

    float d = maxf(1.0f, dmg - p->armor);
    p->hp -= d;
    p->iframe = PLAYER_IFRAME_TIME;
    p->hit_flash = 0.35f;
    g->flash = maxf(g->flash, 0.22f);
    fx_shake(g, 6.0f);
    audio_play_sfx(SFX_HURT);

    if (g->save.opt_damage_numbers) {
        char buf[16];
        snprintf(buf, sizeof(buf), "-%d", (int)d);
        fx_floater(g, v2(p->pos.x, p->pos.y - PLAYER_RADIUS - 4.0f), buf, COL_RED, 0.7f);
    }
}

void player_gain_xp(Game *g, int amount) {
    Player *p = &g->player;
    int gain = (int)(amount * p->growth + 0.5f);
    if (gain < 1) gain = 1;
    p->xp += gain;
    while (p->xp >= p->xp_to_next) {
        p->xp -= p->xp_to_next;
        p->level++;
        p->xp_to_next = XP_BASE + p->level * XP_PER_LEVEL;
        p->pending_levelups++;
        audio_play_sfx(SFX_LEVELUP);
    }
}

void player_update(Game *g, float dt) {
    Player *p = &g->player;
    Vec2 mv = g->input.move;

    p->vel = v2scale(mv, p->speed);
    p->pos = v2add(p->pos, v2scale(p->vel, dt));

    if (v2len2(mv) > 0.04f) {
        p->facing = v2norm(mv);
        p->anim += dt * 11.0f;
    }

    if (p->iframe > 0.0f)    p->iframe -= dt;
    if (p->hit_flash > 0.0f) p->hit_flash -= dt;

    if (p->regen > 0.0f && p->hp < p->max_hp)
        p->hp = minf(p->max_hp, p->hp + p->regen * dt);
}

void player_draw(Game *g) {
    Player *p = &g->player;
    Vec2 s = v2sub(p->pos, g->camera);

    /* pisca durante a invulnerabilidade */
    bool blink = (p->iframe > 0.0f) && ((int)(g->global_time * 18.0f) % 2 == 0);

    /* raio de coleta (sutil) */
    draw_ring(s.x, s.y, p->pickup_radius, 1.0f, RGBA8(0x4D, 0x9D, 0xE8, 0x22));

    /* sombra */
    draw_circle(s.x, s.y + PLAYER_RADIUS * 0.85f, PLAYER_RADIUS * 0.9f, RGBA8(0, 0, 0, 80));

    if (blink) return;

    float bob = sinf(p->anim) * 2.0f;
    sprite_draw(sprite_for_char(p->character), s.x, s.y + bob, PLAYER_RADIUS * 2.9f);

    /* feedback de dano: clarao branco por cima */
    if (p->hit_flash > 0.0f)
        draw_circle(s.x, s.y + bob, PLAYER_RADIUS,
                    col_a(COL_WHITE, (unsigned char)(170.0f * clampf(p->hit_flash / 0.35f, 0.0f, 1.0f))));

    /* indicador de direcao encarada */
    Vec2 e = v2add(v2(s.x, s.y + bob), v2scale(p->facing, PLAYER_RADIUS * 0.75f));
    draw_circle(e.x, e.y, 2.6f, RGBA8(0xFF, 0xF2, 0xC8, 0xFF));
}
