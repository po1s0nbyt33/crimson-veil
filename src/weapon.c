#include "game.h"
#include <string.h>

/* =============================================================
 *  Nomes / descricoes / cores
 * ============================================================= */
const char *weapon_name(WeaponType t) {
    switch (t) {
        case WEAPON_BOLT:  return "Orbe Magico";
        case WEAPON_KNIFE: return "Facas";
        case WEAPON_AXE:   return "Machado";
        case WEAPON_AURA:  return "Aura Sagrada";
        case WEAPON_WHIP:  return "Chicote";
        case WEAPON_BOLT_EVO:  return "Tempestade Arcana";
        case WEAPON_KNIFE_EVO: return "Mil Laminas";
        case WEAPON_AXE_EVO:   return "Machado Lunar";
        case WEAPON_AURA_EVO:  return "Coroa Sagrada";
        case WEAPON_WHIP_EVO:  return "Ceifa Sombria";
        default:           return "?";
    }
}

const char *weapon_desc(WeaponType t) {
    switch (t) {
        case WEAPON_BOLT:  return "Mira no inimigo mais proximo";
        case WEAPON_KNIFE: return "Dispara na direcao do movimento";
        case WEAPON_AXE:   return "Arco com dano alto, perfura";
        case WEAPON_AURA:  return "Dano continuo ao redor";
        case WEAPON_WHIP:  return "Golpe na direcao encarada";
        case WEAPON_BOLT_EVO:  return "Tempestade de orbes devastadora";
        case WEAPON_KNIFE_EVO: return "Chuva de laminas perfurantes";
        case WEAPON_AXE_EVO:   return "Machados lunares colossais";
        case WEAPON_AURA_EVO:  return "Aura divina imensa";
        case WEAPON_WHIP_EVO:  return "Ceifa sombria implacavel";
        default:           return "";
    }
}

uint32_t weapon_color(WeaponType t) {
    switch (t) {
        case WEAPON_BOLT:  return RGBA8(0x6E, 0xC6, 0xFF, 0xFF);
        case WEAPON_KNIFE: return RGBA8(0xE8, 0xE8, 0xF0, 0xFF);
        case WEAPON_AXE:   return RGBA8(0xF0, 0x8A, 0x3C, 0xFF);
        case WEAPON_AURA:  return RGBA8(0x6C, 0xE0, 0x82, 0xFF);
        case WEAPON_WHIP:  return RGBA8(0xF2, 0xC8, 0x4B, 0xFF);
        case WEAPON_BOLT_EVO:  return RGBA8(0xB0, 0xE8, 0xFF, 0xFF);
        case WEAPON_KNIFE_EVO: return RGBA8(0xFF, 0xFF, 0xFF, 0xFF);
        case WEAPON_AXE_EVO:   return RGBA8(0xFF, 0xC8, 0x6E, 0xFF);
        case WEAPON_AURA_EVO:  return RGBA8(0xB8, 0xFF, 0xC8, 0xFF);
        case WEAPON_WHIP_EVO:  return RGBA8(0xFF, 0xE0, 0x8C, 0xFF);
        default:           return COL_WHITE;
    }
}

WeaponType weapon_base(WeaponType t) {
    switch (t) {
        case WEAPON_BOLT_EVO:  return WEAPON_BOLT;
        case WEAPON_KNIFE_EVO: return WEAPON_KNIFE;
        case WEAPON_AXE_EVO:   return WEAPON_AXE;
        case WEAPON_AURA_EVO:  return WEAPON_AURA;
        case WEAPON_WHIP_EVO:  return WEAPON_WHIP;
        default:               return t;
    }
}

bool weapon_is_evo(WeaponType t) {
    return t >= WEAPON_BOLT_EVO && t < WEAPON_TYPE_COUNT;
}

/* =============================================================
 *  Stats por arma/nivel, ja aplicando os modificadores do jogador.
 * ============================================================= */
typedef struct {
    float damage;
    float cooldown;
    int   count;
    float speed;
    int   pierce;
    float radius;
    float range;
} WStat;

static WStat weapon_stats(const Player *p, const Weapon *w) {
    WStat s;
    memset(&s, 0, sizeof(s));
    int L = w->level;
    WeaponType bt = weapon_base(w->type);
    bool evo = (bt != w->type);

    switch (bt) {
        case WEAPON_BOLT:
            s.damage = 10.0f + L * 4.0f;
            s.cooldown = 1.10f;
            s.count = 1 + (L >= 3) + (L >= 6);
            s.speed = 330.0f;
            s.pierce = 1 + (L >= 5);
            s.radius = 7.0f;
            break;
        case WEAPON_KNIFE:
            s.damage = 7.0f + L * 3.0f;
            s.cooldown = 0.66f;
            s.count = 1 + L / 2;
            s.speed = 480.0f;
            s.pierce = 1 + (L >= 4);
            s.radius = 5.0f;
            break;
        case WEAPON_AXE:
            s.damage = 18.0f + L * 7.0f;
            s.cooldown = 1.55f;
            s.count = 1 + (L >= 4) + (L >= 7);
            s.speed = 300.0f;
            s.pierce = 3 + L;
            s.radius = 11.0f;
            break;
        case WEAPON_AURA:
            s.damage = 8.0f + L * 4.0f;   /* dano por segundo */
            s.cooldown = 0.0f;
            s.count = 1;
            s.radius = 50.0f + L * 8.0f;
            break;
        case WEAPON_WHIP:
            s.damage = 12.0f + L * 5.0f;
            s.cooldown = 0.85f;
            s.count = 1 + (L >= 3) + (L >= 6);
            s.range = 118.0f + L * 8.0f;
            s.radius = 26.0f;             /* meia-largura */
            break;
        default: break;
    }

    /* Bonus de evolucao (fusao): bem mais forte que a arma base. */
    if (evo) {
        s.damage   *= 2.1f;
        s.cooldown *= 0.78f;
        s.count    += 2;
        s.pierce   += 3;
        s.radius   *= 1.45f;
        s.range    *= 1.35f;
        s.speed    *= 1.10f;
    }

    s.damage   *= p->might;
    s.cooldown *= p->cooldown_mult;
    s.radius   *= p->area;
    s.range    *= p->area;
    s.speed    *= p->proj_speed;
    if (bt != WEAPON_AURA) s.count += p->amount_bonus;
    if (s.count < 1) s.count = 1;
    return s;
}

/* =============================================================
 *  Pool de projeteis
 * ============================================================= */
static Projectile *proj_alloc(Game *g) {
    for (int i = 0; i < MAX_PROJECTILES; ++i)
        if (!g->projectiles[i].active) return &g->projectiles[i];
    return NULL;
}

static Projectile *spawn_proj(Game *g, WeaponType src, Vec2 pos, Vec2 vel,
                              float dmg, float radius, int pierce, float life,
                              float grav, bool hits, uint32_t color) {
    Projectile *pr = proj_alloc(g);
    if (!pr) return NULL;
    memset(pr, 0, sizeof(*pr));
    pr->active = true;
    pr->src = src;
    pr->pos = pos;
    pr->vel = vel;
    pr->damage = dmg;
    pr->radius = radius;
    pr->pierce = pierce;
    pr->life = life;
    pr->maxlife = life;
    pr->grav = grav;
    pr->hits = hits;
    pr->color = color;
    return pr;
}

/* Dano instantaneo do chicote: caixa orientada ao longo de 'dir'. */
static void whip_hit(Game *g, Vec2 origin, Vec2 dir, float range, float thick, float dmg) {
    for (int e = 0; e < MAX_ENEMIES; ++e) {
        Enemy *en = &g->enemies[e];
        if (!en->active) continue;
        Vec2 rel = v2sub(en->pos, origin);
        float t = v2dot(rel, dir);
        if (t < -en->radius || t > range + en->radius) continue;
        float perp = fabsf(rel.x * (-dir.y) + rel.y * dir.x);
        if (perp <= thick + en->radius)
            enemy_hurt(g, e, dmg);
    }
}

/* =============================================================
 *  Atualizacao das armas (disparo automatico)
 * ============================================================= */
void weapons_update(Game *g, float dt) {
    Player *p = &g->player;
    bool fired = false;

    for (int i = 0; i < p->weapon_count; ++i) {
        Weapon *w = &p->weapons[i];
        WStat st = weapon_stats(p, w);
        WeaponType bt = weapon_base(w->type);
        uint32_t col = weapon_color(w->type);

        if (bt == WEAPON_AURA) {
            float r = st.radius;
            for (int e = 0; e < MAX_ENEMIES; ++e) {
                Enemy *en = &g->enemies[e];
                if (!en->active) continue;
                float rr = r + en->radius;
                if (v2dist2(en->pos, p->pos) <= rr * rr)
                    enemy_hurt(g, e, st.damage * dt);
            }
            continue;
        }

        w->cooldown -= dt;
        if (w->cooldown > 0.0f) continue;
        w->cooldown += (st.cooldown > 0.05f ? st.cooldown : 0.05f);
        fired = true;

        switch (bt) {
            case WEAPON_BOLT: {
                int tgt = enemy_nearest(g, p->pos, 720.0f);
                Vec2 dir = (tgt >= 0) ? v2norm(v2sub(g->enemies[tgt].pos, p->pos))
                                      : rng_dir(&g->rng);
                float base = atan2f(dir.y, dir.x);
                for (int k = 0; k < st.count; ++k) {
                    float ang = base + (k - (st.count - 1) * 0.5f) * 0.20f;
                    Vec2 vel = v2scale(v2(cosf(ang), sinf(ang)), st.speed);
                    spawn_proj(g, WEAPON_BOLT, p->pos, vel, st.damage, st.radius,
                               st.pierce, 1.7f, 0.0f, true, col);
                }
            } break;

            case WEAPON_KNIFE: {
                Vec2 base = (v2len2(p->facing) > 0.01f) ? p->facing : v2(1.0f, 0.0f);
                float bang = atan2f(base.y, base.x);
                for (int k = 0; k < st.count; ++k) {
                    float ang = bang + (k - (st.count - 1) * 0.5f) * 0.12f;
                    Vec2 vel = v2scale(v2(cosf(ang), sinf(ang)), st.speed);
                    spawn_proj(g, WEAPON_KNIFE, p->pos, vel, st.damage, st.radius,
                               st.pierce, 1.0f, 0.0f, true, col);
                }
            } break;

            case WEAPON_AXE: {
                for (int k = 0; k < st.count; ++k) {
                    float vx = rng_range(&g->rng, -80.0f, 80.0f)
                             + (k - (st.count - 1) * 0.5f) * 70.0f;
                    Vec2 vel = v2(vx, -st.speed);
                    spawn_proj(g, WEAPON_AXE, p->pos, vel, st.damage, st.radius,
                               st.pierce, 2.2f, 540.0f, true, col);
                }
            } break;

            case WEAPON_WHIP: {
                Vec2 base = (v2len2(p->facing) > 0.01f) ? p->facing : v2(1.0f, 0.0f);
                for (int k = 0; k < st.count; ++k) {
                    Vec2 dir = (k % 2 == 0) ? base : v2scale(base, -1.0f);
                    whip_hit(g, p->pos, dir, st.range, st.radius, st.damage);
                    Projectile *pr = spawn_proj(g, WEAPON_WHIP, p->pos, v2(0, 0),
                                                0.0f, st.radius, 0, 0.16f, 0.0f,
                                                false, col);
                    if (pr) pr->aux = v2scale(dir, st.range);
                }
            } break;

            default: break;
        }
    }

    if (fired) audio_play_sfx(SFX_SHOOT);
}

/* =============================================================
 *  Atualizacao / colisao dos projeteis
 * ============================================================= */
void projectiles_update(Game *g, float dt) {
    /* otimizacao: so varremos projeteis inimigos se houver algum destrutivel ativo */
    bool any_ebullet = false;
    for (int e = 0; e < MAX_EPROJECTILES; ++e)
        if (g->eprojectiles[e].active && g->eprojectiles[e].destructible) { any_ebullet = true; break; }

    for (int i = 0; i < MAX_PROJECTILES; ++i) {
        Projectile *pr = &g->projectiles[i];
        if (!pr->active) continue;

        pr->life -= dt;
        if (pr->life <= 0.0f) { pr->active = false; continue; }

        pr->spin += dt * 12.0f;
        if (pr->grav > 0.0f) pr->vel.y += pr->grav * dt;
        pr->pos = v2add(pr->pos, v2scale(pr->vel, dt));
        if (pr->hit_cd > 0.0f) pr->hit_cd -= dt;

        if (!pr->hits) continue;

        /* tiros do jogador destroem projeteis de inimigos comuns (bosses sao imunes) */
        if (any_ebullet) {
            for (int e = 0; e < MAX_EPROJECTILES; ++e) {
                EnemyProjectile *ep = &g->eprojectiles[e];
                if (!ep->active || !ep->destructible) continue;
                float rr2 = pr->radius + ep->radius;
                if (v2dist2(pr->pos, ep->pos) <= rr2 * rr2) {
                    ep->active = false;
                    fx_burst(g, ep->pos, ep->color, 3, 70.0f);
                }
            }
        }

        if (pr->pierce <= 0) { pr->active = false; continue; }
        if (pr->hit_cd > 0.0f) continue;

        for (int e = 0; e < MAX_ENEMIES; ++e) {
            Enemy *en = &g->enemies[e];
            if (!en->active) continue;
            float rr = pr->radius + en->radius;
            if (v2dist2(pr->pos, en->pos) <= rr * rr) {
                enemy_hurt(g, e, pr->damage);
                pr->pierce--;
                pr->hit_cd = 0.09f;
                if (pr->pierce <= 0) pr->active = false;
                break;
            }
        }
    }
}

void projectiles_draw(Game *g) {
    for (int i = 0; i < MAX_PROJECTILES; ++i) {
        Projectile *pr = &g->projectiles[i];
        if (!pr->active) continue;
        Vec2 s = v2sub(pr->pos, g->camera);
        if (s.x < -48.0f || s.x > SCREEN_W + 48.0f ||
            s.y < -48.0f || s.y > SCREEN_H + 48.0f) continue;   /* cull fora da tela */

        switch (pr->src) {
            case WEAPON_WHIP: {
                Vec2 ep = v2sub(v2add(pr->pos, pr->aux), g->camera);
                unsigned char a = (unsigned char)(150.0f * (pr->life / pr->maxlife));
                for (int k = 0; k <= 4; ++k) {
                    float t = (float)k / 4.0f;
                    float x = lerpf(s.x, ep.x, t);
                    float y = lerpf(s.y, ep.y, t);
                    draw_circle(x, y, pr->radius, col_a(pr->color, a));
                }
            } break;
            case WEAPON_AXE:
                draw_circle(s.x, s.y, pr->radius, pr->color);
                draw_circle(s.x, s.y, pr->radius * 0.5f, COL_WHITE);
                break;
            case WEAPON_KNIFE:
                draw_circle(s.x, s.y, pr->radius, pr->color);
                break;
            default: /* BOLT */
                draw_circle(s.x, s.y, pr->radius + 2.0f, col_a(pr->color, 90));
                draw_circle(s.x, s.y, pr->radius, pr->color);
                draw_circle(s.x, s.y, pr->radius * 0.5f, COL_WHITE);
                break;
        }
    }
}

void weapons_draw(Game *g) {
    Player *p = &g->player;
    Vec2 c = v2sub(p->pos, g->camera);
    for (int i = 0; i < p->weapon_count; ++i) {
        Weapon *w = &p->weapons[i];
        if (weapon_base(w->type) != WEAPON_AURA) continue;
        WStat st = weapon_stats(p, w);
        float pulse = 1.0f + 0.04f * sinf(g->global_time * 4.0f);
        float r = st.radius * pulse;
        uint32_t col = weapon_color(w->type);
        draw_circle(c.x, c.y, r, col_a(col, 0x24));
        draw_ring(c.x, c.y, r, 2.0f, col_a(col, 0x70));
    }
}

/* =============================================================
 *  Inventario de armas
 * ============================================================= */
int weapon_index(Game *g, WeaponType t) {
    for (int i = 0; i < g->player.weapon_count; ++i)
        if (g->player.weapons[i].type == t) return i;
    return -1;
}

bool weapon_is_maxed(Game *g, WeaponType t) {
    int i = weapon_index(g, t);
    if (i < 0) return false;
    return g->player.weapons[i].level >= WEAPON_MAX_LEVEL;
}

bool weapon_unlocked(Game *g, WeaponType t) {
    /* Evolucoes nunca aparecem no pool normal (vem por fusao). */
    if (weapon_is_evo(t)) return false;
    /* Orbe e Facas estao sempre disponiveis; o resto e desbloqueado jogando. */
    if (t == WEAPON_BOLT || t == WEAPON_KNIFE) return true;
    return (g->save.weapon_unlocks & (1u << t)) != 0;
}

bool weapon_grant(Game *g, WeaponType t) {
    int i = weapon_index(g, t);
    if (i >= 0) {
        if (g->player.weapons[i].level < WEAPON_MAX_LEVEL) {
            g->player.weapons[i].level++;
            return true;
        }
        return false;
    }
    if (g->player.weapon_count >= MAX_WEAPONS) return false;
    Weapon *w = &g->player.weapons[g->player.weapon_count++];
    w->type = t;
    w->level = 1;
    w->cooldown = 0.25f;
    w->tick = 0.0f;
    return true;
}
