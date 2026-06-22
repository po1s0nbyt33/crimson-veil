#ifndef MISSION_H
#define MISSION_H

#include "types.h"

typedef struct Game Game;

typedef enum {
    MT_KILLS,          /* abates totais (lifetime) >= alvo */
    MT_SURVIVE,        /* melhor tempo de sobrevivencia >= alvo (s) */
    MT_BOSS,           /* derrotar o chefe de id 'target' */
    MT_UNLOCK_WEAPON,  /* desbloquear a arma 'target' */
    MT_RUNS,           /* numero de partidas >= alvo */
    MT_FUSION          /* descobrir qualquer fusao */
} MissionType;

typedef enum {
    RWD_COINS,
    RWD_CHARACTER,     /* desbloqueia personagem reward_value */
    RWD_WEAPON         /* desbloqueia arma reward_value */
} MissionReward;

typedef enum { MS_LOCKED, MS_AVAILABLE, MS_COMPLETE, MS_CLAIMED } MissionState;

typedef struct {
    const char   *name;
    const char   *desc;
    MissionType   type;
    int           target;
    MissionReward reward;
    int           reward_value;
    int           prereq;   /* -1, ou indice da missao que precisa estar resgatada antes */
} MissionDef;

int               mission_count(void);
const MissionDef *mission_get(int i);
MissionState      mission_state(Game *g, int i);
int               mission_progress_value(Game *g, int i);  /* progresso atual */
bool              mission_claim(Game *g, int i);           /* resgata recompensa (true se ok) */
const char       *mission_reward_text(int i);

#endif /* MISSION_H */
