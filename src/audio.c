#include "audio.h"
#include <psp2/audioout.h>
#include <psp2/kernel/threadmgr.h>
#include <string.h>
#include <math.h>

/* =============================================================
 *  Sintetizador chiptune simples rodando numa thread propria.
 *  Sem arquivos de audio: tudo gerado em tempo real (leve).
 * ============================================================= */
#define SR     48000
#define GRAIN  1024
#define NVOICE 12          /* voz 0 = lead da musica, voz 1 = baixo; 2.. = SFX */
#define MASTER 0.22f

typedef struct {
    int          active;
    int          wave;     /* 0 square, 1 triangle, 2 noise, 3 sine */
    float        phase;
    float        freq;
    float        slide;    /* multiplicador por amostra */
    float        vol;
    float        env;
    float        decay;    /* multiplicador de envelope por amostra */
    int          samples_left;
    unsigned int noise;
} Voice;

static int      s_port = -1;
static SceUID   s_thid = -1;
static volatile int s_running = 0;
static volatile int s_enabled = 1;
static volatile int s_music = MUS_NONE;
static int16_t  s_buf[2][GRAIN * 2];
static Voice    s_v[NVOICE];

/* sequenciador */
static int s_step = 0;
static int s_step_samp = 0;

/* fila de SFX (produtor: jogo / consumidor: thread de audio) */
#define QN 48
static volatile int s_qhead = 0, s_qtail = 0;
static volatile unsigned char s_q[QN];

/* tabela de notas (Hz); indice 0 = silencio */
static const float NOTE[] = {
    0.0f, 110.0f, 131.0f, 147.0f, 165.0f, 196.0f, 220.0f,
    262.0f, 294.0f, 330.0f, 392.0f, 440.0f, 523.0f
};

typedef struct {
    const unsigned char *lead; int lead_len;
    const unsigned char *bass; int bass_len;
    int step_samples;
    int lead_wave, bass_wave;
} Track;

static const unsigned char MENU_LEAD[] = { 6,0,9,0, 8,0,5,0 };
static const unsigned char MENU_BASS[] = { 1,0,2,0 };
static const unsigned char GAME_LEAD[] = { 6,9,11,9, 8,9,8,6 };
static const unsigned char GAME_BASS[] = { 1,2,4,2 };
static const unsigned char BOSS_LEAD[] = { 11,12,11,9, 11,12,9,8 };
static const unsigned char BOSS_BASS[] = { 1,1,4,4 };

static const Track TRACKS[] = {
    /* MUS_NONE */ { 0, 0, 0, 0, 0, 0, 0 },
    /* MUS_MENU */ { MENU_LEAD, 8, MENU_BASS, 4, 12480, 3, 1 },
    /* MUS_GAME */ { GAME_LEAD, 8, GAME_BASS, 4,  7680, 0, 1 },
    /* MUS_BOSS */ { BOSS_LEAD, 8, BOSS_BASS, 4,  5760, 0, 2 },
};

/* parametros de cada SFX: wave, freq0, slide/sample, vol, duracao(s) */
typedef struct { int wave; float f0; float slide; float vol; float dur; } SfxDef;
static const SfxDef SFX[SFX_COUNT] = {
    /* SELECT  */ { 0, 760.0f,  1.0006f, 0.30f, 0.07f },
    /* MOVE    */ { 0, 440.0f,  1.0f,    0.16f, 0.03f },
    /* SHOOT   */ { 2, 640.0f,  0.9990f, 0.12f, 0.05f },
    /* HIT     */ { 2, 480.0f,  0.9985f, 0.14f, 0.04f },
    /* XP      */ { 0, 1100.0f, 1.0010f, 0.16f, 0.05f },
    /* LEVELUP */ { 0, 523.0f,  1.0009f, 0.30f, 0.34f },
    /* HURT    */ { 0, 320.0f,  0.9990f, 0.30f, 0.20f },
    /* KILL    */ { 2, 360.0f,  0.9980f, 0.18f, 0.06f },
    /* FUSION  */ { 3, 523.0f,  1.0008f, 0.30f, 0.45f },
    /* BOSS    */ { 2, 130.0f,  0.9994f, 0.38f, 0.55f },
    /* BOMB    */ { 2, 220.0f,  0.9988f, 0.40f, 0.40f },
};

static void start_sfx_voice(int id) {
    if (id < 0 || id >= SFX_COUNT) return;
    const SfxDef *d = &SFX[id];
    /* escolhe voz livre em [2,NVOICE) ou a mais antiga */
    int best = 2, bestleft = 0x7FFFFFFF;
    for (int v = 2; v < NVOICE; ++v) {
        if (!s_v[v].active) { best = v; break; }
        if (s_v[v].samples_left < bestleft) { bestleft = s_v[v].samples_left; best = v; }
    }
    Voice *v = &s_v[best];
    int samples = (int)(d->dur * SR);
    if (samples < 1) samples = 1;
    v->active = 1;
    v->wave = d->wave;
    v->phase = 0.0f;
    v->freq = d->f0;
    v->slide = d->slide;
    v->vol = d->vol;
    v->env = 1.0f;
    v->decay = expf(-4.5f / (float)samples);
    v->samples_left = samples;
    v->noise = 0x1234567u + (unsigned)id * 2654435761u;
}

static void set_music_voice(int idx, int note_index, int wave) {
    Voice *v = &s_v[idx];
    if (note_index <= 0) { v->active = 0; return; }   /* silencio */
    v->active = 1;
    v->wave = wave;
    v->phase = 0.0f;
    v->freq = NOTE[note_index];
    v->slide = 1.0f;
    v->vol = (idx == 1) ? 0.5f : 0.42f;   /* baixo um pouco mais forte */
    v->env = 1.0f;
    v->decay = 0.99996f;                  /* sustenta quase a nota toda */
    v->samples_left = SR;                 /* re-disparado a cada passo */
    v->noise = 0x9E3779B9u;
}

static float voice_tick(Voice *v) {
    float w;
    switch (v->wave) {
        case 0: w = (v->phase < 0.5f) ? 1.0f : -1.0f; break;
        case 1: w = 4.0f * fabsf(v->phase - 0.5f) - 1.0f; break;
        case 2:
            v->noise ^= v->noise << 13; v->noise ^= v->noise >> 17; v->noise ^= v->noise << 5;
            w = (float)((int)((v->noise >> 8) & 0xFFFF)) / 32768.0f - 1.0f;
            break;
        default: w = sinf(v->phase * 6.2831853f); break;
    }
    float s = w * v->env * v->vol;
    v->phase += v->freq / (float)SR;
    if (v->phase >= 1.0f) v->phase -= (float)(int)v->phase;
    v->env *= v->decay;
    v->freq *= v->slide;
    if (--v->samples_left <= 0 || v->env < 0.001f) v->active = 0;
    return s;
}

static void synth_fill(int16_t *out, int n) {
    if (!s_enabled) { memset(out, 0, (size_t)n * 2 * sizeof(int16_t)); return; }

    /* consome a fila de SFX */
    while (s_qhead != s_qtail) {
        int id = s_q[s_qhead];
        s_qhead = (s_qhead + 1) % QN;
        start_sfx_voice(id);
    }

    int mid = s_music;
    if (mid < 0 || mid > MUS_BOSS) mid = MUS_NONE;
    const Track *tr = &TRACKS[mid];

    for (int i = 0; i < n; ++i) {
        if (tr->lead_len > 0) {
            if (s_step_samp <= 0) {
                s_step_samp = tr->step_samples;
                set_music_voice(0, tr->lead[s_step % tr->lead_len], tr->lead_wave);
                set_music_voice(1, tr->bass[s_step % tr->bass_len], tr->bass_wave);
                s_step++;
            }
            s_step_samp--;
        } else {
            s_v[0].active = 0; s_v[1].active = 0;
        }

        float acc = 0.0f;
        for (int v = 0; v < NVOICE; ++v)
            if (s_v[v].active) acc += voice_tick(&s_v[v]);

        acc *= MASTER;
        int smp = (int)(acc * 32767.0f);
        if (smp > 32767) smp = 32767;
        if (smp < -32768) smp = -32768;
        out[i * 2] = (int16_t)smp;
        out[i * 2 + 1] = (int16_t)smp;
    }
}

static int audio_thread(SceSize args, void *argp) {
    (void)args; (void)argp;
    int idx = 0;
    while (s_running) {
        synth_fill(s_buf[idx], GRAIN);
        sceAudioOutOutput(s_port, s_buf[idx]);
        idx ^= 1;
    }
    return 0;
}

bool audio_init(void) {
    memset(s_v, 0, sizeof(s_v));
    s_port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN, GRAIN, SR, SCE_AUDIO_OUT_MODE_STEREO);
    if (s_port < 0) { s_port = -1; return false; }

    s_running = 1;
    s_thid = sceKernelCreateThread("vs_audio", audio_thread, 0x10000100, 0x10000, 0, 0, NULL);
    if (s_thid < 0) {
        s_running = 0;
        sceAudioOutReleasePort(s_port);
        s_port = -1;
        return false;
    }
    sceKernelStartThread(s_thid, 0, NULL);
    return true;
}

void audio_fini(void) {
    if (s_port < 0) return;
    s_running = 0;
    if (s_thid >= 0) {
        sceKernelWaitThreadEnd(s_thid, NULL, NULL);
        sceKernelDeleteThread(s_thid);
        s_thid = -1;
    }
    sceAudioOutReleasePort(s_port);
    s_port = -1;
}

void audio_play_sfx(SfxId id) {
    if (s_port < 0 || !s_enabled) return;
    int nt = (s_qtail + 1) % QN;
    if (nt == s_qhead) return;   /* fila cheia: descarta */
    s_q[s_qtail] = (unsigned char)id;
    s_qtail = nt;
}

void audio_set_music(MusicId id) {
    if ((int)id != s_music) s_music = (int)id;
}

void audio_set_enabled(bool on) {
    s_enabled = on ? 1 : 0;
}
