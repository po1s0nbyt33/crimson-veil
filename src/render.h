#ifndef RENDER_H
#define RENDER_H

#include "types.h"
#include <vita2d.h>

/* =============================================================
 *  Camada fina sobre o vita2d: fonte do sistema (PGF), helpers
 *  de texto e formas. Mantem o resto do codigo livre de detalhes
 *  de baixo nivel de desenho.
 * ============================================================= */

extern vita2d_pgf *g_font;

/* Altura aproximada de uma linha de texto na escala dada. */
#define LINE_H(scale) (22.0f * (scale))

/* Paleta basica. */
#define COL_WHITE   RGBA8(0xF2, 0xF2, 0xF5, 0xFF)
#define COL_BLACK   RGBA8(0x08, 0x08, 0x0C, 0xFF)
#define COL_DIM     RGBA8(0x9A, 0x9A, 0xA8, 0xFF)
#define COL_RED     RGBA8(0xE8, 0x3B, 0x3B, 0xFF)
#define COL_GREEN   RGBA8(0x5C, 0xD6, 0x6B, 0xFF)
#define COL_BLUE    RGBA8(0x4D, 0x9D, 0xE8, 0xFF)
#define COL_YELLOW  RGBA8(0xF2, 0xC8, 0x4B, 0xFF)
#define COL_PURPLE  RGBA8(0xA0, 0x6C, 0xE0, 0xFF)
#define COL_ORANGE  RGBA8(0xF0, 0x8A, 0x3C, 0xFF)
#define COL_CYAN    RGBA8(0x4F, 0xD6, 0xD0, 0xFF)
#define COL_PANEL   RGBA8(0x18, 0x1A, 0x26, 0xF0)
#define COL_PANEL2  RGBA8(0x26, 0x2A, 0x3E, 0xFF)
#define COL_ACCENT  RGBA8(0xE2, 0x4A, 0x6A, 0xFF)

static inline unsigned int col_a(unsigned int c, unsigned char a) {
    return (c & 0x00FFFFFFu) | ((unsigned int)a << 24);
}

void  render_init(void);
void  render_fini(void);

void  draw_text(float x, float y, unsigned int color, float scale, const char *s);
void  draw_textf(float x, float y, unsigned int color, float scale, const char *fmt, ...);
void  draw_text_center(float cx, float y, unsigned int color, float scale, const char *s);
void  draw_text_right(float rx, float y, unsigned int color, float scale, const char *s);
float text_width(float scale, const char *s);

void  draw_rect(float x, float y, float w, float h, unsigned int color);
void  draw_rect_outline(float x, float y, float w, float h, float thick, unsigned int color);
void  draw_bar(float x, float y, float w, float h, float frac, unsigned int bg, unsigned int fg);
void  draw_circle(float x, float y, float r, unsigned int color);
void  draw_ring(float x, float y, float r, float thick, unsigned int color);

#endif /* RENDER_H */
