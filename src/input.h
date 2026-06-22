#ifndef INPUT_H
#define INPUT_H

#include "types.h"

/* Botoes logicos do jogo (abstraem o mapeamento fisico do PS Vita). */
typedef enum {
    BTN_CONFIRM,   /* X      */
    BTN_CANCEL,    /* O      */
    BTN_PAUSE,     /* START  */
    BTN_SQUARE,    /* []     */
    BTN_TRIANGLE,  /* /\     */
    BTN_UP,
    BTN_DOWN,
    BTN_LEFT,
    BTN_RIGHT,
    BTN_L,
    BTN_R,
    BTN_COUNT
} Btn;

typedef struct {
    Vec2 move;                 /* vetor de movimento (dpad + analogico esq.) */
    bool down[BTN_COUNT];      /* botao mantido pressionado */
    bool pressed[BTN_COUNT];   /* borda: pressionado neste frame */
    bool released[BTN_COUNT];  /* borda: solto neste frame */
} Input;

void input_init(void);
void input_poll(Input *in);

/* Helper de navegacao de menu: retorna -1/0/+1 com auto-repeat para baixo/cima. */
int  input_menu_vert(Input *in, float dt);
int  input_menu_horz(Input *in, float dt);

#endif /* INPUT_H */
