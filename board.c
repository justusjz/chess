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
#include <stdio.h>

int board_get_moves(const struct board *board, int rank, int file, struct move moves[32])
{
  struct piece piece = board->squares[rank * BOARD_SIZE + file];
  assert(piece.type != PIECE_NONE);
  int move_count = 0;
  if (piece.type == PIECE_BISHOP)
  {
    for (int i = 1; i <= rank; ++i)
    {
      if (file - i >= 0)
      {
        moves[move_count++] = (struct move){rank - i, file - i};
      }
      if (file + i <= 7)
      {
        moves[move_count++] = (struct move){rank - i, file + i};
      }
    }
    for (int i = 1; i < BOARD_SIZE - rank; ++i)
    {
      if (file - i >= 0)
      {
        moves[move_count++] = (struct move){rank + i, file - i};
      }
      if (file + i <= 7)
      {
        moves[move_count++] = (struct move){rank + i, file + i};
      }
    }
  }
  else if (piece.type == PIECE_KING)
  {
    for (int i = -1; i <= 1; ++i)
    {
      for (int j = -1; j <= 1; ++j)
      {
        if (rank + i >= 0 && rank + i <= 7 && file + j >= 0 && file + j <= 7 && (i != 0 || j != 0))
        {
          moves[move_count++] = (struct move){rank + i, file + j};
        }
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
      if (new_rank >= 0 && new_rank <= 7 && left_file >= 0 && left_file <= 7)
      {
        moves[move_count++] = (struct move){rank + rank_distance, left_file};
      }
      if (new_rank >= 0 && new_rank <= 7 && right_file >= 0 && right_file <= 7)
      {
        moves[move_count++] = (struct move){rank + rank_distance, right_file};
      }
    }
  }
  else if (piece.type == PIECE_PAWN)
  {
    int direction = piece.color == PIECE_WHITE ? -1 : 1;
    int new_rank_1 = rank + direction;
    int new_rank_2 = rank + 2 * direction;
    if (new_rank_1 >= 0 && new_rank_1 <= 7)
    {
      moves[move_count++] = (struct move){new_rank_1, file};
    }
    if (new_rank_2 >= 0 && new_rank_2 <= 7)
    {
      moves[move_count++] = (struct move){new_rank_2, file};
    }
  }
  else if (piece.type == PIECE_QUEEN)
  {
    for (int i = 1; i <= rank; ++i)
    {
      if (file - i >= 0)
      {
        moves[move_count++] = (struct move){rank - i, file - i};
      }
      if (file + i <= 7)
      {
        moves[move_count++] = (struct move){rank - i, file + i};
      }
    }
    for (int i = 1; i < BOARD_SIZE - rank; ++i)
    {
      if (file - i >= 0)
      {
        moves[move_count++] = (struct move){rank + i, file - i};
      }
      if (file + i <= 7)
      {
        moves[move_count++] = (struct move){rank + i, file + i};
      }
    }
    for (int i = 0; i < BOARD_SIZE; ++i)
    {
      if (i == rank)
      {
        continue;
      }
      moves[move_count++] = (struct move){i, file};
    }
    for (int i = 0; i < BOARD_SIZE; ++i)
    {
      if (i == file)
      {
        continue;
      }
      moves[move_count++] = (struct move){rank, i};
    }
  }
  else if (piece.type == PIECE_ROOK)
  {
    for (int i = 0; i < BOARD_SIZE; ++i)
    {
      if (i == rank)
      {
        continue;
      }
      moves[move_count++] = (struct move){i, file};
    }
    for (int i = 0; i < BOARD_SIZE; ++i)
    {
      if (i == file)
      {
        continue;
      }
      moves[move_count++] = (struct move){rank, i};
    }
  }
  return move_count;
}
