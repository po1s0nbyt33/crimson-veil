#include "input.h"
#include <psp2/ctrl.h>
#include <string.h>

static SceCtrlData s_prev;

void input_init(void) {
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);
    memset(&s_prev, 0, sizeof(s_prev));
}

void input_poll(Input *in) {
    SceCtrlData pad;
    sceCtrlPeekBufferPositive(0, &pad, 1);

    uint32_t b  = pad.buttons;
    uint32_t pb = s_prev.buttons;

#define MAP(LOGICAL, MASK)                              \
    do {                                                \
        bool d  = (b  & (MASK)) != 0;                   \
        bool pd = (pb & (MASK)) != 0;                   \
        in->down[LOGICAL]     = d;                       \
        in->pressed[LOGICAL]  = d && !pd;                \
        in->released[LOGICAL] = !d && pd;                \
    } while (0)

    MAP(BTN_CONFIRM,  SCE_CTRL_CROSS);
    MAP(BTN_CANCEL,   SCE_CTRL_CIRCLE);
    MAP(BTN_PAUSE,    SCE_CTRL_START);
    MAP(BTN_SQUARE,   SCE_CTRL_SQUARE);
    MAP(BTN_TRIANGLE, SCE_CTRL_TRIANGLE);
    MAP(BTN_UP,       SCE_CTRL_UP);
    MAP(BTN_DOWN,     SCE_CTRL_DOWN);
    MAP(BTN_LEFT,     SCE_CTRL_LEFT);
    MAP(BTN_RIGHT,    SCE_CTRL_RIGHT);
    MAP(BTN_L,        SCE_CTRL_LTRIGGER);
    MAP(BTN_R,        SCE_CTRL_RTRIGGER);
#undef MAP

    /* Movimento: combina dpad e analogico esquerdo. */
    float mx = 0.0f, my = 0.0f;
    if (in->down[BTN_LEFT])  mx -= 1.0f;
    if (in->down[BTN_RIGHT]) mx += 1.0f;
    if (in->down[BTN_UP])    my -= 1.0f;
    if (in->down[BTN_DOWN])  my += 1.0f;

    float ax = ((float)pad.lx - 128.0f) / 128.0f;
    float ay = ((float)pad.ly - 128.0f) / 128.0f;
    const float dead = 0.22f;
    if (fabsf(ax) < dead) ax = 0.0f;
    if (fabsf(ay) < dead) ay = 0.0f;
    mx += ax;
    my += ay;

    Vec2 mv = v2(mx, my);
    if (v2len2(mv) > 1.0f) mv = v2norm(mv);
    in->move = mv;

    s_prev = pad;
}

int input_menu_vert(Input *in, float dt) {
    static float timer = 0.0f;
    static int   last = 0;
    int dir = 0;
    if (in->down[BTN_DOWN] || in->move.y > 0.55f)      dir = 1;
    else if (in->down[BTN_UP] || in->move.y < -0.55f)  dir = -1;

    if (dir == 0) { last = 0; timer = 0.0f; return 0; }
    if (dir != last) { last = dir; timer = 0.34f; return dir; }
    timer -= dt;
    if (timer <= 0.0f) { timer = 0.11f; return dir; }
    return 0;
}

int input_menu_horz(Input *in, float dt) {
    static float timer = 0.0f;
    static int   last = 0;
    int dir = 0;
    if (in->down[BTN_RIGHT] || in->move.x > 0.55f)     dir = 1;
    else if (in->down[BTN_LEFT] || in->move.x < -0.55f) dir = -1;

    if (dir == 0) { last = 0; timer = 0.0f; return 0; }
    if (dir != last) { last = dir; timer = 0.34f; return dir; }
    timer -= dt;
    if (timer <= 0.0f) { timer = 0.11f; return dir; }
    return 0;
}
