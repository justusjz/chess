#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "texture.h"

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