#include "handmade.h"

internal void GameRender(Game_State *GameState, Bitmap_Buffer Buffer) {

  int pitch = Buffer.Width * Buffer.BytesPerPixel;
  u8 *row = (u8 *)Buffer.Memory;

  for (i32 y = 0; y < Buffer.Height; ++y) {
    u32 *pixel = (u32 *)row;
    for (i32 x = 0; x < Buffer.Width; ++x) {
      u8 red = (u8)(x - GameState->XOffset);
      u8 green = 0;
      u8 blue = (u8)(y + GameState->YOffset);
      *pixel++ = (red << 16 | green << 8 | blue);
    }
    row += pitch;
  }
}

internal void GameMovement(Game_State *GameState, const u8 *KeyboardState) {
  if (KeyboardState[SCANCODE_RIGHT] || KeyboardState[SCANCODE_D]) {
    ++GameState->XOffset;
  }
  if (KeyboardState[SCANCODE_LEFT] || KeyboardState[SCANCODE_A]) {
    --GameState->XOffset;
  }
  if (KeyboardState[SCANCODE_UP] || KeyboardState[SCANCODE_W]) {
    ++GameState->YOffset;
  }
  if (KeyboardState[SCANCODE_DOWN] || KeyboardState[SCANCODE_S]) {
    --GameState->YOffset;
  }
}

extern "C" void GameSoundOutput(Game_State *GameState, Audio_State AudioState) {
  i16 right = 0;
  i16 left = 0;
  i32 Amp = 6000;
  f32 Angle = 0;
  for (i32 i = 0; i < AudioState.SampleCount; i += 2) {
    Angle = 2.0f * PI * (f32)GameState->ToneHz * (f32)GameState->SampleIndex /
            (f32)GameState->SamplesPerSecond;
    left = (i16)((f32)Amp * sinf(Angle));
    right = (i16)((f32)Amp * sinf(Angle));
    *AudioState.SampleOut++ = left;
    *AudioState.SampleOut++ = right;
    ++GameState->SampleIndex;
  }
}

extern "C" void GameUpdate(Game_Memory *Memory, Bitmap_Buffer Buffer,
                           const u8 *KeyboardState) {
  Assert(sizeof(Game_State) <= Memory->PermanentStorageSize);
  Game_State *State = (Game_State *)Memory->PermanentStorage;
  if (!Memory->IsInitialised) {
    State->XOffset = 0;
    State->YOffset = 0;

    State->SamplesPerSecond = 48000;
    State->ToneHz = 100.0f;
    State->SampleIndex = 0;

    Memory->IsInitialised = true;
  }
  GameRender(State, Buffer);
  // TODO : we are updating the movement every frame its better to use dt when i
  // implement that
  GameMovement(State, KeyboardState);
}
