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
    moves[(*move_count)++] = (struct move){from_rank, from_file, to_rank, to_file, type, other_piece};
    // we can move through this
    return true;
  }
  if (diagonal && other_piece.type != PIECE_NONE && other_piece.color != piece.color)
  {
    // capture another piece diagonally
    moves[(*move_count)++] = (struct move){from_rank, from_file, to_rank, to_file, MOVE_CAPTURE, other_piece};
    return false;
  }
  if (diagonal && board->en_passant_possible && to_rank == board->en_passant_rank && to_file == board->en_passant_file)
  {
    // capture other pawn en passant
    moves[(*move_count)++] = (struct move){from_rank, from_file, to_rank, to_file, MOVE_EN_PASSANT, other_piece};
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
    moves[(*move_count)++] = (struct move){from_rank, from_file, to_rank, to_file, MOVE_NORMAL, other_piece};
    // we can move through this square
    return true;
  }
  if (other_piece.color != piece.color)
  {
    // capture another piece
    moves[(*move_count)++] = (struct move){from_rank, from_file, to_rank, to_file, MOVE_CAPTURE, other_piece};
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
        check_move(board, rank, file, rank + i, file + j, piece, moves, &move_count);
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

void board_make_move(struct board *board, const struct move *move)
{
  struct piece moved = board->squares[move->from_rank * 8 + move->from_file];
  int direction = moved.color == PIECE_WHITE ? -1 : 1;
  if (move->type == MOVE_DOUBLE)
  {
    // double pawn push
    board->en_passant_possible = true;
    board->en_passant_rank = move->to_rank - direction;
    board->en_passant_file = move->to_file;
  }
  else
  {
    board->en_passant_possible = false;
  }
  board->squares[move->from_rank * 8 + move->from_file] = (struct piece){PIECE_WHITE, PIECE_NONE};
  board->squares[move->to_rank * 8 + move->to_file] = moved;
  if (move->type == MOVE_EN_PASSANT)
  {
    board->squares[(move->to_rank - direction) * 8 + move->to_file] = (struct piece){PIECE_WHITE, PIECE_NONE};
  }
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
        int move_count = board_get_moves(board, rank, file, moves);
        for (int i = 0; i < move_count; ++i)
        {
          if (moves[i].captured.type == PIECE_KING)
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
