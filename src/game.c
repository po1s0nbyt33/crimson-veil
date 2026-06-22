#include "game.h"
#include <string.h>
#include <stdio.h>

/* =============================================================
 *  Ciclo de vida da partida
 * ============================================================= */
void game_set_state(Game *g, GameState s) {
    g->state = s;
    g->state_time = 0.0f;
}

void game_world_clear_transient(Game *g) {
    memset(g->enemies, 0, sizeof(g->enemies));
    memset(g->projectiles, 0, sizeof(g->projectiles));
    memset(g->eprojectiles, 0, sizeof(g->eprojectiles));
    memset(g->pickups, 0, sizeof(g->pickups));
    g->boss_idx = -1;
}

void game_init(Game *g, uint32_t seed) {
    memset(g, 0, sizeof(*g));
    rng_seed(&g->rng, seed);
    save_load(&g->save);
    g->show_fps = g->save.opt_show_fps ? true : false;
    g->audio_on = true;
    g->boss_idx = -1;
    g->sel_main = 0;
    game_set_state(g, GS_MENU);
}

void game_new_run(Game *g) {
    game_world_clear_transient(g);
    fx_reset(g);
    player_init(g);
    wave_init(&g->wave);
    g->run_coins = 0;
    g->boss_idx = -1;
    Vec2 center = v2(SCREEN_W * 0.5f, SCREEN_H * 0.5f);
    g->camera = v2sub(g->player.pos, center);
    game_set_state(g, GS_PLAYING);
}

void game_continue_run(Game *g) {
    game_world_clear_transient(g);
    fx_reset(g);
    g->player    = g->save.run.player;
    g->wave      = g->save.run.wave;
    g->run_coins = g->save.run.run_coins;
    g->camera    = v2(g->save.run.cam_x, g->save.run.cam_y);
    g->boss_idx  = -1;
    game_set_state(g, GS_PLAYING);
}

void game_save_run(Game *g) {
    g->save.run.version  = 1;
    g->save.run.player   = g->player;
    g->save.run.wave     = g->wave;
    g->save.run.run_coins = g->run_coins;
    g->save.run.cam_x    = g->camera.x;
    g->save.run.cam_y    = g->camera.y;
    g->save.has_run      = 1;
    save_write(&g->save);
}

void game_end_run(Game *g) {
    uint32_t bonus = (uint32_t)(g->wave.time / 4.0f);
    g->run_gained = g->run_coins + bonus;
    g->save.coins += g->run_gained;
    g->save.total_kills += (uint32_t)g->wave.kills;
    g->save.lifetime_kills += (uint32_t)g->wave.kills;
    g->save.runs++;
    if ((uint32_t)g->wave.time > g->save.best_time)
        g->save.best_time = (uint32_t)g->wave.time;
    g->save.has_run = 0;
    save_write(&g->save);
    g->sel_gameover = 0;
    game_set_state(g, GS_GAMEOVER);
}

void game_open_levelup(Game *g) {
    g->choice_count = upgrade_roll(g, g->choices, 3);
    g->choice_sel = 0;
    game_set_state(g, GS_LEVELUP);
}

/* Desbloqueios concedidos durante a partida (persistem no save). */
static void check_unlocks(Game *g) {
    if (!(g->save.weapon_unlocks & (1u << WEAPON_AXE)) && g->wave.time >= 180.0f) {
        g->save.weapon_unlocks |= (1u << WEAPON_AXE);
        save_write(&g->save);
        fx_floater(g, v2(g->player.pos.x, g->player.pos.y - 72.0f),
                   "Machado liberado!", COL_ORANGE, 1.0f);
    }
    if (!(g->save.weapon_unlocks & (1u << WEAPON_AURA)) && g->player.level >= 12) {
        g->save.weapon_unlocks |= (1u << WEAPON_AURA);
        save_write(&g->save);
        fx_floater(g, v2(g->player.pos.x, g->player.pos.y - 72.0f),
                   "Aura liberada!", COL_GREEN, 1.0f);
    }
}

/* =============================================================
 *  Atualizacao
 * ============================================================= */
void game_update(Game *g, float dt) {
    g->global_time += dt;
    g->state_time += dt;

    /* trilha conforme o contexto (a thread de audio so troca se mudar) */
    bool inrun = (g->state == GS_PLAYING || g->state == GS_LEVELUP || g->state == GS_PAUSE);
    audio_set_music(inrun ? (g->boss_idx >= 0 ? MUS_BOSS : MUS_GAME) : MUS_MENU);

    if (g->state == GS_PLAYING) {
        player_update(g, dt);
        weapons_update(g, dt);
        projectiles_update(g, dt);
        enemies_update(g, dt);
        eprojectiles_update(g, dt);
        pickups_update(g, dt);
        wave_update(g, dt);
        fx_update(g, dt);

        /* camera segue o jogador (+ tremor) */
        Vec2 center = v2(SCREEN_W * 0.5f, SCREEN_H * 0.5f);
        Vec2 target = v2sub(g->player.pos, center);
        if (g->shake > 0.0f && g->save.opt_shake)
            target = v2add(target, v2scale(rng_dir(&g->rng), g->shake));
        g->camera = target;

        check_unlocks(g);

        if (g->player.hp <= 0.0f) {
            game_end_run(g);
            return;
        }
        if (g->player.pending_levelups > 0) {
            game_open_levelup(g);
            return;
        }
        if (g->input.pressed[BTN_PAUSE]) {
            g->sel_pause = 0;
            game_set_state(g, GS_PAUSE);
        }
    } else {
        menu_update(g, dt);
    }
}

/* =============================================================
 *  Desenho do mundo + HUD
 * ============================================================= */
static void draw_background(Game *g) {
    draw_rect(0, 0, SCREEN_W, SCREEN_H, RGBA8(0x0E, 0x10, 0x18, 0xFF));
    const float gs = 64.0f;
    float ox = -fmodf(g->camera.x, gs); if (ox > 0.0f) ox -= gs;
    float oy = -fmodf(g->camera.y, gs); if (oy > 0.0f) oy -= gs;
    uint32_t lc = RGBA8(0x1A, 0x1E, 0x2C, 0xFF);
    for (float x = ox; x < SCREEN_W; x += gs) draw_rect(x, 0, 1, SCREEN_H, lc);
    for (float y = oy; y < SCREEN_H; y += gs) draw_rect(0, y, SCREEN_W, 1, lc);
}

static void fmt_time(char *buf, int n, float t) {
    int s = (int)t;
    snprintf(buf, n, "%02d:%02d", s / 60, s % 60);
}

static void draw_hud(Game *g) {
    Player *p = &g->player;
    char buf[32];

    /* barra de XP no topo */
    draw_bar(0, 0, SCREEN_W, 6.0f, (float)p->xp / (float)p->xp_to_next,
             RGBA8(0x10, 0x14, 0x20, 0xFF), COL_BLUE);

    /* vida (canto superior esquerdo) */
    draw_bar(12, 14, 168, 16, p->hp / p->max_hp, RGBA8(0, 0, 0, 170), COL_RED);
    draw_rect_outline(12, 14, 168, 16, 1.0f, RGBA8(0xFF, 0xFF, 0xFF, 0x40));
    snprintf(buf, sizeof(buf), "%d / %d", (int)maxf(0.0f, p->hp), (int)p->max_hp);
    draw_text_center(12 + 84, 15, COL_WHITE, 0.6f, buf);

    /* tempo + nivel (centro) */
    fmt_time(buf, sizeof(buf), g->wave.time);
    draw_text_center(SCREEN_W * 0.5f, 10, COL_WHITE, 1.1f, buf);
    snprintf(buf, sizeof(buf), "Nivel %d", p->level);
    draw_text_center(SCREEN_W * 0.5f, 40, COL_YELLOW, 0.7f, buf);

    /* moedas + abates (direita) */
    snprintf(buf, sizeof(buf), "Moedas: %u", (unsigned)g->run_coins);
    draw_text_right(SCREEN_W - 12, 14, COL_YELLOW, 0.7f, buf);
    snprintf(buf, sizeof(buf), "Abates: %d", g->wave.kills);
    draw_text_right(SCREEN_W - 12, 34, COL_DIM, 0.7f, buf);

    if (g->show_fps) {
        snprintf(buf, sizeof(buf), "%d FPS", (int)(g->fps + 0.5f));
        draw_text_right(SCREEN_W - 12, 54, COL_GREEN, 0.6f, buf);
    }

    /* lista de armas (canto inferior esquerdo) */
    int n = p->weapon_count;
    for (int i = 0; i < n; ++i) {
        Weapon *w = &p->weapons[i];
        float y = SCREEN_H - 16.0f - (n - 1 - i) * 18.0f;
        sprite_draw(sprite_for_weapon(w->type), 18.0f, y + 7.0f, 15.0f);
        snprintf(buf, sizeof(buf), "%s  Lv%d", weapon_name(w->type), w->level);
        draw_text(30, y - 2, COL_WHITE, 0.6f, buf);
    }

    /* barra de vida do chefe (centro inferior) */
    if (g->boss_idx >= 0 && g->enemies[g->boss_idx].active && g->enemies[g->boss_idx].is_boss) {
        Enemy *b = &g->enemies[g->boss_idx];
        float bw = 520.0f, bx = (SCREEN_W - bw) * 0.5f, by = SCREEN_H - 28.0f;
        draw_text_center(SCREEN_W * 0.5f, by - 22.0f, COL_RED, 0.8f, boss_name(b->boss_id));
        draw_bar(bx, by, bw, 12.0f, b->hp / b->max_hp, RGBA8(0, 0, 0, 180), COL_RED);
        draw_rect_outline(bx, by, bw, 12.0f, 1.0f, RGBA8(0xFF, 0xFF, 0xFF, 0x50));
    }
}

void game_draw(Game *g) {
    bool world = (g->state == GS_PLAYING || g->state == GS_LEVELUP ||
                  g->state == GS_PAUSE || g->state == GS_GAMEOVER);

    if (world) {
        draw_background(g);
        weapons_draw(g);      /* auras por baixo dos inimigos */
        pickups_draw(g);
        enemies_draw(g);
        player_draw(g);
        projectiles_draw(g);
        eprojectiles_draw(g);
        fx_draw(g);
        draw_hud(g);

        if (g->flash > 0.0f)
            draw_rect(0, 0, SCREEN_W, SCREEN_H, col_a(COL_RED, (unsigned char)(110.0f * g->flash)));
    }

    menu_draw(g);   /* telas/overlays de menu conforme o estado */
}
