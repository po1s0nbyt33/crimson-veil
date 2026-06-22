#include "sprite.h"
#include <math.h>

typedef struct { vita2d_texture *tex; int w, h; } Spr;
static Spr s_spr[SPR_COUNT];

static uint32_t shade(uint32_t c, float f) {
    int r = (c) & 0xFF, g = (c >> 8) & 0xFF, b = (c >> 16) & 0xFF, a = (c >> 24) & 0xFF;
    r = clampi((int)(r * f), 0, 255);
    g = clampi((int)(g * f), 0, 255);
    b = clampi((int)(b * f), 0, 255);
    return RGBA8(r, g, b, a);
}

static uint8_t s_grid[24][24];

static int gridget(int W, int H, int x, int y) {
    if (x < 0 || y < 0 || x >= W || y >= H) return 0;
    return s_grid[y][x];
}

/* Gera uma criatura simetrica (espelhada na horizontal). */
static void gen_creature(Spr *sp, int W, int H, uint32_t seed,
                         uint32_t body, uint32_t outline, uint32_t eye, bool horns) {
    vita2d_texture *t = vita2d_create_empty_texture(W, H);
    vita2d_texture_set_filters(t, SCE_GXM_TEXTURE_FILTER_POINT, SCE_GXM_TEXTURE_FILTER_POINT);
    uint32_t *data = (uint32_t *)vita2d_texture_get_datap(t);
    int stride = (int)vita2d_texture_get_stride(t) / 4;

    Rng r; rng_seed(&r, seed);
    int half = W / 2;

    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < half; ++x) {
            float cx = (float)x / (float)half;                          /* 0 borda .. 1 centro */
            float cy = 1.0f - fabsf((y - H * 0.5f) / (H * 0.5f));        /* 1 centro .. 0 topo/base */
            float prob = 0.14f + 0.55f * cx + 0.32f * cy;
            int solid = (rng_f01(&r) < prob) ? 1 : 0;
            s_grid[y][x] = (uint8_t)solid;
            s_grid[y][W - 1 - x] = (uint8_t)solid;                       /* espelha */
        }
    }
    /* nucleo garantido para nunca sair vazio */
    for (int y = (int)(H * 0.28f); y < (int)(H * 0.80f); ++y) {
        s_grid[y][half - 1] = 1;
        s_grid[y][half] = 1;
    }

    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            uint32_t px = 0;
            if (s_grid[y][x]) {
                int edge = !gridget(W, H, x - 1, y) || !gridget(W, H, x + 1, y) ||
                           !gridget(W, H, x, y - 1) || !gridget(W, H, x, y + 1);
                if (edge) px = outline;
                else      px = shade(body, 1.12f - 0.45f * ((float)y / (float)H));
            }
            data[y * stride + x] = px;
        }
    }

    /* olhos */
    int ey = (int)(H * 0.36f);
    int ex1 = half - 2, ex2 = W - 1 - (half - 2);
    if (ey >= 0 && ey < H) {
        if (ex1 >= 0)     data[ey * stride + ex1] = eye;
        if (ex2 < W)      data[ey * stride + ex2] = eye;
    }
    /* chifres do boss */
    if (horns) {
        if (half - 3 >= 0) data[0 * stride + (half - 3)] = eye;
        if (half + 2 < W)  data[0 * stride + (half + 2)] = eye;
    }

    sp->tex = t; sp->w = W; sp->h = H;
}

/* Gera um emblema/gema colorido para icone de arma. */
static void gen_gem(Spr *sp, int S, uint32_t color) {
    vita2d_texture *t = vita2d_create_empty_texture(S, S);
    vita2d_texture_set_filters(t, SCE_GXM_TEXTURE_FILTER_POINT, SCE_GXM_TEXTURE_FILTER_POINT);
    uint32_t *data = (uint32_t *)vita2d_texture_get_datap(t);
    int stride = (int)vita2d_texture_get_stride(t) / 4;

    float c = (S - 1) * 0.5f, rad = c + 0.6f;
    for (int y = 0; y < S; ++y) {
        for (int x = 0; x < S; ++x) {
            float d = fabsf(x - c) + fabsf(y - c);   /* losango */
            uint32_t px = 0;
            if (d <= rad) {
                if (d > rad - 1.4f)              px = shade(color, 0.5f);   /* contorno */
                else if (x < c && y < c && d < rad * 0.5f) px = shade(color, 1.6f); /* brilho */
                else                              px = color;
            }
            data[y * stride + x] = px;
        }
    }
    sp->tex = t; sp->w = S; sp->h = S;
}

void sprite_init(void) {
    /* personagens */
    gen_creature(&s_spr[SPR_CHAR0], 16, 16, 0x1001, RGBA8(0x5C,0xD6,0x6B,0xFF), RGBA8(0x1E,0x5A,0x2C,0xFF), RGBA8(0xFF,0xFF,0xFF,0xFF), false);
    gen_creature(&s_spr[SPR_CHAR1], 16, 16, 0x1002, RGBA8(0x9C,0x6C,0xF0,0xFF), RGBA8(0x40,0x26,0x70,0xFF), RGBA8(0x7A,0xF0,0xFF,0xFF), false);
    gen_creature(&s_spr[SPR_CHAR2], 16, 16, 0x1003, RGBA8(0xE0,0x9A,0x4C,0xFF), RGBA8(0x6A,0x40,0x18,0xFF), RGBA8(0xFF,0xFF,0xFF,0xFF), false);
    gen_creature(&s_spr[SPR_CHAR3], 16, 16, 0x1004, RGBA8(0xF2,0xC8,0x4B,0xFF), RGBA8(0x80,0x60,0x10,0xFF), RGBA8(0xFF,0xFF,0xFF,0xFF), false);

    /* inimigos comuns */
    gen_creature(&s_spr[SPR_E_SWARM],   14, 14, 0x2001, RGBA8(0xB0,0x6C,0xE0,0xFF), RGBA8(0x4A,0x28,0x66,0xFF), RGBA8(0xFF,0x60,0x60,0xFF), false);
    gen_creature(&s_spr[SPR_E_RUNNER],  14, 14, 0x2002, RGBA8(0xE8,0x5B,0x5B,0xFF), RGBA8(0x6A,0x20,0x20,0xFF), RGBA8(0xFF,0xE0,0x40,0xFF), false);
    gen_creature(&s_spr[SPR_E_BRUTE],   18, 18, 0x2003, RGBA8(0xC8,0x8A,0x4C,0xFF), RGBA8(0x5A,0x36,0x18,0xFF), RGBA8(0xFF,0x50,0x50,0xFF), true);
    gen_creature(&s_spr[SPR_E_SHOOTER], 14, 14, 0x2004, RGBA8(0x4F,0xC6,0xC0,0xFF), RGBA8(0x1C,0x55,0x52,0xFF), RGBA8(0xFF,0xFF,0xFF,0xFF), false);
    gen_creature(&s_spr[SPR_E_MAGE],    14, 14, 0x2005, RGBA8(0xC0,0x7C,0xF0,0xFF), RGBA8(0x50,0x2C,0x72,0xFF), RGBA8(0x80,0xFF,0xF0,0xFF), false);

    /* bosses (maiores, olhos vermelhos, com chifres) */
    gen_creature(&s_spr[SPR_BOSS0], 20, 20, 0x3001, RGBA8(0xE0,0x3C,0x6C,0xFF), RGBA8(0x60,0x14,0x2C,0xFF), RGBA8(0xFF,0x30,0x30,0xFF), true);
    gen_creature(&s_spr[SPR_BOSS1], 20, 20, 0x3002, RGBA8(0x8C,0x3C,0xE0,0xFF), RGBA8(0x38,0x14,0x60,0xFF), RGBA8(0xFF,0x30,0x30,0xFF), true);
    gen_creature(&s_spr[SPR_BOSS2], 20, 20, 0x3003, RGBA8(0xE0,0x5C,0x2C,0xFF), RGBA8(0x60,0x24,0x0C,0xFF), RGBA8(0xFF,0x30,0x30,0xFF), true);
    gen_creature(&s_spr[SPR_BOSS3], 20, 20, 0x3004, RGBA8(0xF0,0x6A,0x2C,0xFF), RGBA8(0x64,0x28,0x0C,0xFF), RGBA8(0xFF,0xE0,0x30,0xFF), true);
    gen_creature(&s_spr[SPR_BOSS4], 20, 20, 0x3005, RGBA8(0x3C,0x9C,0xE0,0xFF), RGBA8(0x12,0x40,0x60,0xFF), RGBA8(0xFF,0x30,0x30,0xFF), true);

    /* icones de armas (gemas coloridas) */
    for (int t = 0; t < WEAPON_TYPE_COUNT; ++t)
        gen_gem(&s_spr[SPR_WEAPON_FIRST + t], 14, weapon_color((WeaponType)t));
}

void sprite_fini(void) {
    for (int i = 0; i < SPR_COUNT; ++i) {
        if (s_spr[i].tex) { vita2d_free_texture(s_spr[i].tex); s_spr[i].tex = NULL; }
    }
}

void sprite_draw(SpriteId id, float cx, float cy, float target_px) {
    if (id < 0 || id >= SPR_COUNT) return;
    Spr *s = &s_spr[id];
    if (!s->tex) return;
    float sc = target_px / (float)s->w;
    vita2d_draw_texture_scale(s->tex, cx - s->w * sc * 0.5f, cy - s->h * sc * 0.5f, sc, sc);
}

SpriteId sprite_for_char(int id) {
    if (id < 0 || id > 3) id = 0;
    return (SpriteId)(SPR_CHAR0 + id);
}

SpriteId sprite_for_enemy(int type) {
    switch (type) {
        case 0: return SPR_E_SWARM;
        case 1: return SPR_E_RUNNER;
        case 2: return SPR_E_BRUTE;
        case 3: return SPR_E_SHOOTER;
        case 4: return SPR_E_MAGE;
        default: return SPR_E_SWARM;
    }
}

SpriteId sprite_for_boss(int id) {
    if (id < 0 || id > 4) id = 0;
    return (SpriteId)(SPR_BOSS0 + id);
}

SpriteId sprite_for_weapon(WeaponType t) {
    if (t < 0 || t >= WEAPON_TYPE_COUNT) t = WEAPON_BOLT;
    return (SpriteId)(SPR_WEAPON_FIRST + t);
}
