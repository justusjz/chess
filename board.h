#ifndef BOARD_H
#define BOARD_H

#include <SDL3/SDL.h>
#include <stdbool.h>

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
  SDL_Texture *piece_textures[12];
  bool en_passant_possible;
  int en_passant_rank;
  int en_passant_file;
};

enum move_type
{
  MOVE_NORMAL,
  MOVE_DOUBLE,
  MOVE_CAPTURE,
  MOVE_EN_PASSANT,
};

struct move
{
  int from_rank;
  int from_file;
  int to_rank;
  int to_file;
  enum move_type type;
  struct piece captured;
};

struct board board_init(SDL_Renderer *renderer, int x, int y, int width, int height);
void board_free(struct board *board);
void board_draw_texture(const struct board *board, SDL_Texture *texture, int rank, int file);
void board_draw(const struct board *board);

int board_get_moves(const struct board *board, int rank, int file, struct move moves[32]);
void board_make_move(struct board *board, const struct move *move);
void board_unmake_move(struct board *board, const struct move *move);

#endif
