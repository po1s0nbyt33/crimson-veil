#include "render.h"
#include <stdarg.h>
#include <stdio.h>
#include <math.h>

vita2d_pgf *g_font = NULL;

/* A fonte PGF desenha a partir da linha de base; deslocamos para que o "y"
 * recebido pelos helpers represente o TOPO do texto (layout mais intuitivo). */
#define BASELINE_OFF(scale) (17.0f * (scale))

void render_init(void) {
    g_font = vita2d_load_default_pgf();
}

void render_fini(void) {
    if (g_font) {
        vita2d_free_pgf(g_font);
        g_font = NULL;
    }
}

void draw_text(float x, float y, unsigned int color, float scale, const char *s) {
    if (g_font && s) {
        vita2d_pgf_draw_text(g_font, x, y + BASELINE_OFF(scale), color, scale, s);
    }
}

void draw_textf(float x, float y, unsigned int color, float scale, const char *fmt, ...) {
    char buf[256];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    draw_text(x, y, color, scale, buf);
}

float text_width(float scale, const char *s) {
    if (g_font && s) return (float)vita2d_pgf_text_width(g_font, scale, s);
    return 0.0f;
}

void draw_text_center(float cx, float y, unsigned int color, float scale, const char *s) {
    draw_text(cx - text_width(scale, s) * 0.5f, y, color, scale, s);
}

void draw_text_right(float rx, float y, unsigned int color, float scale, const char *s) {
    draw_text(rx - text_width(scale, s), y, color, scale, s);
}

void draw_rect(float x, float y, float w, float h, unsigned int color) {
    vita2d_draw_rectangle(x, y, w, h, color);
}

void draw_rect_outline(float x, float y, float w, float h, float t, unsigned int color) {
    vita2d_draw_rectangle(x, y, w, t, color);             /* topo   */
    vita2d_draw_rectangle(x, y + h - t, w, t, color);     /* base   */
    vita2d_draw_rectangle(x, y, t, h, color);             /* esq.   */
    vita2d_draw_rectangle(x + w - t, y, t, h, color);     /* dir.   */
}

void draw_bar(float x, float y, float w, float h, float frac, unsigned int bg, unsigned int fg) {
    frac = clampf(frac, 0.0f, 1.0f);
    draw_rect(x, y, w, h, bg);
    draw_rect(x, y, w * frac, h, fg);
}

void draw_circle(float x, float y, float r, unsigned int color) {
    vita2d_draw_fill_circle(x, y, r, color);
}

void draw_ring(float x, float y, float r, float thick, unsigned int color) {
    const int N = 18;
    for (float rr = r; rr < r + thick; rr += 1.0f) {
        float px = x + rr, py = y;
        for (int i = 1; i <= N; ++i) {
            float a = (float)i / (float)N * 6.28318530718f;
            float nx = x + cosf(a) * rr;
            float ny = y + sinf(a) * rr;
            vita2d_draw_line(px, py, nx, ny, color);
            px = nx; py = ny;
        }
    }
}
