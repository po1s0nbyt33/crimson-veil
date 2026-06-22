#include "game.h"
#include <string.h>
#include <stdio.h>

void fx_reset(Game *g) {
    memset(g->floaters, 0, sizeof(g->floaters));
    memset(g->particles, 0, sizeof(g->particles));
    g->shake = 0.0f;
    g->flash = 0.0f;
}

void fx_floater(Game *g, Vec2 pos, const char *text, uint32_t color, float scale) {
    for (int i = 0; i < MAX_FLOATERS; ++i) {
        Floater *f = &g->floaters[i];
        if (f->active) continue;
        memset(f, 0, sizeof(*f));
        f->active = true;
        f->pos = pos;
        f->vel = v2(rng_range(&g->rng, -10.0f, 10.0f), -42.0f);
        f->life = 0.8f;
        f->maxlife = 0.8f;
        f->color = color;
        f->scale = (scale <= 0.0f) ? 0.8f : scale;
        snprintf(f->text, sizeof(f->text), "%s", text);
        return;
    }
}

void fx_number(Game *g, Vec2 pos, int amount) {
    char b[16];
    snprintf(b, sizeof(b), "%d", amount);
    fx_floater(g, pos, b, COL_WHITE, 0.7f);
}

void fx_burst(Game *g, Vec2 pos, uint32_t color, int count, float speed) {
    for (int n = 0; n < count; ++n) {
        for (int i = 0; i < MAX_PARTICLES; ++i) {
            Particle *pa = &g->particles[i];
            if (pa->active) continue;
            memset(pa, 0, sizeof(*pa));
            pa->active = true;
            pa->pos = pos;
            pa->vel = v2scale(rng_dir(&g->rng), rng_range(&g->rng, speed * 0.3f, speed));
            pa->life = rng_range(&g->rng, 0.25f, 0.55f);
            pa->maxlife = pa->life;
            pa->radius = rng_range(&g->rng, 2.0f, 4.5f);
            pa->color = color;
            break;
        }
    }
}

void fx_shake(Game *g, float amount) {
    g->shake = maxf(g->shake, amount);
}

void fx_update(Game *g, float dt) {
    for (int i = 0; i < MAX_FLOATERS; ++i) {
        Floater *f = &g->floaters[i];
        if (!f->active) continue;
        f->life -= dt;
        if (f->life <= 0.0f) { f->active = false; continue; }
        f->pos = v2add(f->pos, v2scale(f->vel, dt));
        f->vel.y += 30.0f * dt;
    }
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        Particle *p = &g->particles[i];
        if (!p->active) continue;
        p->life -= dt;
        if (p->life <= 0.0f) { p->active = false; continue; }
        p->pos = v2add(p->pos, v2scale(p->vel, dt));
        p->vel = v2scale(p->vel, clampf(1.0f - 4.0f * dt, 0.0f, 1.0f));
    }
    if (g->shake > 0.0f) { g->shake -= dt * 40.0f; if (g->shake < 0.0f) g->shake = 0.0f; }
    if (g->flash > 0.0f) { g->flash -= dt * 1.4f; if (g->flash < 0.0f) g->flash = 0.0f; }
}

void fx_draw(Game *g) {
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        Particle *p = &g->particles[i];
        if (!p->active) continue;
        Vec2 s = v2sub(p->pos, g->camera);
        if (s.x < -16.0f || s.x > SCREEN_W + 16.0f ||
            s.y < -16.0f || s.y > SCREEN_H + 16.0f) continue;   /* cull fora da tela */
        float lf = p->life / p->maxlife;
        draw_circle(s.x, s.y, p->radius * lf + 0.5f, col_a(p->color, (unsigned char)(255.0f * lf)));
    }
    for (int i = 0; i < MAX_FLOATERS; ++i) {
        Floater *f = &g->floaters[i];
        if (!f->active) continue;
        Vec2 s = v2sub(f->pos, g->camera);
        if (s.x < -64.0f || s.x > SCREEN_W + 64.0f ||
            s.y < -32.0f || s.y > SCREEN_H + 32.0f) continue;   /* cull fora da tela */
        float lf = f->life / f->maxlife;
        draw_text_center(s.x, s.y, col_a(f->color, (unsigned char)(255.0f * clampf(lf * 1.5f, 0.0f, 1.0f))),
                         f->scale, f->text);
    }
}
