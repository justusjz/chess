#include <stdio.h>
#include <stdbool.h>
#include <math.h>
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
  // white begins
  int current_player = PIECE_WHITE;
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
      case SDL_EVENT_MOUSE_BUTTON_UP:
        int rank = board_get_rank(&board, event.button.y);
        int file = board_get_rank(&board, event.button.x);
        if (!selected)
        {
          struct piece piece = board.squares[rank * BOARD_SIZE + file];
          if (piece.type == PIECE_NONE || piece.color != current_player)
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
        int move_count = board_get_moves(&board, selected_rank, selected_file, moves);
        bool success = false;
        for (int i = 0; i < move_count; ++i)
        {
          if (moves[i].rank == rank && moves[i].file == file)
          {
            if (moves[i].type == MOVE_NORMAL)
            {
              play_sound(&move_sound);
            }
            else
            {
              play_sound(&capture_sound);
            }
            success = true;
            break;
          }
        }
        if (!success)
        {
          // not a valid move
          break;
        }
        // move piece to new location
        struct piece piece = board.squares[selected_rank * BOARD_SIZE + selected_file];
        board.squares[selected_rank * BOARD_SIZE + selected_file] = (struct piece){PIECE_WHITE, PIECE_NONE};
        board.squares[rank * BOARD_SIZE + file] = piece;
        selected = false;
        current_player = current_player == PIECE_WHITE ? PIECE_BLACK : PIECE_WHITE;
        break;
      }
    }
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderClear(renderer);
    board_draw(&board);
    if (selected)
    {
      draw_selector(&board, selected_rank, selected_file);
      struct move moves[32];
      int move_count = board_get_moves(&board, selected_rank, selected_file, moves);
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
