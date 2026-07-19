#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>

#define global_variable static
#define local_persist static
#define internal static

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

global_variable i32 WINDOW_WIDTH = 1900;
global_variable i32 WINDOW_HEIGHT = 900;
global_variable bool Running = true;
global_variable void *BitmapMemory;
global_variable i32 BitmapMemorySize;
global_variable i32 BitmapWidth;
global_variable i32 BitmapHeight;
global_variable u8 BytesPerPixel = 4;

internal void RenderWeirdGradiant(i32 Xoffset, i32 Yoffset) {
  int pitch = BitmapWidth * BytesPerPixel;
  u8 *row = (u8 *)BitmapMemory;

  u8 red = 255;
  u8 green = 0;
  u8 blue = 0;
  u8 padding = 0;

  for (u64 y = 0; y < BitmapHeight; ++y) {
    u8 *pixel = (u8 *)row;
    for (u64 x = 0; x < BitmapWidth; ++x) {
      *pixel = (u8)(x + Xoffset);
      ++pixel;

      *pixel = (u8)(x + Xoffset);
      ++pixel;

      *pixel = (u8)(y + Yoffset);
      ++pixel;

      *pixel = 0;
      ++pixel;
    }
    row += pitch;
  }
}

internal void ResizeDIBSection(SDL_Renderer *Renderer, SDL_Texture **Texture) {
  if (BitmapMemory) {
    i32 error = munmap(BitmapMemory, BitmapMemorySize);
    if (error != 0) {
      return;
    }
  }
  BitmapWidth = WINDOW_WIDTH;
  BitmapHeight = WINDOW_HEIGHT;

  BitmapMemorySize = BitmapWidth * BitmapHeight * BytesPerPixel;
  BitmapMemory = mmap(NULL, BitmapMemorySize, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (BitmapMemory == MAP_FAILED) {
    return;
  }
  if (*Texture) {
    SDL_DestroyTexture(*Texture);
  }
  *Texture =
      SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_STREAMING, BitmapWidth, BitmapHeight);
}

internal void UpdateWindow(SDL_Renderer *renderer, SDL_Texture *Texture) {
  SDL_UpdateTexture(Texture, NULL, BitmapMemory, BitmapWidth * BytesPerPixel);
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, Texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

internal void HandleEvent(SDL_Window *window, SDL_Renderer *Renderer,
                          SDL_Texture *Texture, SDL_Event event) {
  switch (event.type) {
  case SDL_QUIT: {
    printf("[INFO] user quit\n");
    Running = false;
  } break;
  case SDL_APP_TERMINATING: {
    printf("[INFO] os quit\n");
    Running = false;
  } break;
  case SDL_WINDOWEVENT: {
    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
      printf("[INFO] window resized\n");
      SDL_GetWindowSize(window, &WINDOW_WIDTH, &WINDOW_HEIGHT);
      ResizeDIBSection(Renderer, &Texture);
    }
  } break;
  }
}

int main() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "failed to init SDL\n");
    return 1;
  }

  SDL_Window *Window = SDL_CreateWindow("handmade hero", SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,
                                        WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
  SDL_Renderer *Renderer = SDL_CreateRenderer(Window, -1, 0);
  SDL_Texture *Texture = SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_ARGB8888,
                                           SDL_TEXTUREACCESS_STREAMING,
                                           WINDOW_WIDTH, WINDOW_HEIGHT);

  if (Window == NULL || Renderer == NULL) {
    fprintf(stderr, "[ERR] Failed initilization: %s\n", SDL_GetError());
    return 1;
  }
  i32 x = 0;
  i32 y = 0;
  while (Running) {
    SDL_Event Event;
    while (SDL_PollEvent(&Event)) {
      HandleEvent(Window, Renderer, Texture, Event);
    }
    UpdateWindow(Renderer, Texture);
    RenderWeirdGradiant(x, y);
    ++x;
  }
  return 0;
}
