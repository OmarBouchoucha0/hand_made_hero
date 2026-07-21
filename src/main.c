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
typedef struct {
  void *Memory;
  i32 MemorySize;
  i32 Width;
  i32 Height;
  u8 BytesPerPixel;
} BitmapBuffer;
global_variable BitmapBuffer GlobalBackBuffer =
    (BitmapBuffer){.BytesPerPixel = 4};

internal void RenderWeirdGradiant(BitmapBuffer Buffer, i32 Xoffset,
                                  i32 Yoffset) {
  int pitch = Buffer.Width * Buffer.BytesPerPixel;
  u8 *row = (u8 *)Buffer.Memory;

  for (i32 y = 0; y < Buffer.Height; ++y) {
    u32 *pixel = (u32 *)row;
    for (i32 x = 0; x < Buffer.Width; ++x) {
      u8 red = 0;
      u8 green = y - Yoffset;
      u8 blue = 0;
      *pixel++ = (red << 16 | green << 8 | blue);
    }
    row += pitch;
  }
}

internal void AllocateBitmap(BitmapBuffer *Buffer) {
  Buffer->Width = WINDOW_WIDTH;
  Buffer->Height = WINDOW_HEIGHT;
  if (Buffer->Memory) {
    i32 error = munmap(Buffer->Memory, Buffer->MemorySize);
    if (error != 0) {
      Buffer->Memory = NULL;
      return;
    }
  }
  Buffer->MemorySize = Buffer->Width * Buffer->Height * Buffer->BytesPerPixel;

  Buffer->Memory = mmap(NULL, Buffer->MemorySize, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (Buffer->Memory == MAP_FAILED) {
    return;
  }
}

internal void ResizeDIBSection(BitmapBuffer *Buffer, SDL_Renderer *Renderer,
                               SDL_Texture **Texture) {
  AllocateBitmap(Buffer);
  if (*Texture) {
    SDL_DestroyTexture(*Texture);
  }
  *Texture = SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_ARGB8888,
                               SDL_TEXTUREACCESS_STREAMING, Buffer->Width,
                               Buffer->Height);
}

internal void UpdateWindow(BitmapBuffer Buffer, SDL_Renderer *renderer,
                           SDL_Texture *Texture) {
  SDL_UpdateTexture(Texture, NULL, Buffer.Memory,
                    Buffer.Width * Buffer.BytesPerPixel);
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, Texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

internal void HandleEvent(BitmapBuffer *Buffer, SDL_Window *window,
                          SDL_Renderer *Renderer, SDL_Texture **Texture,
                          SDL_Event event) {
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
      ResizeDIBSection(Buffer, Renderer, Texture);
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
  AllocateBitmap(&GlobalBackBuffer);
  while (Running) {
    SDL_Event Event;
    while (SDL_PollEvent(&Event)) {
      HandleEvent(&GlobalBackBuffer, Window, Renderer, &Texture, Event);
    }
    UpdateWindow(GlobalBackBuffer, Renderer, Texture);
    RenderWeirdGradiant(GlobalBackBuffer, x, y);
    ++x;
    ++y;
  }
  return 0;
}
