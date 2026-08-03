#if defined(__linux__)
#include "handmade.cpp"
#include <link.h>
#include <stdio.h>
#include <sys/mman.h>
#include <x86intrin.h>

#include <SDL2/SDL.h>

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

// TODO: i need to use a diffrent function that thois wrapper if i want to do
// the allocation myself
internal DEBUG_File_Slice DEBUGPlatformReadEntireFile(const char *FileName) {
  // NOTE: The data is allocated with a zero byte at the end (null terminated)
  // for convenience.(from the sdl docs)
  DEBUG_File_Slice File = {};
  size_t Size;
  File.Memory = SDL_LoadFile(FileName, &Size);
  File.MemorySize = (u32)Size;
  return File;
}
internal void DEBUGPlatformFreeEntireFile(void *Memory) { SDL_free(Memory); }
internal void DEBUGPlatformWriteEntireFile(const char *FileName,
                                           DEBUG_File_Slice File) {
  SDL_RWops *Handle = SDL_RWFromFile(FileName, "w");
  // TODO: handle the failure case correctly this just logs for the moment
  if (!Handle) {
    fprintf(stderr, "[ERR] Failed to open file for writing: %s\n",
            SDL_GetError());
    return;
  }
  SDL_RWwrite(Handle, File.Memory, sizeof(u8), File.MemorySize);
  SDL_RWclose(Handle);
}

// internal void AllocateBitmap(GameMemory *Memory, BitmapBuffer *Buffer) {
internal void AllocateBitmap(Bitmap_Buffer *Buffer) {
  Buffer->Width = WINDOW_WIDTH;
  Buffer->Height = WINDOW_HEIGHT;
  Buffer->MemorySize = Buffer->Width * Buffer->Height * Buffer->BytesPerPixel;

  // TODO: use the Memory of the program instead of a new alloc
  Buffer->Memory = mmap(NULL, Buffer->MemorySize, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (Buffer->Memory == MAP_FAILED) {
    return;
  }
}

internal void ResizeDIBSection(Bitmap_Buffer *Buffer, SDL_Renderer *Renderer,
                               SDL_Texture **Texture) {
  AllocateBitmap(Buffer);
  if (*Texture) {
    SDL_DestroyTexture(*Texture);
  }
  *Texture = SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_ARGB8888,
                               SDL_TEXTUREACCESS_STREAMING, Buffer->Width,
                               Buffer->Height);
}

internal void UpdateWindow(Bitmap_Buffer Buffer, SDL_Renderer *renderer,
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
  Game_State *GameState =
      (Game_State *)(((Game_Memory *)userdata)->PermanentStorage);
  Audio_State AudioState = {};
  AudioState.SampleOut = (i16 *)stream;
  AudioState.SampleCount = len / sizeof(i16);
  GameSoundOutput(GameState, AudioState);
}

//------------------------Event--------------------------------------------
internal void HandleEvent(Bitmap_Buffer *Buffer, SDL_Window *window,
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

  Game_Memory Memory = {};
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

  SDL_AudioSpec AudioDesired = {};
  AudioDesired.freq = AUDIO_FREQ;
  AudioDesired.format = AUDIO_FORMAT;
  AudioDesired.channels = AUDIO_CHANNELS;
  AudioDesired.samples = AUDIO_SAMPLES;
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

  Bitmap_Buffer GlobalBackBuffer = {};
  GlobalBackBuffer.BytesPerPixel = 4;
  AllocateBitmap(&GlobalBackBuffer);

  SDL_DisplayMode Mode;
  int DisplayIndex = SDL_GetWindowDisplayIndex(Window);
  if (SDL_GetCurrentDisplayMode(DisplayIndex, &Mode)) {
    fprintf(stderr, "[ERR] Failed to get display mode: %s\n", SDL_GetError());
  }
  i32 MonitorRefreshRate = (Mode.refresh_rate > 0) ? Mode.refresh_rate : 60;
  i32 GameRefreshRate = MonitorRefreshRate / 2;
  f32 TargetMilliSecondsPerFrame = 1000.0f / (f32)GameRefreshRate;
  u64 PrevTickTime = SDL_GetTicks64();

  while (Running) {
    u64 CurrentTickTime = SDL_GetTicks64();
    f32 Dt = (f32)(CurrentTickTime - PrevTickTime) / 1000.0f;
    PrevTickTime = CurrentTickTime;

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
    f32 ElapsedSeconds = (f32)(EndCounter - StartCounter) / (f32)Frequency;
    f64 RealFps = 1.0 / ElapsedSeconds;
    u64 TotalCyclesPerFrame = EndCycle - StartCycle;

    f32 FrameTime = (f32)(SDL_GetTicks64() - CurrentTickTime);
    if (FrameTime < TargetMilliSecondsPerFrame) {
      //TODO: Sleep for most of the remaining time, then spin-wait (busy-loop) for the last ~1-2ms for precision:
      u32 Delay = (u32)(TargetMilliSecondsPerFrame - FrameTime);
      SDL_Delay(Delay);
    } else {
      // TODO: frame missed
    }

    f64 VSyncFps = 1.0f / Dt;
    printf("REAL FPS: %f MCycles per frame: %lu FAKE FPS: %f \n", RealFps,
           TotalCyclesPerFrame / (1000 * 1000), VSyncFps);
  }
  // NOTE: not sure if the quit is nessecary since the os does the cleanup but
  // its here for now
  SDL_Quit();
  return 0;
}
#endif
