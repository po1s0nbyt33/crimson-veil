#ifndef SAVE_H
#define SAVE_H

#include "types.h"
#include "config.h"
#include "player.h"
#include "wave.h"

/* Melhorias permanentes compradas entre partidas (progressao meta). */
typedef enum {
    META_MAXHP,
    META_MIGHT,
    META_SPEED,
    META_ARMOR,
    META_REGEN,
    META_GROWTH,    /* ganho de XP */
    META_GREED,     /* ganho de moedas */
    META_PICKUP,    /* raio de coleta */
    META_AMOUNT,    /* projeteis extras iniciais */
    META_COOLDOWN,  /* recarga */
    META_COUNT
} MetaType;

#define META_MAX_LEVEL 5

/* Snapshot completo de uma partida em andamento (permite "Continuar").
 * Player e WaveDirector sao POD (sem ponteiros), entao serializam direto. */
typedef struct {
    uint32_t     version;
    Player       player;
    WaveDirector wave;
    uint32_t     run_coins;
    float        cam_x, cam_y;
} RunSnapshot;

typedef struct {
    uint32_t magic;
    uint32_t version;

    /* progressao permanente */
    uint8_t  meta[META_COUNT];
    uint32_t coins;
    uint32_t weapon_unlocks;   /* bitmask por WeaponType base */
    uint32_t fusion_unlocks;   /* bitmask por indice de fusao descoberta */
    uint32_t char_unlocks;     /* bitmask por personagem desbloqueado */
    uint32_t passive_seen;     /* bitmask por PassiveType ja obtida (colecao) */
    uint32_t bosses_defeated;  /* bitmask por boss_id derrotado */

    /* estatisticas */
    uint32_t lifetime_kills;
    uint32_t best_time;        /* melhor tempo de sobrevivencia (s) */
    uint32_t total_kills;
    uint32_t runs;

    /* selecoes / configuracoes */
    uint8_t  selected_char;
    uint8_t  opt_shake;
    uint8_t  opt_damage_numbers;
    uint8_t  opt_show_fps;

    /* missoes (recompensa resgatada) */
    uint8_t  mission_claimed[MAX_MISSIONS];

    /* partida em andamento */
    uint8_t  has_run;
    uint8_t  _pad[3];
    RunSnapshot run;

    uint32_t checksum;
} SaveData;

const char *meta_name(MetaType m);
const char *meta_desc(MetaType m);
int         meta_cost(const SaveData *s, MetaType m);  /* custo do proximo nivel; 0 se no maximo */

void save_defaults(SaveData *s);
bool save_load(SaveData *s);     /* false se ausente/corrompido (mantem defaults); migra v2 */
bool save_write(const SaveData *s);
void save_wipe(SaveData *s);     /* zera progresso e grava */

#endif /* SAVE_H */
