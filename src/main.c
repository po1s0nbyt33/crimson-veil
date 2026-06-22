#include "game.h"
#include <vita2d.h>
#include <psp2/kernel/processmgr.h>

/* Estado global do jogo (grande; alocacao estatica evita malloc no Vita). */
static Game g_game;

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    /* Pool de vertices grande: o padrao do vita2d e so 1 MB e nao checa NULL ao
     * encher -> numa cena densa (boss + horda) ele estoura e a thread do GXM
     * recebe comandos invalidos e crasha. 8 MB da folga de sobra. */
    vita2d_init_advanced(8 * 1024 * 1024);
    vita2d_set_clear_color(RGBA8(0x0E, 0x10, 0x18, 0xFF));
    render_init();
    sprite_init();
    input_init();

    uint32_t seed = (uint32_t)sceKernelGetProcessTimeWide();
    game_init(&g_game, seed ? seed : 0xA5A5A5A5u);

    audio_init();   /* se falhar, o jogo segue mudo (sem crash) */
    audio_set_enabled(g_game.audio_on);

    uint64_t prev = sceKernelGetProcessTimeWide();
    float fps_acc = 0.0f;
    int   fps_frames = 0;

    while (!g_game.should_quit) {
        uint64_t now = sceKernelGetProcessTimeWide();
        float dt = (float)((double)(now - prev) / 1000000.0);
        prev = now;
        if (dt > MAX_DT) dt = MAX_DT;
        if (dt < 0.0f)   dt = 0.0f;

        fps_acc += dt;
        fps_frames++;
        if (fps_acc >= 0.5f) {
            g_game.fps = (float)fps_frames / fps_acc;
            fps_acc = 0.0f;
            fps_frames = 0;
        }

        input_poll(&g_game.input);
        game_update(&g_game, dt);

        vita2d_start_drawing();
        vita2d_clear_screen();
        game_draw(&g_game);
        vita2d_end_drawing();
        vita2d_swap_buffers();
    }

    /* Encerramento limpo: espera a GPU/thread de display terminar o ultimo frame
     * ANTES de liberar a fonte e o vita2d (liberar com o render em voo crasha o Vita). */
    audio_fini();       /* para a thread de audio e libera a porta */
    vita2d_wait_rendering_done();
    sprite_fini();      /* libera texturas dos sprites */
    render_fini();      /* libera a fonte PGF */
    vita2d_fini();      /* fecha o vita2d/GXM */
    sceKernelExitProcess(0);
    return 0;
}
