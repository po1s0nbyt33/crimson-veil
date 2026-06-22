#include "game.h"
#include <string.h>

void wave_init(WaveDirector *w) {
    memset(w, 0, sizeof(*w));
    w->next_boss_at = BOSS_INTERVAL;
}

float wave_difficulty(Game *g) {
    return 1.0f + g->wave.time / 60.0f;
}

int wave_minute(Game *g) {
    return (int)(g->wave.time / 60.0f);
}

static void spawn_at_ring(Game *g, EnemyType t) {
    Vec2 dir = rng_dir(&g->rng);
    float r = rng_range(&g->rng, SPAWN_RING_MIN, SPAWN_RING_MAX);
    enemy_spawn(g, t, v2add(g->player.pos, v2scale(dir, r)));
}

static EnemyType pick_type(Game *g) {
    float t = g->wave.time;
    float r = rng_f01(&g->rng);
    if (t < 60.0f)  return (r < 0.85f) ? ENEMY_SWARM : ENEMY_RUNNER;
    if (t < 150.0f) {
        if (r < 0.50f) return ENEMY_SWARM;
        if (r < 0.78f) return ENEMY_RUNNER;
        if (r < 0.90f) return ENEMY_BRUTE;
        return ENEMY_SHOOTER;
    }
    /* tarde: variedade total, incluindo magos */
    if (r < 0.34f) return ENEMY_SWARM;
    if (r < 0.58f) return ENEMY_RUNNER;
    if (r < 0.74f) return ENEMY_BRUTE;
    if (r < 0.88f) return ENEMY_SHOOTER;
    return ENEMY_MAGE;
}

void wave_update(Game *g, float dt) {
    WaveDirector *w = &g->wave;
    w->time += dt;

    int alive = enemies_alive(g);
    const int cap = MAX_ENEMIES - 24;

    /* spawn continuo, taxa crescente com o tempo */
    float rate = 0.6f + w->time * 0.03f;
    if (rate > 14.0f) rate = 14.0f;
    w->spawn_acc += dt * rate;
    while (w->spawn_acc >= 1.0f) {
        w->spawn_acc -= 1.0f;
        if (alive < cap) { spawn_at_ring(g, pick_type(g)); alive++; }
    }

    /* ondas em grupo (pressao por um lado) */
    w->pack_acc += dt;
    float pack_every = clampf(20.0f - w->time * 0.02f, 8.0f, 20.0f);
    if (w->pack_acc >= pack_every) {
        w->pack_acc = 0.0f;
        if (alive < cap - 12) {
            Vec2 dir = rng_dir(&g->rng);
            int n = 6 + (int)(w->time / 40.0f);
            if (n > 20) n = 20;
            for (int k = 0; k < n && alive < cap; ++k) {
                Vec2 base = v2add(g->player.pos,
                                  v2scale(dir, rng_range(&g->rng, SPAWN_RING_MIN, SPAWN_RING_MAX)));
                base = v2add(base, v2scale(rng_dir(&g->rng), rng_range(&g->rng, 0.0f, 60.0f)));
                enemy_spawn(g, ENEMY_SWARM, base);
                alive++;
            }
        }
    }

    /* chefe nos marcos de tempo */
    if (g->boss_idx < 0 && (int)w->time >= w->next_boss_at) {
        w->bosses_spawned++;
        boss_spawn(g, w->bosses_spawned);
        w->next_boss_at += BOSS_INTERVAL;
    }
}
