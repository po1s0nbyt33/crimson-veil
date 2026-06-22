#ifndef MENU_H
#define MENU_H

#include "types.h"

typedef struct Game Game;

/* Trata entrada e desenho de todas as telas que NAO sao a simulacao em si:
 * menu inicial, loja de melhorias, configuracoes, level-up, pausa e game over.
 * A logica de qual tela mostrar vem do estado atual do jogo (g->state). */
void menu_update(Game *g, float dt);
void menu_draw(Game *g);

#endif /* MENU_H */
