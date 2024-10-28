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
  board.en_passant_possible = false;
  board.current_color = PIECE_WHITE;
  board.squares[0] = (struct piece){PIECE_BLACK, PIECE_ROOK, false};
  board.squares[1] = (struct piece){PIECE_BLACK, PIECE_KNIGHT, false};
  board.squares[2] = (struct piece){PIECE_BLACK, PIECE_BISHOP, false};
  board.squares[3] = (struct piece){PIECE_BLACK, PIECE_QUEEN, false};
  board.squares[4] = (struct piece){PIECE_BLACK, PIECE_KING, false};
  board.squares[5] = (struct piece){PIECE_BLACK, PIECE_BISHOP, false};
  board.squares[6] = (struct piece){PIECE_BLACK, PIECE_KNIGHT, false};
  board.squares[7] = (struct piece){PIECE_BLACK, PIECE_ROOK, false};
  for (int file = 0; file < BOARD_SIZE; ++file)
  {
    board.squares[BOARD_SIZE + file] = (struct piece){PIECE_BLACK, PIECE_PAWN, false};
    board.squares[6 * BOARD_SIZE + file] = (struct piece){PIECE_WHITE, PIECE_PAWN, false};
  }
  for (int rank = 2; rank < 6; ++rank)
  {
    for (int file = 0; file < BOARD_SIZE; ++file)
    {
      board.squares[rank * BOARD_SIZE + file] = (struct piece){PIECE_WHITE, PIECE_NONE, false};
    }
  }
  board.squares[7 * BOARD_SIZE] = (struct piece){PIECE_WHITE, PIECE_ROOK, false};
  board.squares[7 * BOARD_SIZE + 1] = (struct piece){PIECE_WHITE, PIECE_KNIGHT, false};
  board.squares[7 * BOARD_SIZE + 2] = (struct piece){PIECE_WHITE, PIECE_BISHOP, false};
  board.squares[7 * BOARD_SIZE + 3] = (struct piece){PIECE_WHITE, PIECE_QUEEN, false};
  board.squares[7 * BOARD_SIZE + 4] = (struct piece){PIECE_WHITE, PIECE_KING, false};
  board.squares[7 * BOARD_SIZE + 5] = (struct piece){PIECE_WHITE, PIECE_BISHOP, false};
  board.squares[7 * BOARD_SIZE + 6] = (struct piece){PIECE_WHITE, PIECE_KNIGHT, false};
  board.squares[7 * BOARD_SIZE + 7] = (struct piece){PIECE_WHITE, PIECE_ROOK, false};
  return board;
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

void board_draw(const struct board *board, SDL_Texture *textures[12])
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
        SDL_Texture *texture = textures[piece.color * 6 + piece.type];
        board_draw_texture(board, texture, rank, file);
      }
    }
  }
}

bool check_move_pawn(const struct board *board, int from_rank, int from_file, int to_rank, int to_file, bool diagonal, struct piece piece, struct move *moves, int *move_count, enum move_type type)
{
  // other pieces are handled by `check_move`
  assert(piece.type == PIECE_PAWN);
  if (to_rank < 0 || to_rank > 7 || to_file < 0 || to_file > 7)
  {
    // outside of playing area
    return false;
  }
  struct piece other_piece = board->squares[to_rank * BOARD_SIZE + to_file];
  if (!diagonal && other_piece.type == PIECE_NONE)
  {
    // move straight to an empty square
    moves[(*move_count)++] = (struct move){to_rank, to_file, type};
    // we can move through this
    return true;
  }
  if (diagonal && other_piece.type != PIECE_NONE && other_piece.color != piece.color)
  {
    // capture another piece diagonally
    moves[(*move_count)++] = (struct move){to_rank, to_file, MOVE_CAPTURE};
    return false;
  }
  if (diagonal && board->en_passant_possible && to_rank == board->en_passant_rank && to_file == board->en_passant_file)
  {
    // capture other pawn en passant
    moves[(*move_count)++] = (struct move){to_rank, to_file, MOVE_EN_PASSANT};
  }
  return false;
}

bool check_move(const struct board *board, int from_rank, int from_file, int to_rank, int to_file, struct piece piece, struct move *moves, int *move_count)
{
  // pawns are handled by `check_move_pawn`
  assert(piece.type != PIECE_PAWN);
  if (to_rank < 0 || to_rank > 7 || to_file < 0 || to_file > 7)
  {
    // outside of playing area
    return false;
  }
  struct piece other_piece = board->squares[to_rank * BOARD_SIZE + to_file];
  if (other_piece.type == PIECE_NONE)
  {
    // move to an empty square
    moves[(*move_count)++] = (struct move){to_rank, to_file, MOVE_NORMAL};
    // we can move through this square
    return true;
  }
  if (other_piece.color != piece.color)
  {
    // capture another piece
    moves[(*move_count)++] = (struct move){to_rank, to_file, MOVE_CAPTURE};
    // we cannot move through this square
    return false;
  }
  return false;
}

void get_straight_moves(const struct board *board, int rank, int file, struct piece piece, struct move moves[32], int *move_count)
{
  // move up
  for (int i = 1; i <= rank; ++i)
  {
    if (!check_move(board, rank, file, rank - i, file, piece, moves, move_count))
    {
      break;
    }
  }
  // move right
  for (int i = 1; i < BOARD_SIZE - file; ++i)
  {
    if (!check_move(board, rank, file, rank, file + i, piece, moves, move_count))
    {
      break;
    }
  }
  // move down
  for (int i = 1; i < BOARD_SIZE - rank; ++i)
  {
    if (!check_move(board, rank, file, rank + i, file, piece, moves, move_count))
    {
      break;
    }
  }
  // move left
  for (int i = 1; i <= file; ++i)
  {
    if (!check_move(board, rank, file, rank, file - i, piece, moves, move_count))
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
    if (!check_move(board, rank, file, rank - i, file - i, piece, moves, move_count))
    {
      break;
    }
  }
  // move up-right
  for (int i = 1; i <= rank; ++i)
  {
    if (!check_move(board, rank, file, rank - i, file + i, piece, moves, move_count))
    {
      break;
    }
  }
  // move down-left
  for (int i = 1; i < BOARD_SIZE - rank; ++i)
  {
    if (!check_move(board, rank, file, rank + i, file - i, piece, moves, move_count))
    {
      break;
    }
  }
  // move down-right
  for (int i = 1; i < BOARD_SIZE - rank; ++i)
  {
    if (!check_move(board, rank, file, rank + i, file + i, piece, moves, move_count))
    {
      break;
    }
  }
}

void check_castle_left(const struct board *board, int rank, struct move moves[32], int *move_count)
{
  enum piece_color color = board->squares[rank * 8 + 4].color;
  if (board->squares[rank * 8].has_moved)
  {
    // cannot castle when rook has moved
    return;
  }
  if (board->squares[rank * 8 + 1].type != PIECE_NONE || board->squares[rank * 8 + 2].type != PIECE_NONE || board->squares[rank * 8 + 3].type != PIECE_NONE)
  {
    // cannot castle when squares are occupied
    return;
  }
  struct board new_board;
  memcpy(&new_board, board, sizeof(struct board));
  board_make_move(&new_board, rank, 4, &(struct move){rank, 3, MOVE_NORMAL});
  if (board_in_check(&new_board, color))
  {
    // cannot castle if intermediate position would be in check
    return;
  }
  moves[(*move_count)++] = (struct move){rank, 2, MOVE_CASTLE_LEFT};
}

void check_castle_right(const struct board *board, int rank, struct move moves[32], int *move_count)
{
  enum piece_color color = board->squares[rank * 8 + 4].color;
  if (board->squares[rank * 8 + 7].has_moved)
  {
    // cannot castle when rook has moved
    return;
  }
  if (board->squares[rank * 8 + 5].type != PIECE_NONE || board->squares[rank * 8 + 6].type != PIECE_NONE)
  {
    // cannot castle when squares are occupied
    return;
  }
  struct board new_board;
  memcpy(&new_board, board, sizeof(struct board));
  board_make_move(&new_board, rank, 4, &(struct move){rank, 5, MOVE_NORMAL});
  if (board_in_check(&new_board, color))
  {
    // cannot castle if intermediate position would be in check
    return;
  }
  moves[(*move_count)++] = (struct move){rank, 6, MOVE_CASTLE_RIGHT};
}

int board_get_pseudo_moves(const struct board *board, int rank, int file, struct move moves[32], bool castling)
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
    // normal movement
    for (int i = -1; i <= 1; ++i)
    {
      for (int j = -1; j <= 1; ++j)
      {
        check_move(board, rank, file, rank + i, file + j, piece, moves, &move_count);
      }
    }
    if (castling)
    {
      if (piece.has_moved || board_in_check(board, piece.color))
      {
        // no castling possible when king has moved or is in check
        return move_count;
      }
      check_castle_left(board, rank, moves, &move_count);
      check_castle_right(board, rank, moves, &move_count);
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
      check_move(board, rank, file, new_rank, left_file, piece, moves, &move_count);
      check_move(board, rank, file, new_rank, right_file, piece, moves, &move_count);
    }
  }
  else if (piece.type == PIECE_PAWN)
  {
    int direction = piece.color == PIECE_WHITE ? -1 : 1;
    bool success = check_move_pawn(board, rank, file, rank + direction, file, false, piece, moves, &move_count, MOVE_NORMAL);
    if (success && (rank == 1 && piece.color == PIECE_BLACK || rank == 6 && piece.color == PIECE_WHITE))
    {
      // pawn is in starting position, allow double move
      check_move_pawn(board, rank, file, rank + 2 * direction, file, false, piece, moves, &move_count, MOVE_DOUBLE);
    }
    check_move_pawn(board, rank, file, rank + direction, file - 1, true, piece, moves, &move_count, MOVE_NORMAL);
    check_move_pawn(board, rank, file, rank + direction, file + 1, true, piece, moves, &move_count, MOVE_NORMAL);
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

void board_make_move(struct board *board, int from_rank, int from_file, const struct move *move)
{
  struct piece moved = board->squares[from_rank * 8 + from_file];
  moved.has_moved = true;
  int direction = moved.color == PIECE_WHITE ? -1 : 1;
  if (move->type == MOVE_DOUBLE)
  {
    // double pawn push
    board->en_passant_possible = true;
    board->en_passant_rank = move->rank - direction;
    board->en_passant_file = move->file;
  }
  else
  {
    board->en_passant_possible = false;
  }
  board->squares[from_rank * 8 + from_file] = (struct piece){PIECE_WHITE, PIECE_NONE, false};
  board->squares[move->rank * 8 + move->file] = moved;
  if (move->type == MOVE_EN_PASSANT)
  {
    board->squares[(move->rank - direction) * 8 + move->file] = (struct piece){PIECE_WHITE, PIECE_NONE, false};
  }
  if (move->type == MOVE_CASTLE_LEFT)
  {
    struct piece rook = board->squares[from_rank * 8];
    rook.has_moved = true;
    board->squares[from_rank * 8] = (struct piece){PIECE_WHITE, PIECE_NONE, false};
    board->squares[from_rank * 8 + 3] = rook;
  }
  if (move->type == MOVE_CASTLE_RIGHT)
  {
    struct piece rook = board->squares[from_rank * 8];
    rook.has_moved = true;
    board->squares[from_rank * 8 + 7] = (struct piece){PIECE_WHITE, PIECE_NONE, false};
    board->squares[from_rank * 8 + 5] = rook;
  }
  board->current_color = board->current_color == PIECE_WHITE ? PIECE_BLACK : PIECE_WHITE;
}

bool board_in_check(const struct board *board, enum piece_color color)
{
  enum piece_color other_color = color == PIECE_WHITE ? PIECE_BLACK : PIECE_WHITE;
  for (int rank = 0; rank < 8; ++rank)
  {
    for (int file = 0; file < 8; ++file)
    {
      struct piece piece = board->squares[rank * 8 + file];
      if (piece.type != PIECE_NONE && piece.color == other_color)
      {
        // found piece of other color, check whether it attacks our king
        struct move moves[32];
        int move_count = board_get_pseudo_moves(board, rank, file, moves, false);
        for (int i = 0; i < move_count; ++i)
        {
          if (board->squares[moves[i].rank * 8 + moves[i].file].type == PIECE_KING)
          {
            // move would capture our king, so we're in check
            return true;
          }
        }
      }
    }
  }
  return false;
}

bool are_moves_possible(struct board *board, enum piece_color color)
{
  for (int rank = 0; rank < 8; ++rank)
  {
    for (int file = 0; file < 8; ++file)
    {
      struct piece piece = board->squares[rank * 8 + file];
      if (piece.type == PIECE_NONE || piece.color != color)
      {
        continue;
      }
      struct move moves[32];
      int move_count = board_get_legal_moves(board, rank, file, moves);
      if (move_count > 0)
      {
        return true;
      }
    }
  }
  return false;
}

enum game_state board_status(struct board *board, enum piece_color color)
{
  if (are_moves_possible(board, color))
  {
    return STATE_OK;
  }
  if (board_in_check(board, color))
  {
    return STATE_MATE;
  }
  return STATE_DRAW;
}

int board_get_legal_moves(const struct board *board, int rank, int file, struct move moves[32])
{
  struct move pseudo_moves[32];
  enum piece_color current_color = board->squares[rank * 8 + file].color;
  int pseudo_move_count = board_get_pseudo_moves(board, rank, file, pseudo_moves, true);
  int move_count = 0;
  for (int i = 0; i < pseudo_move_count; ++i)
  {
    struct board new_board;
    memcpy(&new_board, board, sizeof(struct board));
    board_make_move(&new_board, rank, file, &pseudo_moves[i]);
    if (!board_in_check(&new_board, current_color))
    {
      moves[move_count++] = pseudo_moves[i];
    }
  }
  return move_count;
}
