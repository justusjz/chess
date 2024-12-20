#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include "board.h"
#include "texture.h"

#define WINDOW_SIZE 800
#define SELECTOR_THICKNESS 5

struct sound
{
  Uint8 *data;
  Uint32 length;
  SDL_AudioStream *stream;
};

struct sound load_sound(SDL_AudioDeviceID audioDevice, const char *path)
{
  struct sound result;
  SDL_AudioSpec src;
  SDL_AudioSpec dest;
  SDL_GetAudioDeviceFormat(audioDevice, &dest, NULL);
  SDL_LoadWAV(path, &src, &result.data, &result.length);
  result.stream = SDL_CreateAudioStream(&src, &dest);
  SDL_BindAudioStream(audioDevice, result.stream);
  return result;
}

void play_sound(const struct sound *sound)
{
  SDL_ClearAudioStream(sound->stream);
  SDL_PutAudioStreamData(sound->stream, sound->data, sound->length);
}

void free_sound(struct sound *sound)
{
  SDL_DestroyAudioStream(sound->stream);
  SDL_free(sound->data);
}

void draw_selector(const struct board *board, int rank, int file);
int board_get_rank(const struct board *board, float y);
int board_get_file(const struct board *board, float x);

int main(int argc, char *argv[])
{
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
  {
    printf("SDL initialization failed: %s\n", SDL_GetError());
    return 0;
  }
  SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
  bool running = true;
  SDL_Window *window = SDL_CreateWindow("Chess", WINDOW_SIZE, WINDOW_SIZE, 0);
  SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
  SDL_AudioDeviceID audioDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
  struct sound move_sound = load_sound(audioDevice, "./assets/move.wav");
  struct sound capture_sound = load_sound(audioDevice, "./assets/capture.wav");
  SDL_Texture *move_texture = load_texture(renderer, "./assets/move.png");
  struct board board = board_init(renderer, 0, 0, WINDOW_SIZE, WINDOW_SIZE);
  bool selected = false;
  int selected_rank = 0;
  int selected_file = 0;
  struct board *board_history = malloc(sizeof(struct board) * 256);
  int last_board = 0;
  bool ended = false;
  SDL_Texture *piece_textures[12];
  piece_textures[0] = load_texture(renderer, "./assets/white/bishop.png");
  piece_textures[1] = load_texture(renderer, "./assets/white/king.png");
  piece_textures[2] = load_texture(renderer, "./assets/white/knight.png");
  piece_textures[3] = load_texture(renderer, "./assets/white/pawn.png");
  piece_textures[4] = load_texture(renderer, "./assets/white/queen.png");
  piece_textures[5] = load_texture(renderer, "./assets/white/rook.png");
  piece_textures[6] = load_texture(renderer, "./assets/black/bishop.png");
  piece_textures[7] = load_texture(renderer, "./assets/black/king.png");
  piece_textures[8] = load_texture(renderer, "./assets/black/knight.png");
  piece_textures[9] = load_texture(renderer, "./assets/black/pawn.png");
  piece_textures[10] = load_texture(renderer, "./assets/black/queen.png");
  piece_textures[11] = load_texture(renderer, "./assets/black/rook.png");
  while (running)
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
      case SDL_EVENT_QUIT:
        running = false;
        break;
      case SDL_EVENT_KEY_DOWN:
        if (event.key.key == SDLK_U && last_board > 0)
        {
          ended = false;
          memcpy(&board, &board_history[--last_board], sizeof(struct board));
        }
        break;
      case SDL_EVENT_MOUSE_BUTTON_UP:
        if (ended)
        {
          break;
        }
        int rank = board_get_rank(&board, event.button.y);
        int file = board_get_rank(&board, event.button.x);
        if (!selected)
        {
          struct piece piece = board.squares[rank * BOARD_SIZE + file];
          if (piece.type == PIECE_NONE || piece.color != board.current_color)
          {
            // cannot select empty square or opponent piece
            break;
          }
          // select piece
          selected = true;
          selected_rank = rank;
          selected_file = file;
          break;
        }
        if (rank == selected_rank && file == selected_file)
        {
          // clicked on selected square again, deselect
          selected = false;
          break;
        }
        struct move moves[32];
        int move_count = board_get_legal_moves(&board, selected_rank, selected_file, moves);
        const struct move *move = NULL;
        for (int i = 0; i < move_count; ++i)
        {
          if (moves[i].rank == rank && moves[i].file == file)
          {
            move = &moves[i];
            break;
          }
        }
        if (move == NULL)
        {
          // not a valid move, try selecting
          struct piece piece = board.squares[rank * BOARD_SIZE + file];
          if (piece.type == PIECE_NONE || piece.color != board.current_color)
          {
            // cannot select empty square or opponent piece
            break;
          }
          // select piece
          selected = true;
          selected_rank = rank;
          selected_file = file;
          break;
        }
        if (move->type == MOVE_CAPTURE || move->type == MOVE_EN_PASSANT)
        {
          play_sound(&capture_sound);
        }
        else
        {
          play_sound(&move_sound);
        }
        // move piece to new location
        memcpy(&board_history[last_board++], &board, sizeof(struct board));
        board_make_move(&board, selected_rank, selected_file, move);
        selected = false;
        enum game_state status = board_status(&board, board.current_color);
        if (status == STATE_MATE)
        {
          if (board.current_color == PIECE_WHITE)
          {
            printf("Black won!\n");
          }
          else
          {
            printf("White won!\n");
          }
          ended = true;
        }
        else if (status == STATE_DRAW)
        {
          printf("Draw!\n");
          ended = true;
        }
        break;
      }
    }
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderClear(renderer);
    board_draw(&board, piece_textures);
    if (selected)
    {
      draw_selector(&board, selected_rank, selected_file);
      struct move moves[32];
      int move_count = board_get_legal_moves(&board, selected_rank, selected_file, moves);
      for (int i = 0; i < move_count; ++i)
      {
        board_draw_texture(&board, move_texture, moves[i].rank, moves[i].file);
      }
    }
    SDL_RenderPresent(renderer);
  }
  free_sound(&move_sound);
  free_sound(&capture_sound);
  SDL_CloseAudioDevice(audioDevice);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

void draw_selector(const struct board *board, int rank, int file)
{
  SDL_FRect dest;
  dest.x = (board->x + file * board->square_width) / SELECTOR_THICKNESS;
  dest.y = (board->y + rank * board->square_height) / SELECTOR_THICKNESS;
  dest.w = board->square_width / SELECTOR_THICKNESS;
  dest.h = board->square_height / SELECTOR_THICKNESS;
  SDL_SetRenderDrawColor(board->renderer, 0, 255, 0, 255);
  SDL_SetRenderScale(board->renderer, SELECTOR_THICKNESS, SELECTOR_THICKNESS);
  SDL_RenderRect(board->renderer, &dest);
  SDL_SetRenderScale(board->renderer, 1, 1);
}

int board_get_rank(const struct board *board, float y)
{
  return floor((y - board->y) / board->square_height);
}

int board_get_file(const struct board *board, float x)
{
  return floor((x - board->x) / board->square_width);
}
