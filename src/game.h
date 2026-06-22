#ifndef GAME_H
#define GAME_H

#include "types.h"
#include "config.h"
#include "input.h"
#include "render.h"
#include "weapon.h"
#include "sprite.h"
#include "player.h"
#include "enemy.h"
#include "boss.h"
#include "pickup.h"
#include "wave.h"
#include "fx.h"
#include "upgrade.h"
#include "save.h"
#include "character.h"
#include "fusion.h"
#include "mission.h"
#include "collection.h"
#include "audio.h"
#include "menu.h"

/* Estados da maquina de jogo. */
typedef enum {
    GS_MENU,       /* menu inicial */
    GS_CHARSELECT, /* selecao/loja de personagens */
    GS_SHOP,       /* loja de melhorias permanentes */
    GS_SETTINGS,   /* configuracoes */
    GS_COLLECTION, /* colecao */
    GS_MISSIONS,   /* missoes */
    GS_PLAYING,    /* simulacao ativa */
    GS_LEVELUP,    /* escolha de upgrade (pausa a simulacao) */
    GS_PAUSE,      /* menu de pausa */
    GS_GAMEOVER    /* fim de partida */
} GameState;

struct Game {
    GameState state;
    float     state_time;
    bool      should_quit;

    Rng   rng;
    Input input;

    /* ---- mundo ---- */
    Player          player;
    Enemy           enemies[MAX_ENEMIES];
    Projectile      projectiles[MAX_PROJECTILES];
    EnemyProjectile eprojectiles[MAX_EPROJECTILES];
    Pickup          pickups[MAX_PICKUPS];
    Floater         floaters[MAX_FLOATERS];
    Particle        particles[MAX_PARTICLES];
    WaveDirector    wave;
    Vec2         camera;
    int          boss_idx;     /* indice do boss ativo, ou -1 */
    uint32_t     run_coins;    /* moedas coletadas nesta partida */
    uint32_t     run_gained;   /* total creditado ao fim da ultima partida */
    float        shake;        /* magnitude do tremor de tela */
    float        flash;        /* flash branco/vermelho de tela */

    /* ---- progressao persistente ---- */
    SaveData save;

    /* ---- UI ---- */
    int  sel_main;
    int  sel_shop;
    int  sel_settings;
    int  sel_pause;
    int  sel_gameover;
    int  sel_char;
    int  sel_collection;
    int  col_scroll;
    int  sel_mission;
    int  miss_scroll;
    bool confirm_reset;

    UpgradeChoice choices[4];
    int           choice_count;
    int           choice_sel;

    bool  show_fps;
    bool  audio_on;
    float fps;
    float global_time;
};

void game_init(Game *g, uint32_t seed);
void game_new_run(Game *g);
void game_continue_run(Game *g);
void game_save_run(Game *g);
void game_end_run(Game *g);
void game_open_levelup(Game *g);
void game_update(Game *g, float dt);
void game_draw(Game *g);
void game_set_state(Game *g, GameState s);
void game_world_clear_transient(Game *g);

#endif /* GAME_H */
