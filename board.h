#ifndef BOARD_H
#define BOARD_H

#include <SDL3/SDL.h>

#define BOARD_SIZE 8

enum piece_color
{
  PIECE_WHITE,
  PIECE_BLACK,
};

enum piece_type
{
  PIECE_BISHOP,
  PIECE_KING,
  PIECE_KNIGHT,
  PIECE_PAWN,
  PIECE_QUEEN,
  PIECE_ROOK,
  PIECE_NONE,
};

struct piece
{
  enum piece_color color;
  enum piece_type type;
};

struct board
{
  SDL_Renderer *renderer;
  int x;
  int y;
  int square_width;
  int square_height;
  struct piece squares[BOARD_SIZE * BOARD_SIZE];
  SDL_Texture *textures[12];
};

struct board board_init(SDL_Renderer *renderer, int x, int y, int width, int height);
void board_free(struct board *board);
void board_draw(const struct board *board);

#endif
