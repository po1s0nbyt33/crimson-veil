#ifndef CONFIG_H
#define CONFIG_H

/* =============================================================
 *  Constantes globais de balanceamento e limites de memoria.
 *  Centralizadas aqui para facilitar o ajuste fino (tuning).
 * ============================================================= */

/* Resolucao nativa do PS Vita */
#define SCREEN_W 960
#define SCREEN_H 544

/* Tamanho dos pools de entidades (alocacao estatica, sem malloc) */
#define MAX_ENEMIES      320
#define MAX_PROJECTILES  256
#define MAX_EPROJECTILES 192     /* projeteis inimigos (ranged/mago/boss) */
#define MAX_PICKUPS      512
#define MAX_FLOATERS     96
#define MAX_PARTICLES    192
#define MAX_WEAPONS      8

/* Limites dos sistemas de progressao (defs reais ficam nos respectivos .c) */
#define MAX_CHARACTERS   8
#define MAX_MISSIONS     32
#define MAX_FUSIONS      8
#define MAX_COLLECTION   64

/* Atributos base do jogador */
#define PLAYER_BASE_HP      100.0f
#define PLAYER_BASE_SPEED   135.0f   /* px/s */
#define PLAYER_RADIUS       11.0f
#define PLAYER_IFRAME_TIME  0.45f    /* invulnerabilidade apos tomar dano (s) */
#define PICKUP_BASE_RADIUS  48.0f
#define PLAYER_COLLECT_DIST 16.0f    /* distancia para coletar um drop */

/* Mundo / camera / spawn */
#define SPAWN_RING_MIN   560.0f      /* raio onde inimigos aparecem ao redor do jogador */
#define SPAWN_RING_MAX   660.0f
#define DESPAWN_DIST     1150.0f     /* inimigos alem disso sao reciclados */

/* Experiencia / nivel */
#define XP_BASE          5
#define XP_PER_LEVEL     8

/* Tempo */
#define MAX_DT           0.05f       /* clamp de dt para evitar saltos */
#define BOSS_INTERVAL    120         /* um boss a cada 120 s (2 min) */

/* Save */
#define SAVE_DIR    "ux0:data/CrimsonVeil"
#define SAVE_PATH   "ux0:data/CrimsonVeil/save.bin"
#define SAVE_MAGIC  0x56535631u      /* "VSV1" */
#define SAVE_VERSION 3               /* v3: personagens, missoes, fusoes, colecao */

#endif /* CONFIG_H */
