#include "types.h"

void rng_seed(Rng *r, uint32_t seed) {
    r->s = seed ? seed : 0xC0FFEEu;
}

uint32_t rng_u32(Rng *r) {
    uint32_t x = r->s;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    r->s = x ? x : 0x1234567u;
    return r->s;
}

float rng_f01(Rng *r) {
    /* 24 bits de mantissa -> [0,1) */
    return (float)(rng_u32(r) >> 8) * (1.0f / 16777216.0f);
}

float rng_range(Rng *r, float lo, float hi) {
    return lo + (hi - lo) * rng_f01(r);
}

int rng_int(Rng *r, int lo, int hi) {
    if (hi <= lo) return lo;
    return lo + (int)(rng_u32(r) % (uint32_t)(hi - lo + 1));
}

Vec2 rng_dir(Rng *r) {
    float a = rng_f01(r) * 6.28318530718f;
    return v2(cosf(a), sinf(a));
}
