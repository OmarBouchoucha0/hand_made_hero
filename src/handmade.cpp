#include "handmade.h"

internal void GameRender(Game_State *GameState, Bitmap_Buffer *Buffer) {

  int pitch = Buffer->Width * Buffer->BytesPerPixel;
  u8 *row = (u8 *)Buffer->Memory;
  for (i32 y = 0; y < Buffer->Height; ++y) {
    u32 *pixel = (u32 *)row;
    for (i32 x = 0; x < Buffer->Width; ++x) {
      u8 red = (u8)(x - GameState->XOffset);
      u8 green = 0;
      u8 blue = (u8)(y + GameState->YOffset);
      *pixel++ = (red << 16 | green << 8 | blue);
    }
    row += pitch;
  }
}

internal void PlayerRender(Game_State *GameState, Bitmap_Buffer *Buffer) {
  int pitch = Buffer->Width * Buffer->BytesPerPixel;
  u8 *row = (u8 *)Buffer->Memory;
  i32 PlayerHeight = 20;
  i32 PlayerWidth = 20;
  for (i32 y = 0; y < Buffer->Height; ++y) {
    u32 *pixel = (u32 *)row;
    for (i32 x = 0; x < Buffer->Width; ++x) {
      if ((x >= GameState->PlayerX && x < GameState->PlayerX + PlayerWidth) &&
          (y >= GameState->PlayerY && y < GameState->PlayerY + PlayerHeight)) {
        u8 red = 255;
        u8 green = 255;
        u8 blue = 255;
        *pixel++ = (red << 16 | green << 8 | blue);
      } else {
        pixel++;
      }
    }
    row += pitch;
  }
}

internal void GameMovement(Game_State *GameState, Game_Input *GameInput) {
  if (GameInput->Right) {
    ++GameState->PlayerX;
  }
  if (GameInput->Left) {
    --GameState->PlayerX;
  }
  if (GameInput->Up) {
    --GameState->PlayerY;
  }
  if (GameInput->Down) {
    ++GameState->PlayerY;
  }
}

extern "C" void GameSoundOutput(Game_State *GameState, Audio_State AudioState) {
  i16 right = 0;
  i16 left = 0;
  i32 Amp = 6000;
  f32 AngleIncrement =
      2.0f * PI * GameState->ToneHz / (f32)GameState->SamplesPerSecond;
  for (i32 i = 0; i < AudioState.SampleCount; i += 2) {
    left = (i16)((f32)Amp * sinf(GameState->TSine));
    right = left;
    *AudioState.SampleOut++ = left;
    *AudioState.SampleOut++ = right;
    GameState->TSine += AngleIncrement;
    if (GameState->TSine > 2.0f * PI) {
      GameState->TSine -= 2.0f * PI;
    }
  }
}

extern "C" void GameUpdate(Game_Memory *Memory, Bitmap_Buffer *Buffer,
                           Game_Input *GameInput) {
  Assert(sizeof(Game_State) <= Memory->PermanentStorageSize);
  Game_State *GameState = (Game_State *)Memory->PermanentStorage;
  if (!Memory->IsInitialised) {
    *GameState = {};

    GameState->PlayerX = Buffer->Height / 2;
    GameState->PlayerY = Buffer->Width / 2;

    GameState->SamplesPerSecond = 48000;
    GameState->ToneHz = 100.0f;

    Memory->IsInitialised = true;
  }
  if (!GameState->GlobalPause) {
    GameRender(GameState, Buffer);
    PlayerRender(GameState, Buffer);
    GameMovement(GameState, GameInput);
  }
}
