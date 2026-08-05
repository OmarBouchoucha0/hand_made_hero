#if defined(__linux__)
#include "handmade.h"
#include <dlfcn.h>
#include <link.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <x86intrin.h>

#include <SDL2/SDL.h>

global_variable b32 Running = true;
global_variable i32 WINDOW_WIDTH;
global_variable i32 WINDOW_HEIGHT;

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

//--------------------------dynamic library loading---------------------
typedef void (*Game_UpdateFn)(Game_Memory *, Bitmap_Buffer *, Game_Input *);
typedef void (*Game_SoundOutputFn)(Game_State *GameState,
                                   Audio_State AudioState);

struct Game_Code {
  void *Handle;
  Game_UpdateFn Update;
  Game_SoundOutputFn Sound;
  time_t LastWriteTime;
  bool IsValid;
};

struct Game_Logic_And_State {
  Game_Code GameCode;
  Game_Memory Memory;
};

internal time_t GetLastWriteTime(const char *Path) {
  struct stat FileStat;
  if (stat(Path, &FileStat) == 0) {
    return FileStat.st_mtime;
  }
  return 0;
}

internal Game_Code LoadGameCode(const char *Path) {
  Game_Code Result = {};
  Result.Handle = dlopen(Path, RTLD_NOW);
  if (!Result.Handle) {
    fprintf(stderr, "[ERR] dlopen failed: %s\n", dlerror());
    return Result;
  }
  Result.Update = (Game_UpdateFn)dlsym(Result.Handle, "GameUpdate");
  if (!Result.Update) {
    fprintf(stderr, "[ERR] dlsym GameUpdate failed: %s\n", dlerror());
  }

  Result.Sound = (Game_SoundOutputFn)dlsym(Result.Handle, "GameSoundOutput");
  if (!Result.Sound) {
    fprintf(stderr, "[ERR] dlsym GameSoundOutput failed: %s\n", dlerror());
  }
  Result.IsValid = (Result.Update && Result.Sound);

  return Result;
}

internal void UnloadGameCode(Game_Code *Game) {
  if (Game->Handle) {
    dlclose(Game->Handle);
  }
  *Game = {};
}

//-----------------------------file loading------------------------------
// TODO: i need to use a diffrent function than this wrapper if i want to do
// the allocation myself
DEBUG_File_Slice DEBUGPlatformReadEntireFile(const char *FileName) {
  // NOTE: The data is allocated with a zero byte at the end (null terminated)
  // for convenience.(from the sdl docs)
  DEBUG_File_Slice File = {};
  size_t Size;
  File.Memory = SDL_LoadFile(FileName, &Size);
  File.MemorySize = (u32)Size;
  if (File.Memory == NULL) {
    fprintf(stderr, "[ERR] Failed to open file\n");
  }
  return File;
}
void DEBUGPlatformFreeEntireFile(DEBUG_File_Slice *File) {
  SDL_free(File->Memory);
}
FILE_WRITE_STATUS DEBUGPlatformWriteEntireFile(const char *FileName,
                                               DEBUG_File_Slice *File) {
  SDL_RWops *Handle = SDL_RWFromFile(FileName, "w");
  if (!Handle) {
    fprintf(stderr, "[ERR] Failed to open file for writing: %s\n",
            SDL_GetError());
    return FILE_WRITE_FAIL;
  }
  SDL_RWwrite(Handle, File->Memory, sizeof(u8), File->MemorySize);
  SDL_RWclose(Handle);
  return FILE_WRITE_SUCCES;
}

//------------------------utils-----------------------------------------
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
//-----------------------Recording------------------------------
struct Input_Replay_State {
  DEBUG_File_Slice InputRecordingFile;
  i32 InputRecordingIndex;
  DEBUG_File_Slice InputPlayBackFile;
  i32 InputPlayBackIndex;
};

// TODO: write new input
internal void StartReplayLoop(Input_Replay_State *InputReplayState,
                              i32 InputRecordingIndex) {
  const char *FileName = "test.rls";
  InputReplayState->InputRecordingFile = DEBUGPlatformReadEntireFile(FileName);
  if (InputReplayState->InputRecordingFile.Memory == NULL) {
    return;
  }
  FILE_WRITE_STATUS FileWriteStatus = DEBUGPlatformWriteEntireFile(
      FileName, &InputReplayState->InputRecordingFile);
  if (FileWriteStatus == FILE_WRITE_FAIL) {
    return;
  }

  InputReplayState->InputRecordingIndex = InputRecordingIndex;
}

internal void StopReplayLoop(Input_Replay_State *InputReplayState) {
  DEBUGPlatformFreeEntireFile(&InputReplayState->InputRecordingFile);
}

internal void ReplayLoop(Input_Replay_State *InputReplayState,
                         i32 InputRecordingIndex) {
  StartReplayLoop(InputReplayState, InputRecordingIndex);
  if (InputReplayState->InputRecordingFile.Memory == NULL) {
    return;
  }
  StopReplayLoop(InputReplayState);
}

internal void StartPlayBackLoop(Input_Replay_State *InputReplayState,
                                i32 InputPlayBackIndex) {
  const char *FileName = "test.rls";
  InputReplayState->InputPlayBackFile = DEBUGPlatformReadEntireFile(FileName);
  InputReplayState->InputPlayBackIndex = InputPlayBackIndex;
}

internal void StopPlayBackLoop(Input_Replay_State *InputReplayState) {
  DEBUGPlatformFreeEntireFile(&InputReplayState->InputPlayBackFile);
}
//------------------------Input--------------------------------------

internal Game_Input MapKeyboardToInput(const u8 *KeyboardState) {
  Game_Input GameInput = {};
  if (KeyboardState[SDL_SCANCODE_RIGHT] || KeyboardState[SDL_SCANCODE_D]) {
    GameInput.Right = true;
  }
  if (KeyboardState[SDL_SCANCODE_LEFT] || KeyboardState[SDL_SCANCODE_A]) {
    GameInput.Left = true;
  }
  if (KeyboardState[SDL_SCANCODE_UP] || KeyboardState[SDL_SCANCODE_W]) {
    GameInput.Up = true;
  }
  if (KeyboardState[SDL_SCANCODE_DOWN] || KeyboardState[SDL_SCANCODE_S]) {
    GameInput.Down = true;
  }
  return GameInput;
}

//------------------------Audio--------------------------------------------
internal void ToggleAudio(SDL_AudioDeviceID AudioDeviceID, b32 *IsAudioPaused) {
  *IsAudioPaused = !*IsAudioPaused;
  SDL_PauseAudioDevice(AudioDeviceID, *IsAudioPaused);
}

void AudioCallback(void *userdata, u8 *stream, i32 len) {
  Game_Logic_And_State *GameLogicAndState = (Game_Logic_And_State *)userdata;
  Game_State *GameState =
      (Game_State *)GameLogicAndState->Memory.PermanentStorage;
  Game_Code GameCode = GameLogicAndState->GameCode;
  Audio_State AudioState = {};
  AudioState.SampleOut = (i16 *)stream;
  AudioState.SampleCount = len / sizeof(i16);
  GameCode.Sound(GameState, AudioState);
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

    if (event.key.repeat == 0 && event.key.keysym.scancode == SDL_SCANCODE_L) {
        // TODO: loop
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
  Memory.TransiantStorageSize = Gigabytes(1);
  u64 TotalSize = Memory.PermanentStorageSize + Memory.TransiantStorageSize;
  Memory.PermanentStorage = mmap(BaseAddress, TotalSize, PROT_READ | PROT_WRITE,
                                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  Memory.TransiantStorage =
      (u8 *)Memory.PermanentStorage + Memory.PermanentStorageSize;
  if (Memory.PermanentStorage == MAP_FAILED) {
    fprintf(stderr, "[ERR] Failed initilization: Memory Allocation\n");
    return 1;
  }
  // TODO: hardcoded path
  const char *GameLibPath = "./out/handmade.so";
  Game_Code GameCode = LoadGameCode(GameLibPath);

  SDL_Window *Window = SDL_CreateWindow("handmade hero", SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,
                                        WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
  SDL_Renderer *Renderer = SDL_CreateRenderer(Window, -1, 0);
  SDL_Texture *Texture = SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_ARGB8888,
                                           SDL_TEXTUREACCESS_STREAMING,
                                           WINDOW_WIDTH, WINDOW_HEIGHT);
  Game_Logic_And_State GameLogicAndState = {};
  GameLogicAndState.GameCode = GameCode;
  GameLogicAndState.Memory = Memory;

  SDL_AudioSpec AudioDesired = {};
  AudioDesired.freq = AUDIO_FREQ;
  AudioDesired.format = AUDIO_FORMAT;
  AudioDesired.channels = AUDIO_CHANNELS;
  AudioDesired.samples = AUDIO_SAMPLES;
  AudioDesired.callback = AudioCallback;
  AudioDesired.userdata = (void *)&GameLogicAndState;

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
  i32 GameRefreshRate = MonitorRefreshRate / 1;
  f32 TargetMilliSecondsPerFrame = 1000.0f / (f32)GameRefreshRate;
  u64 PrevTickTime = SDL_GetTicks64();

  while (Running) {
    u64 CurrentTickTime = SDL_GetTicks64();
    f32 Dt = (f32)(CurrentTickTime - PrevTickTime) / 1000.0f;
    PrevTickTime = CurrentTickTime;

    u64 StartCounter = SDL_GetPerformanceCounter();
    u64 StartCycle = __rdtsc();

    time_t NewWriteTime = GetLastWriteTime(GameLibPath);
    if (NewWriteTime != GameCode.LastWriteTime) {
      UnloadGameCode(&GameCode);
      GameCode = LoadGameCode(GameLibPath);
      GameCode.LastWriteTime = NewWriteTime;
      printf("[INFO] game code reloaded\n");
    }

    SDL_Event Event;
    while (SDL_PollEvent(&Event)) {
      HandleEvent(&GlobalBackBuffer, Window, Renderer, &Texture, Event,
                  AudioDeviceID, &IsAudioPaused);
    }
    UpdateWindow(GlobalBackBuffer, Renderer, Texture);

    if (GameCode.IsValid) {
      Game_Input GameInput = MapKeyboardToInput(KeyboardState);
      GameCode.Update(&Memory, &GlobalBackBuffer, &GameInput);
    }

    u64 EndCounter = SDL_GetPerformanceCounter();
    u64 Frequency = SDL_GetPerformanceFrequency();
    f32 ElapsedMS =
        ((f32)(EndCounter - StartCounter) / (f32)Frequency) * 1000.0f;
    f64 RealFps = 1000.0f / ElapsedMS;

    if (ElapsedMS < TargetMilliSecondsPerFrame) {
      // NOTE: we give the os time to wake up with the 1 ms delay, we still have
      // some occasional spikes not sure why
      SDL_Delay((u32)(TargetMilliSecondsPerFrame - ElapsedMS - 1.0f));
      while (ElapsedMS < TargetMilliSecondsPerFrame) {
        EndCounter = SDL_GetPerformanceCounter();
        ElapsedMS =
            ((f32)(EndCounter - StartCounter) / (f32)Frequency) * 1000.0f;
      }
    } else {
      // TODO: frame missed
    }

    f64 VSyncFps = 1000.0f / ElapsedMS;
    f32 DelayMS = ElapsedMS - TargetMilliSecondsPerFrame;

    u64 EndCycle = __rdtsc();
    u64 TotalCyclesPerFrame = EndCycle - StartCycle;
    printf("[INFO] UNCAPPED FPS: %f VSYNC FPS: %f Delay : %f ms MCycles per "
           "frame: "
           "%lu  \n",
           RealFps, VSyncFps, DelayMS, TotalCyclesPerFrame / (1000 * 1000));
  }
  // NOTE: not sure if the quit is nessecary since the os does the cleanup but
  // its here for now
  UnloadGameCode(&GameCode);
  SDL_Quit();
  return 0;
}
#endif
