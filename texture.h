#ifndef TEXTURE_H
#define TEXTURE_H

#include <SDL3/SDL.h>

SDL_Texture *load_texture(SDL_Renderer *renderer, const char *filename);

#endif
