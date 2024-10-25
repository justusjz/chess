#include <assert.h>
#include <stdlib.h>
#include "texture.h"
#include "board.h"

struct board board_init(SDL_Renderer *renderer, int x, int y, int width, int height)
{
  struct board board;
  board.renderer = renderer;
  board.x = x;
  board.y = y;
  board.square_width = width / BOARD_SIZE;
  board.square_height = height / BOARD_SIZE;
  board.squares[0] = (struct piece){PIECE_BLACK, PIECE_ROOK};
  board.squares[1] = (struct piece){PIECE_BLACK, PIECE_KNIGHT};
  board.squares[2] = (struct piece){PIECE_BLACK, PIECE_BISHOP};
  board.squares[3] = (struct piece){PIECE_BLACK, PIECE_QUEEN};
  board.squares[4] = (struct piece){PIECE_BLACK, PIECE_KING};
  board.squares[5] = (struct piece){PIECE_BLACK, PIECE_BISHOP};
  board.squares[6] = (struct piece){PIECE_BLACK, PIECE_KNIGHT};
  board.squares[7] = (struct piece){PIECE_BLACK, PIECE_ROOK};
  for (int file = 0; file < BOARD_SIZE; ++file)
  {
    board.squares[BOARD_SIZE + file] = (struct piece){PIECE_BLACK, PIECE_PAWN};
    board.squares[6 * BOARD_SIZE + file] = (struct piece){PIECE_WHITE, PIECE_PAWN};
  }
  for (int rank = 2; rank < 6; ++rank)
  {
    for (int file = 0; file < BOARD_SIZE; ++file)
    {
      board.squares[rank * BOARD_SIZE + file] = (struct piece){PIECE_WHITE, PIECE_NONE};
    }
  }
  board.squares[7 * BOARD_SIZE] = (struct piece){PIECE_WHITE, PIECE_ROOK};
  board.squares[7 * BOARD_SIZE + 1] = (struct piece){PIECE_WHITE, PIECE_KNIGHT};
  board.squares[7 * BOARD_SIZE + 2] = (struct piece){PIECE_WHITE, PIECE_BISHOP};
  board.squares[7 * BOARD_SIZE + 3] = (struct piece){PIECE_WHITE, PIECE_QUEEN};
  board.squares[7 * BOARD_SIZE + 4] = (struct piece){PIECE_WHITE, PIECE_KING};
  board.squares[7 * BOARD_SIZE + 5] = (struct piece){PIECE_WHITE, PIECE_BISHOP};
  board.squares[7 * BOARD_SIZE + 6] = (struct piece){PIECE_WHITE, PIECE_KNIGHT};
  board.squares[7 * BOARD_SIZE + 7] = (struct piece){PIECE_WHITE, PIECE_ROOK};
  board.piece_textures[0] = load_texture(renderer, "./assets/white/bishop.png");
  board.piece_textures[1] = load_texture(renderer, "./assets/white/king.png");
  board.piece_textures[2] = load_texture(renderer, "./assets/white/knight.png");
  board.piece_textures[3] = load_texture(renderer, "./assets/white/pawn.png");
  board.piece_textures[4] = load_texture(renderer, "./assets/white/queen.png");
  board.piece_textures[5] = load_texture(renderer, "./assets/white/rook.png");
  board.piece_textures[6] = load_texture(renderer, "./assets/black/bishop.png");
  board.piece_textures[7] = load_texture(renderer, "./assets/black/king.png");
  board.piece_textures[8] = load_texture(renderer, "./assets/black/knight.png");
  board.piece_textures[9] = load_texture(renderer, "./assets/black/pawn.png");
  board.piece_textures[10] = load_texture(renderer, "./assets/black/queen.png");
  board.piece_textures[11] = load_texture(renderer, "./assets/black/rook.png");
  return board;
}

void board_free(struct board *board)
{
  for (int i = 0; i < 12; ++i)
  {
    SDL_DestroyTexture(board->piece_textures[i]);
  }
}

void board_draw_texture(const struct board *board, SDL_Texture *texture, int rank, int file)
{
  SDL_FRect dest;
  dest.x = board->x + file * board->square_width;
  dest.y = board->y + rank * board->square_height;
  dest.w = board->square_width;
  dest.h = board->square_height;
  SDL_RenderTexture(board->renderer, texture, NULL, &dest);
}

void board_draw(const struct board *board)
{
  for (int rank = 0; rank < BOARD_SIZE; ++rank)
  {
    for (int file = 0; file < BOARD_SIZE; ++file)
    {
      if (((rank + file) & 1) == 0)
      {
        SDL_SetRenderDrawColor(board->renderer, 255, 255, 153, 255);
      }
      else
      {
        SDL_SetRenderDrawColor(board->renderer, 102, 51, 0, 255);
      }
      SDL_FRect square;
      square.x = board->x + file * board->square_width;
      square.y = board->y + rank * board->square_height;
      square.w = board->square_width;
      square.h = board->square_height;
      SDL_RenderFillRect(board->renderer, &square);
      struct piece piece = board->squares[rank * BOARD_SIZE + file];
      if (piece.type != PIECE_NONE)
      {
        SDL_Texture *texture = board->piece_textures[piece.color * 6 + piece.type];
        board_draw_texture(board, texture, rank, file);
      }
    }
  }
}

bool check_move_pawn(const struct board *board, int rank, int file, bool diagonal, struct piece piece, struct move *moves, int *move_count)
{
  // other pieces are handled by `check_move`
  assert(piece.type == PIECE_PAWN);
  if (rank < 0 || rank > 7 || file < 0 || file > 7)
  {
    // outside of playing area
    return false;
  }
  struct piece other_piece = board->squares[rank * BOARD_SIZE + file];
  if (!diagonal && other_piece.type != PIECE_NONE)
  {
    // cannot move to square taken by another piece
    return false;
  }
  if (diagonal && (other_piece.type == PIECE_NONE || other_piece.color == piece.color))
  {
    // cannot move diagonally to empty square or capture a piece with the same color
    return false;
  }
  moves[(*move_count)++] = (struct move){rank, file};
  return true;
}

bool check_move(const struct board *board, int rank, int file, struct piece piece, struct move *moves, int *move_count)
{
  // pawns are handled by `check_move_pawn`
  assert(piece.type != PIECE_PAWN);
  if (rank < 0 || rank > 7 || file < 0 || file > 7)
  {
    // outside of playing area
    return false;
  }
  struct piece other_piece = board->squares[rank * BOARD_SIZE + file];
  if (other_piece.type != PIECE_NONE && other_piece.color == piece.color)
  {
    // cannot move to square taken by piece of same color
    return false;
  }
  moves[(*move_count)++] = (struct move){rank, file};
  if (other_piece.type != PIECE_NONE)
  {
    // we can capture this piece, but we can't continue afterwards
    return false;
  }
  return true;
}

void get_straight_moves(const struct board *board, int rank, int file, struct piece piece, struct move moves[32], int *move_count)
{
  // move up
  for (int i = 1; i <= rank; ++i)
  {
    if (!check_move(board, rank - i, file, piece, moves, move_count))
    {
      break;
    }
  }
  // move right
  for (int i = 1; i < BOARD_SIZE - file; ++i)
  {
    if (!check_move(board, rank, file + i, piece, moves, move_count))
    {
      break;
    }
  }
  // move down
  for (int i = 1; i < BOARD_SIZE - rank; ++i)
  {
    if (!check_move(board, rank + i, file, piece, moves, move_count))
    {
      break;
    }
  }
  // move left
  for (int i = 1; i <= file; ++i)
  {
    if (!check_move(board, rank, file - i, piece, moves, move_count))
    {
      break;
    }
  }
}

void get_diagonal_moves(const struct board *board, int rank, int file, struct piece piece, struct move moves[32], int *move_count)
{
  // move up-left
  for (int i = 1; i <= rank; ++i)
  {
    if (!check_move(board, rank - i, file - i, piece, moves, move_count))
    {
      break;
    }
  }
  // move up-right
  for (int i = 1; i <= rank; ++i)
  {
    if (!check_move(board, rank - i, file + i, piece, moves, move_count))
    {
      break;
    }
  }
  // move down-left
  for (int i = 1; i < BOARD_SIZE - rank; ++i)
  {
    if (!check_move(board, rank + i, file - i, piece, moves, move_count))
    {
      break;
    }
  }
  // move down-right
  for (int i = 1; i < BOARD_SIZE - rank; ++i)
  {
    if (!check_move(board, rank + i, file + i, piece, moves, move_count))
    {
      break;
    }
  }
}

int board_get_moves(const struct board *board, int rank, int file, struct move moves[32])
{
  struct piece piece = board->squares[rank * BOARD_SIZE + file];
  assert(piece.type != PIECE_NONE);
  int move_count = 0;
  if (piece.type == PIECE_BISHOP)
  {
    get_diagonal_moves(board, rank, file, piece, moves, &move_count);
  }
  else if (piece.type == PIECE_KING)
  {
    for (int i = -1; i <= 1; ++i)
    {
      for (int j = -1; j <= 1; ++j)
      {
        check_move(board, rank + i, file + j, piece, moves, &move_count);
      }
    }
  }
  else if (piece.type == PIECE_KNIGHT)
  {
    for (int rank_distance = -2; rank_distance <= 2; ++rank_distance)
    {
      if (rank_distance == 0)
      {
        // a knight always moves at least one rank
        continue;
      }
      // if we move 2 ranks, we move 1 file,
      // if we move 1 rank, we move 2 files
      int file_distance = 3 - abs(rank_distance);
      int left_file = file - file_distance;
      int right_file = file + file_distance;
      int new_rank = rank + rank_distance;
      check_move(board, new_rank, left_file, piece, moves, &move_count);
      check_move(board, new_rank, right_file, piece, moves, &move_count);
    }
  }
  else if (piece.type == PIECE_PAWN)
  {
    int direction = piece.color == PIECE_WHITE ? -1 : 1;
    bool success = check_move_pawn(board, rank + direction, file, false, piece, moves, &move_count);
    if (success && (rank == 1 && piece.color == PIECE_BLACK || rank == 6 && piece.color == PIECE_WHITE))
    {
      // pawn is in starting position, allow double move
      check_move_pawn(board, rank + 2 * direction, file, false, piece, moves, &move_count);
    }
    check_move_pawn(board, rank + direction, file - 1, true, piece, moves, &move_count);
    check_move_pawn(board, rank + direction, file + 1, true, piece, moves, &move_count);
  }
  else if (piece.type == PIECE_QUEEN)
  {
    get_straight_moves(board, rank, file, piece, moves, &move_count);
    get_diagonal_moves(board, rank, file, piece, moves, &move_count);
  }
  else if (piece.type == PIECE_ROOK)
  {
    get_straight_moves(board, rank, file, piece, moves, &move_count);
  }
  return move_count;
}
