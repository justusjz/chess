#include "stb_image.h"
#include "board.h"

SDL_Texture *load_texture(SDL_Renderer *renderer, const char *filename)
{
  int width, height;
  unsigned char *data = stbi_load(filename, &width, &height, NULL, 4);
  SDL_Surface *surface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_RGBA32, data, width * 4);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
  SDL_DestroySurface(surface);
  stbi_image_free(data);
  return texture;
}

struct board board_init(SDL_Renderer *renderer, int x, int y, int width, int height)
{
  struct board board;
  board.renderer = renderer;
  board.x = x;
  board.y = y;
  board.square_width = width / BOARD_SIZE;
  board.square_height = height / BOARD_SIZE;
  board.squares[0] = (struct piece){PIECE_WHITE, PIECE_ROOK};
  board.squares[1] = (struct piece){PIECE_WHITE, PIECE_KNIGHT};
  board.squares[2] = (struct piece){PIECE_WHITE, PIECE_BISHOP};
  board.squares[3] = (struct piece){PIECE_WHITE, PIECE_QUEEN};
  board.squares[4] = (struct piece){PIECE_WHITE, PIECE_KING};
  board.squares[5] = (struct piece){PIECE_WHITE, PIECE_BISHOP};
  board.squares[6] = (struct piece){PIECE_WHITE, PIECE_KNIGHT};
  board.squares[7] = (struct piece){PIECE_WHITE, PIECE_ROOK};
  for (int file = 0; file < BOARD_SIZE; ++file)
  {
    board.squares[BOARD_SIZE + file] = (struct piece){PIECE_WHITE, PIECE_PAWN};
    board.squares[6 * BOARD_SIZE + file] = (struct piece){PIECE_BLACK, PIECE_PAWN};
  }
  for (int rank = 2; rank < 6; ++rank)
  {
    for (int file = 0; file < BOARD_SIZE; ++file)
    {
      board.squares[rank * BOARD_SIZE + file] = (struct piece){PIECE_WHITE, PIECE_NONE};
    }
  }
  board.squares[7 * BOARD_SIZE] = (struct piece){PIECE_BLACK, PIECE_ROOK};
  board.squares[7 * BOARD_SIZE + 1] = (struct piece){PIECE_BLACK, PIECE_KNIGHT};
  board.squares[7 * BOARD_SIZE + 2] = (struct piece){PIECE_BLACK, PIECE_BISHOP};
  board.squares[7 * BOARD_SIZE + 3] = (struct piece){PIECE_BLACK, PIECE_QUEEN};
  board.squares[7 * BOARD_SIZE + 4] = (struct piece){PIECE_BLACK, PIECE_KING};
  board.squares[7 * BOARD_SIZE + 5] = (struct piece){PIECE_BLACK, PIECE_BISHOP};
  board.squares[7 * BOARD_SIZE + 6] = (struct piece){PIECE_BLACK, PIECE_KNIGHT};
  board.squares[7 * BOARD_SIZE + 7] = (struct piece){PIECE_BLACK, PIECE_ROOK};
  board.textures[0] = load_texture(renderer, "./assets/white/bishop.png");
  board.textures[1] = load_texture(renderer, "./assets/white/king.png");
  board.textures[2] = load_texture(renderer, "./assets/white/knight.png");
  board.textures[3] = load_texture(renderer, "./assets/white/pawn.png");
  board.textures[4] = load_texture(renderer, "./assets/white/queen.png");
  board.textures[5] = load_texture(renderer, "./assets/white/rook.png");
  board.textures[6] = load_texture(renderer, "./assets/black/bishop.png");
  board.textures[7] = load_texture(renderer, "./assets/black/king.png");
  board.textures[8] = load_texture(renderer, "./assets/black/knight.png");
  board.textures[9] = load_texture(renderer, "./assets/black/pawn.png");
  board.textures[10] = load_texture(renderer, "./assets/black/queen.png");
  board.textures[11] = load_texture(renderer, "./assets/black/rook.png");
  return board;
}

void board_free(struct board *board)
{
  for (int i = 0; i < 12; ++i)
  {
    SDL_DestroyTexture(board->textures[i]);
  }
}

void draw_piece(const struct board *board, struct piece piece, int rank, int file)
{
  SDL_Texture *texture = board->textures[piece.color * 6 + piece.type];
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
        draw_piece(board, piece, rank, file);
      }
    }
  }
}
