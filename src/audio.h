#ifndef AUDIO_H
#define AUDIO_H

#include "types.h"

/* Efeitos sonoros (sintetizados proceduralmente). */
typedef enum {
    SFX_SELECT,
    SFX_MOVE,
    SFX_SHOOT,
    SFX_HIT,
    SFX_XP,
    SFX_LEVELUP,
    SFX_HURT,
    SFX_KILL,
    SFX_FUSION,
    SFX_BOSS,
    SFX_BOMB,
    SFX_COUNT
} SfxId;

/* Trilhas de musica. */
typedef enum { MUS_NONE, MUS_MENU, MUS_GAME, MUS_BOSS } MusicId;

bool audio_init(void);            /* abre a porta e inicia a thread; false se falhar (jogo segue mudo) */
void audio_fini(void);            /* para a thread e libera a porta */
void audio_play_sfx(SfxId id);
void audio_set_music(MusicId id);
void audio_set_enabled(bool on);

#endif /* AUDIO_H */
