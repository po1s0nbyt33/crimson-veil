#include "game.h"
#include <string.h>
#include <stdio.h>

int pickup_spawn(Game *g, PickupType t, Vec2 pos, int value) {
    for (int i = 0; i < MAX_PICKUPS; ++i) {
        Pickup *k = &g->pickups[i];
        if (k->active) continue;

        memset(k, 0, sizeof(*k));
        k->active = true;
        k->type = t;
        k->pos = pos;
        k->value = value;
        k->vel = v2scale(rng_dir(&g->rng), rng_range(&g->rng, 20.0f, 70.0f));
        k->bob = rng_f01(&g->rng) * 6.2831f;

        switch (t) {
            case PICK_XP:     k->radius = (value >= 5 ? 7.0f : value >= 3 ? 6.0f : 4.5f); break;
            case PICK_COIN:   k->radius = 5.0f; break;
            case PICK_HEAL:   k->radius = 7.0f; break;
            case PICK_MAGNET: k->radius = 9.0f; break;
            case PICK_BOMB:   k->radius = 9.0f; break;
            default:          k->radius = 5.0f; break;
        }
        return i;
    }
    return -1;
}

void pickups_magnet_all(Game *g) {
    for (int i = 0; i < MAX_PICKUPS; ++i) {
        Pickup *k = &g->pickups[i];
        if (k->active && k->type == PICK_XP) k->homing = true;
    }
}

void pickups_update(Game *g, float dt) {
    Player *p = &g->player;
    float pr2 = p->pickup_radius * p->pickup_radius;

    for (int i = 0; i < MAX_PICKUPS; ++i) {
        Pickup *k = &g->pickups[i];
        if (!k->active) continue;

        k->bob += dt * 4.0f;
        float d2 = v2dist2(k->pos, p->pos);

        if (!k->homing) {
            k->pos = v2add(k->pos, v2scale(k->vel, dt));
            k->vel = v2scale(k->vel, clampf(1.0f - 3.5f * dt, 0.0f, 1.0f));
            if (d2 <= pr2) k->homing = true;
        }

        if (k->homing) {
            Vec2 dir = v2norm(v2sub(p->pos, k->pos));
            float sp = clampf(260.0f + (p->pickup_radius - sqrtf(d2)) * 2.0f, 240.0f, 660.0f);
            k->pos = v2add(k->pos, v2scale(dir, sp * dt));

            float cr = PLAYER_COLLECT_DIST + k->radius;
            if (v2dist2(k->pos, p->pos) <= cr * cr) {
                switch (k->type) {
                    case PICK_XP: {
                        player_gain_xp(g, k->value);
                        static float t_xp = 0.0f;
                        if (g->global_time - t_xp > 0.05f) { audio_play_sfx(SFX_XP); t_xp = g->global_time; }
                    } break;
                    case PICK_COIN:
                        g->run_coins += (uint32_t)maxi(1, (int)(k->value * p->greed + 0.5f));
                        break;
                    case PICK_HEAL:
                        player_heal(p, (float)k->value);
                        if (g->save.opt_damage_numbers) {
                            char b[16];
                            snprintf(b, sizeof(b), "+%d", k->value);
                            fx_floater(g, p->pos, b, COL_GREEN, 0.8f);
                        }
                        break;
                    case PICK_MAGNET:
                        pickups_magnet_all(g);
                        fx_floater(g, p->pos, "IMA!", COL_CYAN, 1.0f);
                        break;
                    case PICK_BOMB:
                        for (int e = 0; e < MAX_ENEMIES; ++e)
                            if (g->enemies[e].active && !g->enemies[e].is_boss)
                                enemy_hurt(g, e, 9999.0f);
                        fx_shake(g, 14.0f);
                        g->flash = maxf(g->flash, 0.5f);
                        fx_floater(g, p->pos, "BOOM!", COL_ORANGE, 1.2f);
                        audio_play_sfx(SFX_BOMB);
                        break;
                    default: break;
                }
                k->active = false;
            }
        }
    }
}

void pickups_draw(Game *g) {
    for (int i = 0; i < MAX_PICKUPS; ++i) {
        Pickup *k = &g->pickups[i];
        if (!k->active) continue;

        Vec2 s = v2sub(k->pos, g->camera);
        if (s.x < -20.0f || s.x > SCREEN_W + 20.0f || s.y < -20.0f || s.y > SCREEN_H + 20.0f)
            continue;

        float bob = sinf(k->bob) * 2.0f;
        switch (k->type) {
            case PICK_XP: {
                uint32_t c = (k->value >= 5) ? COL_CYAN : (k->value >= 3) ? COL_GREEN : COL_BLUE;
                draw_circle(s.x, s.y + bob, k->radius, c);
                draw_circle(s.x, s.y + bob, k->radius * 0.5f, COL_WHITE);
            } break;
            case PICK_COIN:
                draw_circle(s.x, s.y + bob, k->radius, COL_YELLOW);
                draw_circle(s.x, s.y + bob, k->radius * 0.5f, RGBA8(0xFF, 0xF0, 0xB0, 0xFF));
                break;
            case PICK_HEAL:
                draw_circle(s.x, s.y + bob, k->radius, COL_GREEN);
                draw_rect(s.x - 1.0f, s.y + bob - 3.0f, 2.0f, 6.0f, COL_WHITE);
                draw_rect(s.x - 3.0f, s.y + bob - 1.0f, 6.0f, 2.0f, COL_WHITE);
                break;
            case PICK_MAGNET:
                draw_circle(s.x, s.y + bob, k->radius, COL_CYAN);
                draw_circle(s.x, s.y + bob, k->radius * 0.5f, COL_WHITE);
                break;
            case PICK_BOMB:
                draw_circle(s.x, s.y + bob, k->radius, COL_ORANGE);
                draw_circle(s.x, s.y + bob, k->radius * 0.5f, COL_RED);
                break;
            default: break;
        }
    }
}
