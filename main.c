#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <SDL3/SDL.h>
#include "board.h"

#define WINDOW_SIZE 800
#define SELECTOR_THICKNESS 5

void draw_selector(const struct board *board, int rank, int file);
int board_get_rank(const struct board *board, float y);
int board_get_file(const struct board *board, float x);

int main(int argc, char *argv[])
{
  if (!SDL_Init(SDL_INIT_VIDEO))
  {
    printf("SDL initialization failed: %s\n", SDL_GetError());
    return 0;
  }
  SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
  bool running = true;
  SDL_Window *window = SDL_CreateWindow("Chess", WINDOW_SIZE, WINDOW_SIZE, 0);
  SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
  struct board board = board_init(renderer, 0, 0, WINDOW_SIZE, WINDOW_SIZE);
  bool selected = false;
  int selected_rank = 0;
  int selected_file = 0;
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
          if (board.squares[rank * BOARD_SIZE + file].type == PIECE_NONE)
          {
            // cannot select empty square
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
        if (board.squares[rank * BOARD_SIZE + file].type != PIECE_NONE)
        {
          // cannot move to a place that's already taken
          break;
        }
        // move piece to new location
        struct piece piece = board.squares[selected_rank * BOARD_SIZE + selected_file];
        board.squares[selected_rank * BOARD_SIZE + selected_file] = (struct piece){PIECE_WHITE, PIECE_NONE};
        board.squares[rank * BOARD_SIZE + file] = piece;
        selected = false;
        break;
      }
    }
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderClear(renderer);
    board_draw(&board);
    if (selected)
    {
      draw_selector(&board, selected_rank, selected_file);
    }
    SDL_RenderPresent(renderer);
  }
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
