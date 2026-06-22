#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/* =============================================================
 *  Tipos base: vetor 2D, utilitarios matematicos e RNG.
 * ============================================================= */

typedef struct { float x, y; } Vec2;

static inline Vec2 v2(float x, float y) { Vec2 r = { x, y }; return r; }
static inline Vec2 v2add(Vec2 a, Vec2 b) { return v2(a.x + b.x, a.y + b.y); }
static inline Vec2 v2sub(Vec2 a, Vec2 b) { return v2(a.x - b.x, a.y - b.y); }
static inline Vec2 v2scale(Vec2 a, float s) { return v2(a.x * s, a.y * s); }
static inline float v2dot(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }
static inline float v2len2(Vec2 a) { return a.x * a.x + a.y * a.y; }
static inline float v2len(Vec2 a) { return sqrtf(a.x * a.x + a.y * a.y); }
static inline float v2dist2(Vec2 a, Vec2 b) { return v2len2(v2sub(a, b)); }
static inline Vec2 v2norm(Vec2 a) {
    float l = v2len(a);
    if (l < 1e-6f) return v2(0.0f, 0.0f);
    return v2(a.x / l, a.y / l);
}

static inline float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
static inline float maxf(float a, float b) { return a > b ? a : b; }
static inline float minf(float a, float b) { return a < b ? a : b; }
static inline int   clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
static inline int   maxi(int a, int b) { return a > b ? a : b; }
static inline int   mini(int a, int b) { return a < b ? a : b; }
static inline float lerpf(float a, float b, float t) { return a + (b - a) * t; }

/* RNG xorshift32 — deterministico e barato, ideal para o Vita. */
typedef struct { uint32_t s; } Rng;

void     rng_seed(Rng *r, uint32_t seed);
uint32_t rng_u32(Rng *r);
float    rng_f01(Rng *r);                    /* [0,1) */
float    rng_range(Rng *r, float lo, float hi);
int      rng_int(Rng *r, int lo, int hi);    /* inclusivo [lo,hi] */
Vec2     rng_dir(Rng *r);                    /* vetor unitario aleatorio */

#endif /* TYPES_H */
