#include "game.h"
#include <stdio.h>

/* name, desc, type, target, reward, reward_value, prereq */
static const MissionDef MISSIONS[] = {
    { "Primeiro Sangue",   "Derrote 100 inimigos.",              MT_KILLS,         100,           RWD_COINS,     100, -1 },
    { "Cacador de Chefes", "Derrote o primeiro chefe.",          MT_BOSS,            0,           RWD_WEAPON,    WEAPON_AXE, -1 },
    { "Sobrevivente",      "Sobreviva por 3 minutos.",           MT_SURVIVE,       180,           RWD_COINS,     200,  0 },
    { "Arsenal Arcano",    "Desbloqueie a Aura Sagrada.",        MT_UNLOCK_WEAPON, WEAPON_AURA,   RWD_COINS,     300, -1 },
    { "Exterminador",      "Derrote 1000 inimigos no total.",    MT_KILLS,        1000,           RWD_CHARACTER,   2,  0 },
    { "Maratonista",       "Sobreviva por 5 minutos.",           MT_SURVIVE,       300,           RWD_COINS,     400,  2 },
    { "Alquimista",        "Descubra qualquer fusao de arma.",   MT_FUSION,          0,           RWD_COINS,     500, -1 },
    { "Lenda Viva",        "Derrote o Ceifador Sangrento.",      MT_BOSS,            2,           RWD_CHARACTER,   3,  1 },
    { "Veterano",          "Jogue 10 partidas.",                 MT_RUNS,           10,           RWD_COINS,     250, -1 },
};
#define NMISS ((int)(sizeof(MISSIONS) / sizeof(MISSIONS[0])))

int mission_count(void) { return NMISS; }

const MissionDef *mission_get(int i) {
    if (i < 0 || i >= NMISS) return &MISSIONS[0];
    return &MISSIONS[i];
}

static bool mission_complete(Game *g, int i) {
    const MissionDef *m = &MISSIONS[i];
    SaveData *s = &g->save;
    switch (m->type) {
        case MT_KILLS:         return s->lifetime_kills >= (uint32_t)m->target;
        case MT_SURVIVE:       return s->best_time >= (uint32_t)m->target;
        case MT_BOSS:          return (s->bosses_defeated & (1u << m->target)) != 0;
        case MT_UNLOCK_WEAPON: return (s->weapon_unlocks & (1u << m->target)) != 0;
        case MT_RUNS:          return s->runs >= (uint32_t)m->target;
        case MT_FUSION:        return s->fusion_unlocks != 0;
    }
    return false;
}

MissionState mission_state(Game *g, int i) {
    if (i < 0 || i >= NMISS) return MS_LOCKED;
    if (g->save.mission_claimed[i]) return MS_CLAIMED;
    int pr = MISSIONS[i].prereq;
    if (pr >= 0 && pr < NMISS && !g->save.mission_claimed[pr]) return MS_LOCKED;
    if (mission_complete(g, i)) return MS_COMPLETE;
    return MS_AVAILABLE;
}

int mission_progress_value(Game *g, int i) {
    const MissionDef *m = &MISSIONS[i];
    SaveData *s = &g->save;
    switch (m->type) {
        case MT_KILLS:         return (int)s->lifetime_kills;
        case MT_SURVIVE:       return (int)s->best_time;
        case MT_RUNS:          return (int)s->runs;
        case MT_BOSS:          return (s->bosses_defeated & (1u << m->target)) ? 1 : 0;
        case MT_UNLOCK_WEAPON: return (s->weapon_unlocks & (1u << m->target)) ? 1 : 0;
        case MT_FUSION:        return s->fusion_unlocks ? 1 : 0;
    }
    return 0;
}

bool mission_claim(Game *g, int i) {
    if (mission_state(g, i) != MS_COMPLETE) return false;
    const MissionDef *m = &MISSIONS[i];
    switch (m->reward) {
        case RWD_COINS:     g->save.coins += (uint32_t)m->reward_value; break;
        case RWD_CHARACTER: g->save.char_unlocks |= (1u << m->reward_value); break;
        case RWD_WEAPON:    g->save.weapon_unlocks |= (1u << m->reward_value); break;
    }
    g->save.mission_claimed[i] = 1;
    save_write(&g->save);
    return true;
}

const char *mission_reward_text(int i) {
    static char buf[48];
    const MissionDef *m = mission_get(i);
    switch (m->reward) {
        case RWD_COINS:     snprintf(buf, sizeof(buf), "Recompensa: %d moedas", m->reward_value); break;
        case RWD_CHARACTER: snprintf(buf, sizeof(buf), "Recompensa: %s", character_get(m->reward_value)->name); break;
        case RWD_WEAPON:    snprintf(buf, sizeof(buf), "Recompensa: %s", weapon_name((WeaponType)m->reward_value)); break;
        default:            buf[0] = '\0'; break;
    }
    return buf;
}
