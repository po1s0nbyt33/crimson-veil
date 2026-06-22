#include "game.h"

/* name, hp, speed, radius, damage, color, xp, coins */
static const BossDef BOSSES[] = {
    { "Senhor das Sombras", 380.0f,  46.0f, 32.0f, 26.0f, RGBA8(0xE0, 0x3C, 0x6C, 0xFF),  60, 18 }, /* 0: carrega + invoca */
    { "O Invocador",        640.0f,  34.0f, 36.0f, 28.0f, RGBA8(0x8C, 0x3C, 0xE0, 0xFF),  90, 26 }, /* 1: invoca hordas  */
    { "Ceifador Sangrento", 900.0f,  54.0f, 34.0f, 34.0f, RGBA8(0xE0, 0x5C, 0x2C, 0xFF), 120, 34 }, /* 2: carrega + leque */
    { "Arauto das Chamas", 1150.0f,  40.0f, 34.0f, 32.0f, RGBA8(0xF0, 0x6A, 0x2C, 0xFF), 150, 44 }, /* 3: rajada radial   */
    { "Rainha Abissal",    1500.0f,  38.0f, 40.0f, 40.0f, RGBA8(0x3C, 0x9C, 0xE0, 0xFF), 200, 58 }, /* 4: baque em area   */
};
#define NBOSS ((int)(sizeof(BOSSES) / sizeof(BOSSES[0])))

int boss_count(void) { return NBOSS; }

const char *boss_name(int id) {
    if (id < 0 || id >= NBOSS) return "Chefe";
    return BOSSES[id].name;
}

void boss_spawn(Game *g, int wave_index) {
    int id = (wave_index > 0 ? (wave_index - 1) : 0) % NBOSS;
    const BossDef *d = &BOSSES[id];

    Vec2 dir = rng_dir(&g->rng);
    Vec2 pos = v2add(g->player.pos, v2scale(dir, SPAWN_RING_MIN));
    int idx = enemy_spawn(g, ENEMY_BOSS, pos);
    if (idx < 0) return;

    Enemy *e = &g->enemies[idx];
    /* sobe mais a cada ciclo completo pela lista de chefes */
    float scale = 1.0f + 0.30f * (float)((wave_index - 1) / NBOSS);
    if (scale < 1.0f) scale = 1.0f;

    e->boss_id  = id;
    e->max_hp   = d->hp * scale;
    e->hp       = e->max_hp;
    e->speed    = d->speed;
    e->radius   = d->radius;
    e->damage   = d->damage * (0.9f + 0.05f * (float)wave_index);
    e->color    = d->color;
    e->xp       = d->xp;
    e->coins    = d->coins;
    e->attack_cd = 3.0f;
    e->phase    = 0;

    g->boss_idx = idx;

    /* entrada especial */
    fx_floater(g, v2(g->player.pos.x, g->player.pos.y - 92.0f), "CHEFE", COL_RED, 1.0f);
    fx_floater(g, v2(g->player.pos.x, g->player.pos.y - 64.0f), d->name, COL_RED, 1.3f);
    fx_shake(g, 12.0f);
    g->flash = maxf(g->flash, 0.34f);
    audio_play_sfx(SFX_BOSS);
}

static void chase(Enemy *e, Vec2 dir, float dt) {
    e->pos = v2add(e->pos, v2scale(dir, e->speed * dt));
}

void boss_update(Game *g, int idx, float dt) {
    Enemy *e = &g->enemies[idx];
    Player *p = &g->player;
    Vec2 to = v2sub(p->pos, e->pos);
    float dist = v2len(to);
    Vec2 dir = (dist > 1e-3f) ? v2scale(to, 1.0f / dist) : v2(0.0f, 1.0f);

    /* fase 1: investida em linha reta (direcao travada) */
    if (e->phase == 1) {
        e->phase_t -= dt;
        e->pos = v2add(e->pos, v2scale(e->vel, dt));
        if (e->phase_t <= 0.0f) e->phase = 0;
        return;
    }
    /* fase 2: telegrafo do baque (Rainha Abissal) */
    if (e->phase == 2) {
        e->phase_t -= dt;
        if (e->phase_t <= 0.0f) {
            const float R = 135.0f;
            if (dist < R) player_take_damage(g, e->damage * 1.5f);
            fx_burst(g, e->pos, RGBA8(0x6C, 0xC0, 0xFF, 0xFF), 22, 220.0f);
            fx_shake(g, 12.0f);
            for (int k = 0; k < 3; ++k)
                enemy_spawn(g, ENEMY_RUNNER, v2add(e->pos, v2scale(rng_dir(&g->rng), e->radius + 24.0f)));
            e->phase = 0;
            e->attack_cd = 4.2f;
        }
        return;
    }

    e->attack_cd -= dt;

    switch (e->boss_id) {
        case 1: /* O Invocador */
            chase(e, dir, dt);
            if (e->attack_cd <= 0.0f) {
                e->attack_cd = 3.6f;
                for (int k = 0; k < 4; ++k) {
                    EnemyType t = (k & 1) ? ENEMY_RUNNER : ENEMY_SWARM;
                    enemy_spawn(g, t, v2add(e->pos, v2scale(rng_dir(&g->rng), e->radius + 20.0f)));
                }
                fx_burst(g, e->pos, e->color, 12, 120.0f);
            }
            break;

        case 3: /* Arauto das Chamas — rajada radial (bullet hell) */
            if (dist > 220.0f) chase(e, dir, dt);
            if (e->attack_cd <= 0.0f) {
                e->attack_cd = 2.6f;
                const int n = 14;
                for (int k = 0; k < n; ++k) {
                    float a = (float)k / (float)n * 6.2831853f;
                    Vec2 v = v2scale(v2(cosf(a), sinf(a)), 185.0f);
                    eproj_spawn(g, e->pos, v, e->damage, 8.0f, 5.0f, false, false, RGBA8(0xFF, 0x8A, 0x3C, 0xFF));
                }
                fx_shake(g, 5.0f);
            }
            break;

        case 4: /* Rainha Abissal — prepara o baque */
            chase(e, dir, dt);
            if (e->attack_cd <= 0.0f) {
                e->phase = 2;
                e->phase_t = 0.85f;
            }
            break;

        case 2: /* Ceifador — investida + leque de projeteis */
        case 0: /* Senhor das Sombras — investida + invocacao */
        default:
            chase(e, dir, dt);
            if (e->attack_cd <= 0.0f) {
                e->attack_cd = 4.5f;
                e->phase = 1;
                e->phase_t = 0.7f;
                e->vel = v2scale(dir, e->speed * 5.0f);   /* trava a investida */
                fx_shake(g, 6.0f);
                if (e->boss_id == 2) {
                    float ba = atan2f(dir.y, dir.x);
                    for (int k = -1; k <= 1; ++k) {
                        float a = ba + k * 0.26f;
                        eproj_spawn(g, e->pos, v2scale(v2(cosf(a), sinf(a)), 240.0f),
                                    e->damage, 7.0f, 4.0f, false, false, RGBA8(0xE0, 0x5C, 0x2C, 0xFF));
                    }
                } else { /* boss 0 */
                    for (int k = 0; k < 3; ++k)
                        enemy_spawn(g, ENEMY_RUNNER, v2add(e->pos, v2scale(rng_dir(&g->rng), e->radius + 22.0f)));
                }
            }
            break;
    }
}
