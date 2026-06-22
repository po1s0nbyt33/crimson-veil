#include "game.h"
#include <stdio.h>

/* =============================================================
 *  Helpers de desenho de UI
 * ============================================================= */
#define LOCK_COL RGBA8(0x12, 0x12, 0x18, 0xFF)

static CollectionEntry s_col[MAX_COLLECTION];
static int s_col_n = 0;

static void dim_overlay(unsigned char a) {
    draw_rect(0, 0, SCREEN_W, SCREEN_H, RGBA8(0, 0, 0, a));
}

static void fmt_time_local(char *buf, int n, float t) {
    int s = (int)t;
    snprintf(buf, n, "%02d:%02d", s / 60, s % 60);
}

static void menu_bg(Game *g) {
    /* gradiente vertical escuro arroxeado (tema vampirico) */
    for (int y = 0; y < SCREEN_H; y += 8) {
        float f = (float)y / SCREEN_H;
        int r = (int)(0x18 + f * 0x10), gg = (int)(0x09 + f * 0x06), b = (int)(0x1A + f * 0x12);
        draw_rect(0, (float)y, SCREEN_W, 8.0f, RGBA8(r, gg, b, 0xFF));
    }
    /* nevoa translucida */
    for (int i = 0; i < 7; ++i) {
        float t = g->global_time * 0.18f + i * 0.9f;
        float x = SCREEN_W * 0.5f + cosf(t) * (140.0f + i * 42.0f);
        float y = SCREEN_H * 0.5f + sinf(t * 0.7f) * (90.0f + i * 24.0f);
        draw_circle(x, y, 70.0f + i * 12.0f, RGBA8(0x2A, 0x12, 0x30, 0x14));
    }
    /* criaturas vagando ao fundo */
    for (int i = 0; i < 5; ++i) {
        float x = fmodf(i * 210.0f + g->global_time * 22.0f, SCREEN_W + 80.0f) - 40.0f;
        float y = 320.0f + sinf(g->global_time * 0.6f + i) * 26.0f;
        sprite_draw((SpriteId)(SPR_E_SWARM + i), x, y, 22.0f);
    }
}

static void col_icon(CollectionEntry *e, float cx, float cy, float sz) {
    if (!e->unlocked) { draw_rect(cx - sz * 0.5f, cy - sz * 0.5f, sz, sz, LOCK_COL); return; }
    switch (e->cat) {
        case CC_WEAPON:    sprite_draw(sprite_for_weapon((WeaponType)e->id), cx, cy, sz); break;
        case CC_FUSION:    sprite_draw(sprite_for_weapon(fusion_get(e->id)->result), cx, cy, sz); break;
        case CC_CHARACTER: sprite_draw(sprite_for_char(e->id), cx, cy, sz); break;
        default:
            draw_rect(cx - sz * 0.5f, cy - sz * 0.5f, sz, sz, e->color);
            draw_rect(cx - sz * 0.25f, cy - sz * 0.25f, sz * 0.5f, sz * 0.5f, RGBA8(0xFF, 0xFF, 0xFF, 0x40));
            break;
    }
}

static void panel(float x, float y, float w, float h) {
    draw_rect(x, y, w, h, COL_PANEL);
    draw_rect_outline(x, y, w, h, 1.0f, RGBA8(0xFF, 0xFF, 0xFF, 0x22));
}

static void row_highlight(float x, float y, float w, float h, bool sel) {
    if (sel) {
        draw_rect(x, y, w, h, COL_PANEL2);
        draw_rect(x, y, 4.0f, h, COL_ACCENT);
    }
}

static void pips(float x, float y, int level, int maxlv, uint32_t on) {
    for (int i = 0; i < maxlv; ++i) {
        uint32_t c = (i < level) ? on : RGBA8(0x33, 0x36, 0x44, 0xFF);
        draw_rect(x + i * 14.0f, y, 10.0f, 10.0f, c);
    }
}

static void pct_line(float x, float y, const char *label, float mult) {
    char b[40];
    int pc = (int)((mult - 1.0f) * 100.0f + (mult >= 1.0f ? 0.5f : -0.5f));
    snprintf(b, sizeof(b), "%s: %+d%%", label, pc);
    draw_text(x, y, pc >= 0 ? COL_GREEN : COL_RED, 0.68f, b);
}

/* =============================================================
 *  MENU INICIAL
 * ============================================================= */
static void main_update(Game *g, float dt) {
    const int items = 6;
    int d = input_menu_vert(&g->input, dt);
    if (d) g->sel_main = (g->sel_main + d + items) % items;

    if (g->input.pressed[BTN_CONFIRM]) {
        switch (g->sel_main) {
            case 0:
                g->sel_char = (int)g->save.selected_char;
                if (g->sel_char < 0 || g->sel_char >= character_count()) g->sel_char = 0;
                game_set_state(g, GS_CHARSELECT);
                break;
            case 1: if (g->save.has_run) game_continue_run(g); break;
            case 2: g->sel_collection = 0; g->col_scroll = 0; game_set_state(g, GS_COLLECTION); break;
            case 3: g->sel_mission = 0; g->miss_scroll = 0; game_set_state(g, GS_MISSIONS); break;
            case 4: g->sel_settings = 0; g->confirm_reset = false; game_set_state(g, GS_SETTINGS); break;
            case 5: g->should_quit = true; break;
        }
    }
}

static void main_draw(Game *g) {
    menu_bg(g);
    float cx = SCREEN_W * 0.5f;

    /* logo com bosses ladeando + sombra + linha de destaque */
    sprite_draw(SPR_BOSS0, cx - 232.0f, 62.0f, 70.0f);
    sprite_draw(SPR_BOSS3, cx + 232.0f, 62.0f, 70.0f);
    draw_text_center(cx + 3.0f, 30.0f, RGBA8(0, 0, 0, 0xC0), 2.2f, "CRIMSON");
    draw_text_center(cx, 28.0f, COL_ACCENT, 2.2f, "CRIMSON");
    draw_text_center(cx, 74.0f, COL_WHITE, 1.7f, "VEIL");
    draw_rect(cx - 150.0f, 112.0f, 300.0f, 2.0f, COL_ACCENT);

    const char *items[6] = { "Jogar", "Continuar", "Colecao", "Missoes", "Configuracoes", "Sair" };
    uint32_t icols[6] = { COL_GREEN, COL_BLUE, COL_CYAN, COL_YELLOW, COL_DIM, COL_RED };
    float top = 148.0f, sp = 42.0f, w = 300.0f, x = cx - w * 0.5f;
    for (int i = 0; i < 6; ++i) {
        float y = top + i * sp;
        bool sel = (i == g->sel_main);
        bool disabled = (i == 1 && !g->save.has_run);
        draw_rect(x, y - 4.0f, w, 34.0f, sel ? COL_PANEL2 : RGBA8(0x16, 0x16, 0x22, 0xC8));
        if (sel) {
            draw_rect(x, y - 4.0f, 4.0f, 34.0f, COL_ACCENT);
            draw_rect_outline(x, y - 4.0f, w, 34.0f, 1.0f, COL_ACCENT);
        }
        uint32_t ic = disabled ? RGBA8(0x44, 0x44, 0x50, 0xFF) : icols[i];
        draw_rect(x + 12.0f, y + 2.0f, 16.0f, 16.0f, ic);
        draw_rect(x + 15.0f, y + 5.0f, 10.0f, 10.0f, col_a(COL_WHITE, 0x44));
        uint32_t c = disabled ? RGBA8(0x55, 0x57, 0x64, 0xFF) : (sel ? COL_WHITE : COL_DIM);
        draw_text(x + 42.0f, y, c, 0.9f, items[i]);
    }

    char b[48];
    snprintf(b, sizeof(b), "Moedas: %u", (unsigned)g->save.coins);
    draw_text_center(cx, 430.0f, COL_YELLOW, 0.8f, b);
    if (g->save.best_time > 0) {
        snprintf(b, sizeof(b), "Melhor tempo: %02u:%02u", g->save.best_time / 60, g->save.best_time % 60);
        draw_text_center(cx, 456.0f, COL_DIM, 0.66f, b);
    }
    draw_text_right(SCREEN_W - 8.0f, SCREEN_H - 22.0f, RGBA8(0x44, 0x46, 0x52, 0xFF), 0.6f, "v2.1");
}

/* =============================================================
 *  SELECAO / LOJA DE PERSONAGENS
 * ============================================================= */
static void charselect_update(Game *g, float dt) {
    int n = character_count();
    int dv = input_menu_vert(&g->input, dt);
    if (dv) g->sel_char = (g->sel_char + dv + n) % n;

    if (g->input.pressed[BTN_CANCEL]) { game_set_state(g, GS_MENU); return; }

    if (g->input.pressed[BTN_CONFIRM]) {
        if (character_unlocked(g, g->sel_char)) {
            g->save.selected_char = (uint8_t)g->sel_char;
            save_write(&g->save);
            g->sel_shop = 0;
            game_set_state(g, GS_SHOP);
        } else {
            character_buy(g, g->sel_char);   /* compra se houver moedas */
        }
    }
}

static void charselect_draw(Game *g) {
    menu_bg(g);
    float cx = SCREEN_W * 0.5f;
    draw_text_center(cx, 16.0f, COL_WHITE, 1.2f, "ESCOLHA O PERSONAGEM");
    char b[48];
    snprintf(b, sizeof(b), "Moedas: %u", (unsigned)g->save.coins);
    draw_text_right(SCREEN_W - 16.0f, 20.0f, COL_YELLOW, 0.8f, b);

    int n = character_count();
    float lx = 48.0f, ly = 78.0f, lw = 300.0f, rowh = 56.0f;
    for (int i = 0; i < n; ++i) {
        float y = ly + i * rowh;
        bool sel = (i == g->sel_char);
        const CharacterDef *c = character_get(i);
        bool unl = character_unlocked(g, i);
        row_highlight(lx, y - 4.0f, lw, 48.0f, sel);
        if (unl) sprite_draw(sprite_for_char(i), lx + 26.0f, y + 18.0f, 32.0f);
        else     draw_rect(lx + 12.0f, y + 2.0f, 28.0f, 28.0f, LOCK_COL);
        draw_text(lx + 52.0f, y + 2.0f, sel ? COL_WHITE : COL_DIM, 0.82f, unl ? c->name : "???");
        if (!unl) {
            snprintf(b, sizeof(b), "%d moedas", c->cost);
            draw_text(lx + 52.0f, y + 24.0f, COL_YELLOW, 0.6f, b);
        }
    }

    /* painel de detalhes */
    const CharacterDef *c = character_get(g->sel_char);
    bool unl = character_unlocked(g, g->sel_char);
    float rx = 392.0f, ry = 78.0f, rw = 520.0f, rh = 392.0f;
    panel(rx, ry, rw, rh);
    if (unl) sprite_draw(sprite_for_char(g->sel_char), rx + 56.0f, ry + 56.0f, 68.0f);
    else     draw_rect(rx + 26.0f, ry + 26.0f, 60.0f, 60.0f, LOCK_COL);
    draw_text(rx + 104.0f, ry + 24.0f, COL_WHITE, 1.1f, unl ? c->name : "???");

    if (unl) {
        draw_text(rx + 104.0f, ry + 56.0f, COL_CYAN, 0.7f, c->trait);
        draw_text(rx + 24.0f, ry + 110.0f, COL_DIM, 0.72f, c->desc);
        snprintf(b, sizeof(b), "Arma inicial: %s", weapon_name(c->start_weapon));
        draw_text(rx + 24.0f, ry + 146.0f, weapon_color(c->start_weapon), 0.74f, b);
        pct_line(rx + 24.0f, ry + 188.0f, "Vida",       c->hp_mult);
        pct_line(rx + 24.0f, ry + 214.0f, "Dano",       c->might_mult);
        pct_line(rx + 24.0f, ry + 240.0f, "Velocidade", c->speed_mult);
        pct_line(rx + 24.0f, ry + 266.0f, "Area",       c->area_mult);
        draw_text_center(rx + rw * 0.5f, ry + rh - 44.0f, COL_GREEN, 1.0f, "X: JOGAR");
    } else {
        draw_text(rx + 104.0f, ry + 56.0f, COL_RED, 0.75f, "BLOQUEADO");
        draw_text(rx + 24.0f, ry + 150.0f, COL_DIM, 0.72f, "Compre para liberar este heroi.");
        snprintf(b, sizeof(b), "Custa %d moedas", c->cost);
        uint32_t cc = (g->save.coins >= (uint32_t)c->cost) ? COL_GREEN : COL_RED;
        draw_text_center(rx + rw * 0.5f, ry + rh - 78.0f, cc, 0.9f, b);
        draw_text_center(rx + rw * 0.5f, ry + rh - 44.0f, cc, 1.0f, "X: COMPRAR");
    }

    draw_text_center(cx, SCREEN_H - 22.0f, RGBA8(0x66,0x68,0x76,0xFF), 0.6f,
                     "Cima/Baixo: escolher    X: confirmar    O: voltar");
}

/* =============================================================
 *  LOJA (melhorias permanentes) — apos escolher personagem
 * ============================================================= */
static void shop_start_run(Game *g) {
    g->save.has_run = 0;
    game_new_run(g);
}

static void shop_update(Game *g, float dt) {
    const int items = META_COUNT + 1;   /* +1 = INICIAR */
    int dv = input_menu_vert(&g->input, dt);
    if (dv) g->sel_shop = (g->sel_shop + dv + items) % items;

    if (g->input.pressed[BTN_CANCEL])   { game_set_state(g, GS_CHARSELECT); return; }
    if (g->input.pressed[BTN_TRIANGLE]) { shop_start_run(g); return; }

    if (g->input.pressed[BTN_CONFIRM]) {
        if (g->sel_shop < META_COUNT) {
            MetaType m = (MetaType)g->sel_shop;
            int cost = meta_cost(&g->save, m);
            if (cost > 0 && g->save.coins >= (uint32_t)cost) {
                g->save.coins -= (uint32_t)cost;
                g->save.meta[m]++;
                save_write(&g->save);
            }
        } else {
            shop_start_run(g);
        }
    }
}

static void shop_draw(Game *g) {
    menu_bg(g);
    draw_text_center(SCREEN_W * 0.5f, 18.0f, COL_WHITE, 1.2f, "MELHORIAS PERMANENTES");
    char b[48];
    snprintf(b, sizeof(b), "Moedas: %u", (unsigned)g->save.coins);
    draw_text_right(SCREEN_W - 16.0f, 22.0f, COL_YELLOW, 0.85f, b);
    const CharacterDef *pc = character_get(g->save.selected_char);
    snprintf(b, sizeof(b), "Heroi: %s", pc->name);
    draw_text(16.0f, 22.0f, pc->color, 0.8f, b);

    float top = 60.0f, sp = 30.0f, x = 90.0f, w = SCREEN_W - 180.0f;
    for (int i = 0; i < META_COUNT; ++i) {
        float y = top + i * sp;
        bool sel = (i == g->sel_shop);
        row_highlight(x, y - 2.0f, w, sp - 2.0f, sel);
        draw_text(x + 12.0f, y, sel ? COL_WHITE : COL_DIM, 0.72f, meta_name((MetaType)i));
        pips(x + 230.0f, y + 3.0f, g->save.meta[i], META_MAX_LEVEL, COL_GREEN);
        int cost = meta_cost(&g->save, (MetaType)i);
        if (cost == 0) snprintf(b, sizeof(b), "MAX");
        else           snprintf(b, sizeof(b), "%d moedas", cost);
        uint32_t cc = (cost == 0) ? COL_GREEN : (g->save.coins >= (uint32_t)cost ? COL_YELLOW : COL_RED);
        draw_text_right(x + w - 12.0f, y, cc, 0.72f, b);
        if (sel) draw_text(x + 12.0f, y + 14.0f, RGBA8(0x88,0x8A,0x98,0xFF), 0.55f, meta_desc((MetaType)i));
    }

    float ys = top + META_COUNT * sp;
    bool sels = (g->sel_shop == META_COUNT);
    row_highlight(x, ys - 2.0f, w, sp - 2.0f, sels);
    draw_text_center(SCREEN_W * 0.5f, ys, sels ? COL_GREEN : COL_DIM, 0.85f, "INICIAR PARTIDA");

    draw_text_center(SCREEN_W * 0.5f, SCREEN_H - 22.0f, RGBA8(0x66,0x68,0x76,0xFF), 0.6f,
                     "X: comprar/iniciar   /\\: iniciar rapido   O: voltar");
}

/* =============================================================
 *  COLECAO
 * ============================================================= */
static const char *col_cat_name(ColCat c) {
    switch (c) {
        case CC_WEAPON:    return "Arma";
        case CC_FUSION:    return "Fusao";
        case CC_PASSIVE:   return "Passiva";
        case CC_CHARACTER: return "Heroi";
        default:           return "";
    }
}

static void collection_update(Game *g, float dt) {
    s_col_n = collection_build(g, s_col, MAX_COLLECTION);
    int n = s_col_n;
    const int cols = 5, vis_rows = 5;
    if (n < 1) { if (g->input.pressed[BTN_CANCEL]) game_set_state(g, GS_MENU); return; }

    int dh = input_menu_horz(&g->input, dt);
    int dv = input_menu_vert(&g->input, dt);
    if (dh) g->sel_collection = clampi(g->sel_collection + dh, 0, n - 1);
    if (dv) g->sel_collection = clampi(g->sel_collection + dv * cols, 0, n - 1);

    int row = g->sel_collection / cols;
    if (row < g->col_scroll) g->col_scroll = row;
    if (row >= g->col_scroll + vis_rows) g->col_scroll = row - vis_rows + 1;

    if (g->input.pressed[BTN_CANCEL]) game_set_state(g, GS_MENU);
}

static void collection_draw(Game *g) {
    menu_bg(g);
    draw_text_center(SCREEN_W * 0.5f, 16.0f, COL_WHITE, 1.2f, "COLECAO");
    char b[64];
    int unlocked = 0;
    for (int i = 0; i < s_col_n; ++i) if (s_col[i].unlocked) unlocked++;
    snprintf(b, sizeof(b), "%d / %d", unlocked, s_col_n);
    draw_text_right(SCREEN_W - 16.0f, 20.0f, COL_DIM, 0.8f, b);

    const int cols = 5, vis_rows = 5;
    float gx = 56.0f, gy = 64.0f, cw = (SCREEN_W - 112.0f) / cols, ch = 78.0f;

    for (int i = 0; i < s_col_n; ++i) {
        int row = i / cols, col = i % cols;
        if (row < g->col_scroll || row >= g->col_scroll + vis_rows) continue;
        float x = gx + col * cw;
        float y = gy + (row - g->col_scroll) * ch;
        bool sel = (i == g->sel_collection);
        CollectionEntry *e = &s_col[i];

        if (sel) draw_rect_outline(x + 4.0f, y, cw - 8.0f, ch - 10.0f, 2.0f, COL_ACCENT);
        float ccx = x + cw * 0.5f, ccy = y + 22.0f;
        col_icon(e, ccx, ccy, 38.0f);
        draw_text_center(ccx, y + 46.0f, e->unlocked ? COL_WHITE : COL_DIM, 0.56f, e->name);
    }

    /* rodape: detalhe do selecionado */
    if (s_col_n > 0) {
        CollectionEntry *e = &s_col[g->sel_collection];
        panel(40.0f, SCREEN_H - 78.0f, SCREEN_W - 80.0f, 50.0f);
        snprintf(b, sizeof(b), "[%s] %s", col_cat_name(e->cat), e->name);
        draw_text(56.0f, SCREEN_H - 72.0f, e->unlocked ? COL_WHITE : COL_DIM, 0.78f, b);
        draw_text(56.0f, SCREEN_H - 48.0f, COL_DIM, 0.66f,
                  e->unlocked ? e->desc : "Item ainda nao descoberto.");
    }
    draw_text_right(SCREEN_W - 16.0f, SCREEN_H - 22.0f, RGBA8(0x66,0x68,0x76,0xFF), 0.6f, "O: voltar");
}

/* =============================================================
 *  MISSOES
 * ============================================================= */
static void missions_update(Game *g, float dt) {
    int n = mission_count();
    const int vis = 5;
    int dv = input_menu_vert(&g->input, dt);
    if (dv) g->sel_mission = clampi(g->sel_mission + dv, 0, n - 1);

    if (g->sel_mission < g->miss_scroll) g->miss_scroll = g->sel_mission;
    if (g->sel_mission >= g->miss_scroll + vis) g->miss_scroll = g->sel_mission - vis + 1;

    if (g->input.pressed[BTN_CANCEL]) { game_set_state(g, GS_MENU); return; }
    if (g->input.pressed[BTN_CONFIRM]) mission_claim(g, g->sel_mission);
}

static void missions_draw(Game *g) {
    menu_bg(g);
    draw_text_center(SCREEN_W * 0.5f, 16.0f, COL_WHITE, 1.2f, "MISSOES");

    int n = mission_count();
    const int vis = 5;
    float x = 48.0f, top = 60.0f, w = SCREEN_W - 96.0f, rh = 86.0f;
    char b[64];

    for (int k = 0; k < vis; ++k) {
        int i = g->miss_scroll + k;
        if (i < 0 || i >= n) break;
        const MissionDef *m = mission_get(i);
        MissionState st = mission_state(g, i);
        float y = top + k * rh;
        bool sel = (i == g->sel_mission);

        panel(x, y, w, rh - 10.0f);
        if (sel) draw_rect(x, y, 5.0f, rh - 10.0f, COL_ACCENT);

        const char *stxt; uint32_t scol;
        switch (st) {
            case MS_LOCKED:    stxt = "BLOQUEADA";  scol = COL_DIM;    break;
            case MS_AVAILABLE: stxt = "EM PROGRESSO"; scol = COL_YELLOW; break;
            case MS_COMPLETE:  stxt = "RESGATAR!";  scol = COL_GREEN;  break;
            default:           stxt = "CONCLUIDA";  scol = COL_BLUE;   break;
        }

        bool show = (st != MS_LOCKED);
        draw_text(x + 14.0f, y + 8.0f, sel ? COL_WHITE : RGBA8(0xCF,0xD0,0xDC,0xFF), 0.82f,
                  show ? m->name : "???");
        draw_text_right(x + w - 14.0f, y + 8.0f, scol, 0.7f, stxt);
        draw_text(x + 14.0f, y + 32.0f, COL_DIM, 0.64f, show ? m->desc : "Conclua a missao anterior.");

        if (show) {
            /* progresso */
            if (m->type == MT_KILLS || m->type == MT_RUNS) {
                snprintf(b, sizeof(b), "%d / %d", mission_progress_value(g, i), m->target);
                draw_text(x + 14.0f, y + 54.0f, COL_CYAN, 0.64f, b);
            } else if (m->type == MT_SURVIVE) {
                snprintf(b, sizeof(b), "%02d:%02d / %02d:%02d",
                         mission_progress_value(g, i) / 60, mission_progress_value(g, i) % 60,
                         m->target / 60, m->target % 60);
                draw_text(x + 14.0f, y + 54.0f, COL_CYAN, 0.64f, b);
            }
            draw_text_right(x + w - 14.0f, y + 54.0f, COL_YELLOW, 0.62f, mission_reward_text(i));
        }
    }

    draw_text_center(SCREEN_W * 0.5f, SCREEN_H - 22.0f, RGBA8(0x66,0x68,0x76,0xFF), 0.6f,
                     "Cima/Baixo: navegar   X: resgatar   O: voltar");
}

/* =============================================================
 *  CONFIGURACOES
 * ============================================================= */
static void settings_update(Game *g, float dt) {
    if (g->confirm_reset) {
        if (g->input.pressed[BTN_CONFIRM]) {
            save_wipe(&g->save);
            g->show_fps = g->save.opt_show_fps ? true : false;
            g->confirm_reset = false;
        } else if (g->input.pressed[BTN_CANCEL]) {
            g->confirm_reset = false;
        }
        return;
    }

    const int items = 6;
    int dv = input_menu_vert(&g->input, dt);
    if (dv) g->sel_settings = (g->sel_settings + dv + items) % items;

    if (g->input.pressed[BTN_CANCEL]) { game_set_state(g, GS_MENU); return; }
    if (g->input.pressed[BTN_CONFIRM]) {
        switch (g->sel_settings) {
            case 0: g->save.opt_shake ^= 1; save_write(&g->save); break;
            case 1: g->save.opt_damage_numbers ^= 1; save_write(&g->save); break;
            case 2: g->save.opt_show_fps ^= 1; g->show_fps = g->save.opt_show_fps ? true : false; save_write(&g->save); break;
            case 3: g->audio_on = !g->audio_on; audio_set_enabled(g->audio_on); break;
            case 4: g->confirm_reset = true; break;
            case 5: game_set_state(g, GS_MENU); break;
        }
    }
}

static void settings_draw(Game *g) {
    menu_bg(g);
    draw_text_center(SCREEN_W * 0.5f, 50.0f, COL_WHITE, 1.4f, "CONFIGURACOES");

    char b[64];
    const char *labels[6] = { "Tremor de tela", "Numeros de dano", "Mostrar FPS",
                              "Audio", "Apagar progresso", "Voltar" };
    int vals[4] = { g->save.opt_shake, g->save.opt_damage_numbers, g->save.opt_show_fps,
                    g->audio_on ? 1 : 0 };
    float top = 132.0f, sp = 44.0f, w = 460.0f, cx = SCREEN_W * 0.5f, x = cx - w * 0.5f;
    for (int i = 0; i < 6; ++i) {
        float y = top + i * sp;
        bool sel = (i == g->sel_settings);
        row_highlight(x, y - 4.0f, w, 34.0f, sel);
        draw_text(x + 14.0f, y, sel ? COL_WHITE : COL_DIM, 0.85f, labels[i]);
        if (i < 4) {
            snprintf(b, sizeof(b), "%s", vals[i] ? "LIGADO" : "DESLIGADO");
            draw_text_right(x + w - 14.0f, y, vals[i] ? COL_GREEN : COL_RED, 0.8f, b);
        }
    }

    if (g->confirm_reset) {
        dim_overlay(180);
        panel(cx - 230.0f, 210.0f, 460.0f, 120.0f);
        draw_text_center(cx, 232.0f, COL_RED, 0.9f, "Apagar TODO o progresso?");
        draw_text_center(cx, 268.0f, COL_DIM, 0.7f, "Moedas, herois, missoes e colecao.");
        draw_text_center(cx, 298.0f, COL_WHITE, 0.75f, "X: confirmar    O: cancelar");
    }
}

/* =============================================================
 *  LEVEL UP
 * ============================================================= */
static void levelup_update(Game *g, float dt) {
    int n = g->choice_count > 0 ? g->choice_count : 1;
    int dv = input_menu_vert(&g->input, dt);
    int dh = input_menu_horz(&g->input, dt);
    int d = dv ? dv : dh;
    if (d) g->choice_sel = (g->choice_sel + d + n) % n;

    if (g->input.pressed[BTN_CONFIRM]) {
        upgrade_apply(g, &g->choices[g->choice_sel]);
        if (g->player.pending_levelups > 0) g->player.pending_levelups--;
        if (g->player.pending_levelups > 0) game_open_levelup(g);
        else game_set_state(g, GS_PLAYING);
    }
}

static void levelup_draw(Game *g) {
    dim_overlay(170);
    draw_text_center(SCREEN_W * 0.5f, 40.0f, COL_YELLOW, 1.6f, "SUBIU DE NIVEL!");
    char b[48];
    snprintf(b, sizeof(b), "Nivel %d  -  escolha uma melhoria", g->player.level);
    draw_text_center(SCREEN_W * 0.5f, 86.0f, COL_DIM, 0.7f, b);

    float w = 580.0f, h = 78.0f, cx = SCREEN_W * 0.5f, x = cx - w * 0.5f, top = 130.0f, sp = 90.0f;
    for (int i = 0; i < g->choice_count; ++i) {
        UpgradeChoice *c = &g->choices[i];
        float y = top + i * sp;
        bool sel = (i == g->choice_sel);
        panel(x, y, w, h);
        if (sel) {
            draw_rect_outline(x, y, w, h, 2.0f, COL_ACCENT);
            draw_rect(x, y, 6.0f, h, COL_ACCENT);
        }
        if (c->kind == UPK_WEAPON_NEW || c->kind == UPK_WEAPON_LEVEL || c->kind == UPK_FUSION)
            sprite_draw(sprite_for_weapon(c->weapon), x + 35.0f, y + 39.0f, 36.0f);
        else
            draw_rect(x + 18.0f, y + 22.0f, 34.0f, 34.0f, c->color);
        draw_text(x + 66.0f, y + 14.0f, sel ? COL_WHITE : RGBA8(0xCF,0xD0,0xDC,0xFF), 0.95f, c->title);
        draw_text(x + 66.0f, y + 44.0f, COL_DIM, 0.66f, c->desc);
    }
    draw_text_center(cx, SCREEN_H - 24.0f, RGBA8(0x66,0x68,0x76,0xFF), 0.6f,
                     "Cima/Baixo: escolher    X: confirmar");
}

/* =============================================================
 *  PAUSA
 * ============================================================= */
static void pause_update(Game *g, float dt) {
    const int items = 3;
    int dv = input_menu_vert(&g->input, dt);
    if (dv) g->sel_pause = (g->sel_pause + dv + items) % items;

    if (g->input.pressed[BTN_PAUSE] || g->input.pressed[BTN_CANCEL]) {
        game_set_state(g, GS_PLAYING);
        return;
    }
    if (g->input.pressed[BTN_CONFIRM]) {
        switch (g->sel_pause) {
            case 0: game_set_state(g, GS_PLAYING); break;
            case 1: game_save_run(g); game_set_state(g, GS_MENU); break;
            case 2: g->save.has_run = 0; save_write(&g->save); game_set_state(g, GS_MENU); break;
        }
    }
}

static void pause_draw(Game *g) {
    dim_overlay(170);
    float cx = SCREEN_W * 0.5f;
    draw_text_center(cx, 110.0f, COL_WHITE, 1.6f, "PAUSA");
    const char *items[3] = { "Continuar", "Salvar e sair", "Sair sem salvar" };
    float top = 220.0f, sp = 50.0f, w = 360.0f;
    for (int i = 0; i < 3; ++i) {
        float y = top + i * sp;
        bool sel = (i == g->sel_pause);
        row_highlight(cx - w * 0.5f, y - 4.0f, w, 36.0f, sel);
        draw_text_center(cx, y, sel ? COL_WHITE : COL_DIM, 0.95f, items[i]);
    }
}

/* =============================================================
 *  GAME OVER
 * ============================================================= */
static void gameover_update(Game *g, float dt) {
    const int items = 2;
    int dv = input_menu_vert(&g->input, dt);
    if (dv) g->sel_gameover = (g->sel_gameover + dv + items) % items;

    if (g->input.pressed[BTN_CONFIRM]) {
        if (g->sel_gameover == 0) {
            g->sel_char = (int)g->save.selected_char;
            game_set_state(g, GS_CHARSELECT);
        } else {
            game_set_state(g, GS_MENU);
        }
    }
}

static void gameover_draw(Game *g) {
    dim_overlay(200);
    float cx = SCREEN_W * 0.5f;
    draw_text_center(cx, 70.0f, COL_RED, 1.8f, "VOCE MORREU");

    char b[48];
    panel(cx - 200.0f, 140.0f, 400.0f, 150.0f);
    fmt_time_local(b, sizeof(b), g->wave.time);
    draw_text(cx - 180.0f, 152.0f, COL_DIM, 0.8f, "Tempo:");
    draw_text_right(cx + 180.0f, 152.0f, COL_WHITE, 0.8f, b);
    snprintf(b, sizeof(b), "%d", g->player.level);
    draw_text(cx - 180.0f, 182.0f, COL_DIM, 0.8f, "Nivel:");
    draw_text_right(cx + 180.0f, 182.0f, COL_WHITE, 0.8f, b);
    snprintf(b, sizeof(b), "%d", g->wave.kills);
    draw_text(cx - 180.0f, 212.0f, COL_DIM, 0.8f, "Abates:");
    draw_text_right(cx + 180.0f, 212.0f, COL_WHITE, 0.8f, b);
    snprintf(b, sizeof(b), "+%u", (unsigned)g->run_gained);
    draw_text(cx - 180.0f, 242.0f, COL_DIM, 0.8f, "Moedas ganhas:");
    draw_text_right(cx + 180.0f, 242.0f, COL_YELLOW, 0.8f, b);

    const char *items[2] = { "Jogar novamente", "Menu principal" };
    float top = 330.0f, sp = 48.0f, w = 360.0f;
    for (int i = 0; i < 2; ++i) {
        float y = top + i * sp;
        bool sel = (i == g->sel_gameover);
        row_highlight(cx - w * 0.5f, y - 4.0f, w, 36.0f, sel);
        draw_text_center(cx, y, sel ? COL_WHITE : COL_DIM, 0.95f, items[i]);
    }
}

/* =============================================================
 *  Dispatch
 * ============================================================= */
void menu_update(Game *g, float dt) {
    if (g->input.pressed[BTN_CONFIRM]) audio_play_sfx(SFX_SELECT);
    if (g->input.pressed[BTN_UP] || g->input.pressed[BTN_DOWN] ||
        g->input.pressed[BTN_LEFT] || g->input.pressed[BTN_RIGHT]) audio_play_sfx(SFX_MOVE);

    switch (g->state) {
        case GS_MENU:       main_update(g, dt); break;
        case GS_CHARSELECT: charselect_update(g, dt); break;
        case GS_SHOP:       shop_update(g, dt); break;
        case GS_COLLECTION: collection_update(g, dt); break;
        case GS_MISSIONS:   missions_update(g, dt); break;
        case GS_SETTINGS:   settings_update(g, dt); break;
        case GS_LEVELUP:    levelup_update(g, dt); break;
        case GS_PAUSE:      pause_update(g, dt); break;
        case GS_GAMEOVER:   gameover_update(g, dt); break;
        default: break;
    }
}

void menu_draw(Game *g) {
    switch (g->state) {
        case GS_MENU:       main_draw(g); break;
        case GS_CHARSELECT: charselect_draw(g); break;
        case GS_SHOP:       shop_draw(g); break;
        case GS_COLLECTION: collection_draw(g); break;
        case GS_MISSIONS:   missions_draw(g); break;
        case GS_SETTINGS:   settings_draw(g); break;
        case GS_LEVELUP:    levelup_draw(g); break;
        case GS_PAUSE:      pause_draw(g); break;
        case GS_GAMEOVER:   gameover_draw(g); break;
        default: break;
    }
}
