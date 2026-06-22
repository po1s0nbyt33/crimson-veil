#include "game.h"
#include <string.h>

/* Dano minimo para gerar flash/knockback/numero (filtra ticks de aura). */
#define HIT_THRESHOLD 2.0f

int enemy_spawn(Game *g, EnemyType t, Vec2 pos) {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        Enemy *e = &g->enemies[i];
        if (e->active) continue;

        memset(e, 0, sizeof(*e));
        e->active = true;
        e->type = t;
        e->pos = pos;

        switch (t) {
            case ENEMY_SWARM:
                e->max_hp = 12.0f; e->speed = 46.0f; e->radius = 9.0f;
                e->damage = 8.0f;  e->xp = 1; e->color = RGBA8(0xB0, 0x6C, 0xE0, 0xFF);
                break;
            case ENEMY_RUNNER:
                e->max_hp = 8.0f;  e->speed = 94.0f; e->radius = 8.0f;
                e->damage = 6.0f;  e->xp = 1; e->color = RGBA8(0xE8, 0x5B, 0x5B, 0xFF);
                break;
            case ENEMY_BRUTE:
                e->max_hp = 58.0f; e->speed = 34.0f; e->radius = 16.0f;
                e->damage = 18.0f; e->xp = 4; e->coins = 1;
                e->color = RGBA8(0xC8, 0x8A, 0x4C, 0xFF);
                break;
            case ENEMY_SHOOTER:
                e->max_hp = 22.0f; e->speed = 40.0f; e->radius = 10.0f;
                e->damage = 10.0f; e->xp = 2; e->color = RGBA8(0x4F, 0xC6, 0xC0, 0xFF);
                e->attack_cd = 1.5f;
                break;
            case ENEMY_MAGE:
                e->max_hp = 18.0f; e->speed = 34.0f; e->radius = 10.0f;
                e->damage = 12.0f; e->xp = 3; e->coins = 1;
                e->color = RGBA8(0xC0, 0x7C, 0xF0, 0xFF);
                e->attack_cd = 2.2f;
                break;
            case ENEMY_BOSS:
                e->max_hp = 400.0f; e->speed = 42.0f; e->radius = 34.0f;
                e->damage = 28.0f;  e->xp = 60; e->coins = 20; e->is_boss = true;
                e->color = RGBA8(0xE0, 0x3C, 0x6C, 0xFF);
                break;
            default: break;
        }

        if (t != ENEMY_BOSS) {
            float diff = wave_difficulty(g);
            e->max_hp *= diff;
            e->damage *= (0.8f + 0.2f * diff);
            e->xp += (int)(diff * 0.3f);
        }
        e->hp = e->max_hp;
        return i;
    }
    return -1;
}

void enemies_update(Game *g, float dt) {
    Player *p = &g->player;
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        Enemy *e = &g->enemies[i];
        if (!e->active) continue;

        if (e->hit_flash > 0.0f) e->hit_flash -= dt;

        if (e->is_boss) {
            boss_update(g, i, dt);
        } else {
            Vec2 to = v2sub(p->pos, e->pos);
            float dist = v2len(to);
            Vec2 dir = (dist > 1e-3f) ? v2scale(to, 1.0f / dist) : v2(0.0f, 1.0f);

            if (e->knockback > 0.0f) {
                e->knockback -= dt;
                e->pos = v2sub(e->pos, v2scale(dir, e->speed * 0.7f * dt));   /* recua */
            } else if (e->type == ENEMY_SHOOTER || e->type == ENEMY_MAGE) {
                /* mantem distancia e atira */
                float pref = (e->type == ENEMY_MAGE) ? 320.0f : 250.0f;
                if (dist > pref + 30.0f)      e->pos = v2add(e->pos, v2scale(dir, e->speed * dt));
                else if (dist < pref - 40.0f) e->pos = v2sub(e->pos, v2scale(dir, e->speed * dt));
                e->attack_cd -= dt;
                if (e->attack_cd <= 0.0f && dist < 560.0f) {
                    if (e->type == ENEMY_MAGE) {
                        e->attack_cd = 2.4f;
                        eproj_spawn(g, e->pos, v2scale(dir, 150.0f), e->damage, 8.0f, 4.5f, true, true,
                                    RGBA8(0xC8, 0x82, 0xFF, 0xFF));
                    } else {
                        e->attack_cd = 1.6f;
                        eproj_spawn(g, e->pos, v2scale(dir, 250.0f), e->damage, 6.0f, 3.0f, false, true,
                                    RGBA8(0x7A, 0xEC, 0xE6, 0xFF));
                    }
                }
            } else {
                e->pos = v2add(e->pos, v2scale(dir, e->speed * dt));
            }

            if (v2dist2(e->pos, p->pos) > DESPAWN_DIST * DESPAWN_DIST) {
                e->active = false;
                continue;
            }
        }

        float rr = e->radius + PLAYER_RADIUS;
        if (v2dist2(e->pos, p->pos) <= rr * rr)
            player_take_damage(g, e->damage);
    }
}

void enemies_draw(Game *g) {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        Enemy *e = &g->enemies[i];
        if (!e->active) continue;

        Vec2 s = v2sub(e->pos, g->camera);
        float m = e->radius + 40.0f;
        if (s.x < -m || s.x > SCREEN_W + m || s.y < -m || s.y > SCREEN_H + m) continue;

        /* sombra */
        draw_circle(s.x, s.y + e->radius * 0.7f, e->radius * 0.85f, RGBA8(0, 0, 0, 70));

        /* sprite (inimigo comum ou boss) */
        SpriteId sid = e->is_boss ? sprite_for_boss(e->boss_id) : sprite_for_enemy(e->type);
        sprite_draw(sid, s.x, s.y, e->radius * 2.4f);

        /* feedback de dano */
        if (e->hit_flash > 0.0f)
            draw_circle(s.x, s.y, e->radius,
                        col_a(COL_WHITE, (unsigned char)(150.0f * clampf(e->hit_flash / 0.08f, 0.0f, 1.0f))));

        if (e->is_boss)
            draw_ring(s.x, s.y, e->radius + 3.0f, 2.0f, COL_RED);

        if ((e->is_boss || e->type == ENEMY_BRUTE) && e->hp < e->max_hp) {
            float w = e->radius * 2.0f;
            draw_bar(s.x - w * 0.5f, s.y - e->radius - 7.0f, w, 3.0f,
                     e->hp / e->max_hp, RGBA8(0, 0, 0, 160), COL_RED);
        }
    }
}

void enemy_hurt(Game *g, int idx, float dmg) {
    Enemy *e = &g->enemies[idx];
    if (!e->active) return;

    e->hp -= dmg;

    if (dmg >= HIT_THRESHOLD) {
        e->hit_flash = 0.08f;
        if (!e->is_boss) e->knockback = 0.05f;
        if (g->save.opt_damage_numbers)
            fx_number(g, v2(e->pos.x, e->pos.y - e->radius), (int)(dmg + 0.5f));
    }

    if (e->hp <= 0.0f) enemy_kill(g, idx);
}

void enemy_kill(Game *g, int idx) {
    Enemy *e = &g->enemies[idx];
    if (!e->active) return;
    e->active = false;
    if (idx == g->boss_idx) g->boss_idx = -1;   /* nunca deixa boss_idx pendurado */
    g->wave.kills++;

    {   /* som de abate com limite para nao saturar em mortes em massa */
        static float t_kill = 0.0f;
        if (g->global_time - t_kill > 0.045f) { audio_play_sfx(SFX_KILL); t_kill = g->global_time; }
    }

    fx_burst(g, e->pos, e->color, e->is_boss ? 26 : 6, e->is_boss ? 170.0f : 90.0f);

    if (e->xp > 0)    pickup_spawn(g, PICK_XP, e->pos, e->xp);
    if (e->coins > 0) pickup_spawn(g, PICK_COIN, e->pos, e->coins);
    if (!e->is_boss && rng_f01(&g->rng) < 0.012f)
        pickup_spawn(g, PICK_HEAL, e->pos, 25);

    if (e->is_boss) {
        g->boss_idx = -1;
        g->save.bosses_defeated |= (1u << e->boss_id);   /* registra p/ missoes/colecao */
        save_write(&g->save);
        fx_shake(g, 16.0f);
        g->flash = maxf(g->flash, 0.40f);
        fx_floater(g, v2(e->pos.x, e->pos.y - e->radius - 10.0f), "CHEFE DERROTADO!", COL_YELLOW, 1.1f);
        pickup_spawn(g, PICK_HEAL,   v2add(e->pos, v2( 14.0f, 0.0f)), 50);
        pickup_spawn(g, PICK_MAGNET, v2add(e->pos, v2(-14.0f, 0.0f)), 0);
        pickup_spawn(g, PICK_COIN,   e->pos, 15);
        for (int k = 0; k < 6; ++k)
            pickup_spawn(g, PICK_XP, v2add(e->pos, v2scale(rng_dir(&g->rng), 18.0f)), 5);
    }
}

/* =============================================================
 *  Projeteis inimigos (atingem o jogador)
 * ============================================================= */
void eproj_spawn(Game *g, Vec2 pos, Vec2 vel, float dmg, float radius,
                 float life, bool homing, bool destructible, uint32_t color) {
    for (int i = 0; i < MAX_EPROJECTILES; ++i) {
        EnemyProjectile *e = &g->eprojectiles[i];
        if (e->active) continue;
        memset(e, 0, sizeof(*e));
        e->active = true;
        e->pos = pos;
        e->vel = vel;
        e->damage = dmg;
        e->radius = radius;
        e->life = life;
        e->homing = homing;
        e->destructible = destructible;
        e->color = color;
        return;
    }
}

void eprojectiles_update(Game *g, float dt) {
    Player *p = &g->player;
    for (int i = 0; i < MAX_EPROJECTILES; ++i) {
        EnemyProjectile *e = &g->eprojectiles[i];
        if (!e->active) continue;

        e->life -= dt;
        if (e->life <= 0.0f) { e->active = false; continue; }
        e->spin += dt * 8.0f;

        if (e->homing) {
            float spd = v2len(e->vel);
            if (spd < 1.0f) spd = 150.0f;
            Vec2 want = v2scale(v2norm(v2sub(p->pos, e->pos)), spd);
            e->vel = v2add(v2scale(e->vel, 0.92f), v2scale(want, 0.08f));   /* steer suave */
        }
        e->pos = v2add(e->pos, v2scale(e->vel, dt));

        float rr = e->radius + PLAYER_RADIUS;
        if (v2dist2(e->pos, p->pos) <= rr * rr) {
            player_take_damage(g, e->damage);
            e->active = false;
        }
    }
}

void eprojectiles_draw(Game *g) {
    for (int i = 0; i < MAX_EPROJECTILES; ++i) {
        EnemyProjectile *e = &g->eprojectiles[i];
        if (!e->active) continue;
        Vec2 s = v2sub(e->pos, g->camera);
        if (s.x < -24.0f || s.x > SCREEN_W + 24.0f ||
            s.y < -24.0f || s.y > SCREEN_H + 24.0f) continue;
        draw_circle(s.x, s.y, e->radius + 1.5f, col_a(e->color, 90));
        draw_circle(s.x, s.y, e->radius, e->color);
        draw_circle(s.x, s.y, e->radius * 0.5f, COL_WHITE);
    }
}

int enemy_nearest(Game *g, Vec2 from, float maxdist) {
    int best = -1;
    float bd = maxdist * maxdist;
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        Enemy *e = &g->enemies[i];
        if (!e->active) continue;
        float d = v2dist2(from, e->pos);
        if (d < bd) { bd = d; best = i; }
    }
    return best;
}

int enemies_alive(Game *g) {
    int n = 0;
    for (int i = 0; i < MAX_ENEMIES; ++i)
        if (g->enemies[i].active) n++;
    return n;
}
