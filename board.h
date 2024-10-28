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
  bool has_moved;
};

struct board
{
  SDL_Renderer *renderer;
  int x;
  int y;
  int square_width;
  int square_height;
  struct piece squares[BOARD_SIZE * BOARD_SIZE];
  bool en_passant_possible;
  int en_passant_rank;
  int en_passant_file;
  enum piece_color current_color;
};

enum move_type
{
  MOVE_NORMAL,
  MOVE_DOUBLE,
  MOVE_CAPTURE,
  MOVE_EN_PASSANT,
  MOVE_CASTLE_LEFT,
  MOVE_CASTLE_RIGHT,
};

struct move
{
  int rank;
  int file;
  enum move_type type;
};

enum game_state
{
  STATE_OK,
  STATE_MATE,
  STATE_DRAW,
};

struct board board_init(SDL_Renderer *renderer, int x, int y, int width, int height);
void board_draw_texture(const struct board *board, SDL_Texture *texture, int rank, int file);
void board_draw(const struct board *board, SDL_Texture *textures[12]);

int board_get_pseudo_moves(const struct board *board, int rank, int file, struct move moves[32], bool castling);
void board_make_move(struct board *board, int from_rank, int from_file, const struct move *move);

bool board_in_check(const struct board *board, enum piece_color color);
enum game_state board_status(struct board *board, enum piece_color color);

int board_get_legal_moves(const struct board *board, int rank, int file, struct move moves[32]);

#endif
