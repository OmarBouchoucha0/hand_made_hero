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
global_variable i32 WINDOW_WIDTH = 1600;
global_variable i32 WINDOW_HEIGHT = 900;

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

SDL_RWops *DEBUGPlatformGetAppendFileHandle(const char *FileName) {
  SDL_RWops *Handle = SDL_RWFromFile(FileName, "a");
  if (!Handle) {
    fprintf(stderr, "[ERR] Failed to open file for writing: %s\n",
            SDL_GetError());
  }
  return Handle;
}

SDL_RWops *DEBUGPlatformGetReadFileHandle(const char *FileName) {
  SDL_RWops *Handle = SDL_RWFromFile(FileName, "rb");
  if (!Handle) {
    fprintf(stderr, "[ERR] Failed to open file for writing: %s\n",
            SDL_GetError());
  }
  return Handle;
}

void DEBUGPlatformAppendToFile(SDL_RWops *Handle, void *Memory,
                               u32 MemorySize) {
  SDL_RWwrite(Handle, Memory, sizeof(u8), MemorySize);
}

void DEBUGPlatformCloseFile(SDL_RWops *Handle) { SDL_RWclose(Handle); }

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
  const char *FileName;
  SDL_RWops *InputRecordingFileHanlde;
  u64 InputRecordingIndex;
  SDL_RWops *InputPlaybackFileHanlde;
  u64 InputPlaybackIndex;
};

internal void BeginInputRecording(Game_State *GameState,
                                  Input_Replay_State *InputReplayState) {

  if (GameState->Recording == false) {
    GameState->Recording = true;
    InputReplayState->InputRecordingIndex = 0;
    InputReplayState->InputRecordingFileHanlde =
        DEBUGPlatformGetAppendFileHandle(InputReplayState->FileName);
  }
}

internal void StopInputRecording(Game_State *GameState,
                                 Input_Replay_State *InputReplayState) {

  if (GameState->Recording == true) {
    DEBUGPlatformCloseFile(InputReplayState->InputRecordingFileHanlde);
    GameState->Recording = false;
  }
}

internal void RecordInput(Input_Replay_State *InputReplayState,
                          Game_Input *GameInput) {
  DEBUGPlatformAppendToFile(InputReplayState->InputRecordingFileHanlde,
                            (void *)GameInput, sizeof(Game_Input));
  ++InputReplayState->InputRecordingIndex;
}

internal void BeginInputPlayback(Game_State *GameState,
                                 Input_Replay_State *InputReplayState) {

  if (GameState->Playback == false) {
    GameState->Playback = true;
    InputReplayState->InputPlaybackIndex = 0;
    InputReplayState->InputPlaybackFileHanlde =
        DEBUGPlatformGetReadFileHandle(InputReplayState->FileName);
  }
}

internal void StopInputPlayback(Game_State *GameState,
                                Input_Replay_State *InputReplayState) {
  if (GameState->Playback == true) {
    DEBUGPlatformCloseFile(InputReplayState->InputPlaybackFileHanlde);
    GameState->Playback = false;
  }
}

internal Game_Input PlaybackInput(Game_State *GameState,
                                  Input_Replay_State *InputReplayState) {
  Game_Input GameInput = {};
  i64 Offset = InputReplayState->InputPlaybackIndex * sizeof(Game_Input);
  i64 Result = SDL_RWseek(InputReplayState->InputPlaybackFileHanlde, Offset,
                          RW_SEEK_SET);
  if (Result == -1) {
    fprintf(stderr, "[ERR] Seek failed: %s\n", SDL_GetError());
  }
  if (InputReplayState->InputPlaybackIndex <
      InputReplayState->InputRecordingIndex) {
    u64 ChunksRead = SDL_RWread(InputReplayState->InputPlaybackFileHanlde,
                                &GameInput, sizeof(Game_Input), 1);
  } else {
    StopInputPlayback(GameState, InputReplayState);
    InputReplayState->InputRecordingIndex = 0;
    return GameInput;
  }
  ++InputReplayState->InputPlaybackIndex;
  return GameInput;
}

//------------------------Input--------------------------------------
internal void MapKeyboardToInput(const u8 *KeyboardState,
                                 Game_Input *GameInput) {
  if (KeyboardState[SDL_SCANCODE_RIGHT] || KeyboardState[SDL_SCANCODE_D]) {
    GameInput->Right = true;
  }
  if (KeyboardState[SDL_SCANCODE_LEFT] || KeyboardState[SDL_SCANCODE_A]) {
    GameInput->Left = true;
  }
  if (KeyboardState[SDL_SCANCODE_UP] || KeyboardState[SDL_SCANCODE_W]) {
    GameInput->Up = true;
  }
  if (KeyboardState[SDL_SCANCODE_DOWN] || KeyboardState[SDL_SCANCODE_S]) {
    GameInput->Down = true;
  }
}

internal void MapJoystickToInput(SDL_GameController *GameController,
                                 Game_Input *GameInput) {

  if (SDL_GameControllerGetButton(GameController,
                                  SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) {
    GameInput->Right = true;
  }
  if (SDL_GameControllerGetButton(GameController,
                                  SDL_CONTROLLER_BUTTON_DPAD_LEFT)) {
    GameInput->Left = true;
  }
  if (SDL_GameControllerGetButton(GameController,
                                  SDL_CONTROLLER_BUTTON_DPAD_UP)) {
    GameInput->Up = true;
  }
  if (SDL_GameControllerGetButton(GameController,
                                  SDL_CONTROLLER_BUTTON_DPAD_DOWN)) {
    GameInput->Down = true;
  }

  i16 StickX =
      SDL_GameControllerGetAxis(GameController, SDL_CONTROLLER_AXIS_LEFTX);
  i16 StickY =
      SDL_GameControllerGetAxis(GameController, SDL_CONTROLLER_AXIS_LEFTY);
  const i16 DeadZone = 8000;

  if (StickX > DeadZone) {
    GameInput->Right = true;
  }
  if (StickX < (-DeadZone)) {
    GameInput->Left = true;
  }
  if (StickY < (-DeadZone)) {
    GameInput->Up = true;
  }
  if (StickY > DeadZone) {
    GameInput->Down = true;
  }
}

//------------------------Audio--------------------------------------------
internal void ToggleAudio(SDL_AudioDeviceID AudioDeviceID, b32 *AudioPause) {
  *AudioPause = !*AudioPause;
  SDL_PauseAudioDevice(AudioDeviceID, *AudioPause);
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
                          Game_State *GameState,
                          Input_Replay_State *InputReplayState,
                          SDL_GameController **GameController,
                          i32 NumberJoysticks) {
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
      ToggleAudio(AudioDeviceID, &GameState->AudioPause);
    }
    if (event.key.repeat == 0 && event.key.keysym.scancode == SDL_SCANCODE_P) {
      GameState->GlobalPause = !GameState->GlobalPause;
    }
    if (event.key.repeat == 0 && event.key.keysym.scancode == SDL_SCANCODE_L) {
      if (InputReplayState->InputRecordingIndex == 0) {
        BeginInputRecording(GameState, InputReplayState);
        StopInputPlayback(GameState, InputReplayState);
      } else {
        StopInputRecording(GameState, InputReplayState);
        BeginInputPlayback(GameState, InputReplayState);
      }
    }
  } break;
    // NOTE: only 1 controller allowed
  case SDL_CONTROLLERDEVICEADDED: {
    if (NumberJoysticks == 0) {
      SDL_GameController *NewController = SDL_GameControllerOpen(0);
      NumberJoysticks = SDL_NumJoysticks();
      if (NewController) {
        printf("[INFO] controller connected\n");
        *GameController = NewController;
      } else {
        fprintf(stderr, "[ERR] Failed to open controller: %s\n",
                SDL_GetError());
      }
    }
  } break;
  case SDL_CONTROLLERDEVICEREMOVED: {
    if (NumberJoysticks == 1) {
      printf("[INFO] controller disconnected\n");
      if (*GameController) {
        SDL_GameControllerClose(*GameController);
        *GameController = NULL;
        NumberJoysticks = SDL_NumJoysticks();
      }
    }
  } break;
  }
}

int main() {
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

  Game_State *GameState = {};

  // TODO: hardcoded path
  const char *GameLibPath = "./out/handmade.so";
  Game_Code GameCode = LoadGameCode(GameLibPath);
  GameCode.LastWriteTime = GetLastWriteTime(GameLibPath);

  Game_Logic_And_State GameLogicAndState = {};
  GameLogicAndState.GameCode = GameCode;
  GameLogicAndState.Memory = Memory;

  Bitmap_Buffer GlobalBackBuffer = {};
  GlobalBackBuffer.BytesPerPixel = 4;
  AllocateBitmap(&GlobalBackBuffer);

  Input_Replay_State InputReplayState = {};
  InputReplayState.FileName = "./assets/test.rpf";

  Game_Input GameInput = {};
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) !=
      0) {
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
  SDL_PauseAudioDevice(AudioDeviceID, true);

  SDL_GameController *GameController = {};
  i32 NumberJoysticks = SDL_NumJoysticks();
  if (NumberJoysticks > 0) {
    GameController = SDL_GameControllerOpen(0);
  }

  if (Window == NULL || Renderer == NULL || AudioDeviceID == 0 ||
      NumberJoysticks < 0) {
    fprintf(stderr, "[ERR] Failed initilization: %s\n", SDL_GetError());
    return 1;
  }

  SDL_DisplayMode Mode;
  int DisplayIndex = SDL_GetWindowDisplayIndex(Window);
  if (SDL_GetCurrentDisplayMode(DisplayIndex, &Mode)) {
    fprintf(stderr, "[ERR] Failed to get display mode: %s\n", SDL_GetError());
  }

  const u8 *KeyboardState = SDL_GetKeyboardState(NULL);
  b32 IsAudioPaused = true;

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
      // TODO: this is beign read by sdl's audio thread so updating it may cause
      // a race condition
      GameLogicAndState.GameCode = GameCode;
      printf("[INFO] game code reloaded\n");
    }

    SDL_Event Event;
    while (SDL_PollEvent(&Event)) {
      HandleEvent(&GlobalBackBuffer, Window, Renderer, &Texture, Event,
                  AudioDeviceID, GameState, &InputReplayState, &GameController,
                  NumberJoysticks);
    }
    UpdateWindow(GlobalBackBuffer, Renderer, Texture);

    if (GameCode.IsValid) {
      GameInput = {};
      if (NumberJoysticks > 0) {
        MapJoystickToInput(GameController, &GameInput);
      }
      MapKeyboardToInput(KeyboardState, &GameInput);
      GameState = (Game_State *)Memory.PermanentStorage;
      if (GameState->Playback) {
        GameInput = PlaybackInput(GameState, &InputReplayState);
      }
      GameCode.Update(&Memory, &GlobalBackBuffer, &GameInput);
      if (GameState->Recording) {
        RecordInput(&InputReplayState, &GameInput);
      }
    }

    u64 EndCounter = SDL_GetPerformanceCounter();
    u64 Frequency = SDL_GetPerformanceFrequency();
    f32 ElapsedMS =
        ((f32)(EndCounter - StartCounter) / (f32)Frequency) * 1000.0f;
    f64 RealFps = 1000.0f / ElapsedMS;

    if (ElapsedMS < TargetMilliSecondsPerFrame) {
      // NOTE: we give the os time to wake up with the 1 ms delay, we still
      // have some occasional spikes not sure why
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
