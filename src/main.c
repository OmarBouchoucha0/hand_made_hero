#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 600

typedef float f32;
typedef double f64;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

int main() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "failed to init SDL\n");
    return 1;
  }

  SDL_Window *Window =
      SDL_CreateWindow("handmade hero", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);

  SDL_Renderer *Renderer = SDL_CreateRenderer(Window, -1, 0);
  bool Running = true;
  while (Running) {
    SDL_Event Event;
    while (SDL_PollEvent(&Event)) {
      if (Event.type == SDL_QUIT) {
        Running = false;
      }
    }

    SDL_RenderClear(Renderer);
    SDL_RenderPresent(Renderer);
  }
  SDL_Quit();
  return 0;
}
