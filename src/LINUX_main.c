#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <x86intrin.h>

#define global_variable static
#define local_persist static
#define internal static
#define PI 3.14159265358979323846f

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

typedef i32 b32;

#include "handmade.cpp"
#include <SDL2/SDL.h>

global_variable i32 WINDOW_WIDTH = 99999;
global_variable i32 WINDOW_HEIGHT = 99999;
global_variable b32 Running = true;

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

//------------------------Audio--------------------------------------------
void ToggleAudio(SDL_AudioDeviceID AudioDeviceID, b32 *IsAudioPaused) {
  *IsAudioPaused = !*IsAudioPaused;
  SDL_PauseAudioDevice(AudioDeviceID, *IsAudioPaused);
}

void AudioCallback(void *userdata, u8 *stream, i32 len) {

  AudioState *State = (AudioState *)userdata;
  State->SampleOut = (i16 *)stream;
  State->SampleCount = len / sizeof(i16);
  GameSoundOutput(State);
}

//------------------------Event--------------------------------------------
internal void HandleEvent(BitmapBuffer *Buffer, SDL_Window *window,
                          SDL_Renderer *Renderer, SDL_Texture **Texture,
                          SDL_Event event, SDL_AudioDeviceID AudioDeviceID,
                          b32 *IsAudioPaused) {
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
  case SDL_KEYDOWN: {
    if (event.key.repeat == 0 &&
        event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
      ToggleAudio(AudioDeviceID, IsAudioPaused);
    }
  } break;
  }
}

//------------------------Movment--------------------------------------------
void Movement(const u8 *KeyboardState, i32 *x, i32 *y) {
  if (KeyboardState[SDL_SCANCODE_RIGHT] || KeyboardState[SDL_SCANCODE_D]) {
    *x += 1;
  }
  if (KeyboardState[SDL_SCANCODE_LEFT] || KeyboardState[SDL_SCANCODE_A]) {
    *x -= 1;
  }
  if (KeyboardState[SDL_SCANCODE_UP] || KeyboardState[SDL_SCANCODE_W]) {
    *y += 1;
  }
  if (KeyboardState[SDL_SCANCODE_DOWN] || KeyboardState[SDL_SCANCODE_S]) {
    *y -= 1;
  }
}

int main() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
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

  AudioState State = {.SamplesPerSecond = 48000, .ToneHz = 100.0f};
  const SDL_AudioSpec AudioDesired = {.freq = 48000,
                                      .format = AUDIO_S16,
                                      .channels = 2,
                                      .samples = 4096,
                                      .callback = AudioCallback,
                                      .userdata = &State};
  SDL_AudioSpec AudioObtained;
  const char *Device = SDL_GetAudioDeviceName(1, 0);
  SDL_AudioDeviceID AudioDeviceID = SDL_OpenAudioDevice(
      Device, 0, &AudioDesired, &AudioObtained, SDL_AUDIO_ALLOW_ANY_CHANGE);

  if (Window == NULL || Renderer == NULL || AudioDeviceID == 0) {
    fprintf(stderr, "[ERR] Failed initilization: %s\n", SDL_GetError());
    return 1;
  }

  const u8 *KeyboardState = SDL_GetKeyboardState(NULL);
  b32 IsAudioPaused = true;

  i32 x = 0;
  i32 y = 0;

  BitmapBuffer GlobalBackBuffer = (BitmapBuffer){.BytesPerPixel = 4};
  AllocateBitmap(&GlobalBackBuffer);

  while (Running) {
    u64 StartCounter = SDL_GetPerformanceCounter();
    u64 StartCycle = __rdtsc();
    //--------------------------the start of a frame--------------

    SDL_Event Event;
    while (SDL_PollEvent(&Event)) {
      HandleEvent(&GlobalBackBuffer, Window, Renderer, &Texture, Event,
                  AudioDeviceID, &IsAudioPaused);
    }
    Movement(KeyboardState, &x, &y);
    UpdateWindow(GlobalBackBuffer, Renderer, Texture);
    GameRender(GlobalBackBuffer, x, y);

    //--------------------------the end of a frame----------------
    u64 EndCycle = __rdtsc();
    u64 EndCounter = SDL_GetPerformanceCounter();
    u64 Frequency = SDL_GetPerformanceFrequency();
    f64 ElapsedSeconds = (f64)(EndCounter - StartCounter) / (f64)Frequency;
    f64 fps = 1.0 / ElapsedSeconds;
    u64 TotalCyclesPerFrame = EndCycle - StartCycle;
    printf("FPS: %f MCycles per frame: %lu \n", fps,
           TotalCyclesPerFrame / (1000 * 1000));
  }
  return 0;
}
