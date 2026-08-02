#if defined(__linux__)
#include <link.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <x86intrin.h>

#include "handmade.cpp"
#include <SDL2/SDL.h>

global_variable i32 WINDOW_WIDTH = 1800;
global_variable i32 WINDOW_HEIGHT = 900;
global_variable b32 Running = true;

#if !HANDMADE_INTERNAL
internal int BaseAddressCallback(struct dl_phdr_info *info, size_t size,
                                 void *data) {
  u64 *base = (u64 *)data;

  if (info->dlpi_name == NULL || info->dlpi_name[0] == '\0') {
    *base = (u64)info->dlpi_addr;
    return 1;
  }
  return 0;
}
#endif

// TODO: allocate soemthing for the biggest screen so we dont have to reallocate
// when we resize
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
  GameState *GState = (GameState *)(((GameMemory *)userdata)->PermanentStorage);
  AudioState AState = {};
  AState.SampleOut = (i16 *)stream;
  AState.SampleCount = len / sizeof(i16);
  GameSoundOutput(GState, AState);
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

int main() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
    fprintf(stderr, "failed to init SDL\n");
    return 1;
  }

  void *BaseAddress = (void *)Terabytes(2);
#if !HANDMADE_INTERNAL
  *BaseAddress = 0;
#endif

  GameMemory Memory = {};
  Memory.PermanentStorageSize = Megabytes(64);
  Memory.TransiantStorageSize = Gigabytes(4);
  u64 TotalSize = Memory.PermanentStorageSize + Memory.TransiantStorageSize;
  Memory.PermanentStorage = mmap(BaseAddress, TotalSize, PROT_READ | PROT_WRITE,
                                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  Memory.TransiantStorage =
      (u8 *)Memory.PermanentStorage + Memory.PermanentStorageSize;
  if (Memory.PermanentStorage == MAP_FAILED) {
    fprintf(stderr, "[ERR] Failed initilization: Memory Allocation\n");
    return 1;
  }

  SDL_Window *Window = SDL_CreateWindow("handmade hero", SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,
                                        WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
  SDL_Renderer *Renderer = SDL_CreateRenderer(Window, -1, 0);
  SDL_Texture *Texture = SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_ARGB8888,
                                           SDL_TEXTUREACCESS_STREAMING,
                                           WINDOW_WIDTH, WINDOW_HEIGHT);

  // TODO: extract these params to the platform layer
  SDL_AudioSpec AudioDesired = {};
  AudioDesired.freq = 48000;
  AudioDesired.format = AUDIO_S16;
  AudioDesired.channels = 2;
  AudioDesired.samples = Kilobytes(4);
  AudioDesired.callback = AudioCallback;
  AudioDesired.userdata = (void *)&Memory;

  SDL_AudioSpec AudioObtained;
  SDL_AudioDeviceID AudioDeviceID = SDL_OpenAudioDevice(
      NULL, 0, &AudioDesired, &AudioObtained, SDL_AUDIO_ALLOW_ANY_CHANGE);

  if (Window == NULL || Renderer == NULL || AudioDeviceID == 0) {
    fprintf(stderr, "[ERR] Failed initilization: %s\n", SDL_GetError());
    return 1;
  }

  const u8 *KeyboardState = SDL_GetKeyboardState(NULL);
  b32 IsAudioPaused = true;

  BitmapBuffer GlobalBackBuffer = {};
  GlobalBackBuffer.BytesPerPixel = 4;
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
    UpdateWindow(GlobalBackBuffer, Renderer, Texture);
    GameUpdate(&Memory, GlobalBackBuffer, KeyboardState);

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
#endif
