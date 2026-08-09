#include "handmade.h"
#include <stdio.h>

internal void GameRender(Game_State *GameState, Bitmap_Buffer *Buffer) {
  // int pitch = Buffer->Width * Buffer->BytesPerPixel;
  // u8 *row = (u8 *)Buffer->Memory;
  // for (i32 y = 0; y < Buffer->Height; ++y) {
  //   u32 *pixel = (u32 *)row;
  //   for (i32 x = 0; x < Buffer->Width; ++x) {
  //     u8 red = (u8)(x - GameState->XOffset);
  //     u8 green = 0;
  //     u8 blue = (u8)(y + GameState->YOffset);
  //     *pixel++ = (red << 16 | green << 8 | blue);
  //   }
  //   row += pitch;
  // }
  ClearScreen(Buffer);

  u32 TileMap[TileRows][TileCols] = {
      {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1},
      {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
      {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
      {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
      {1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
      {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
      {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1},
  };

  for (u32 y = 0; y < TileRows; ++y) {
    for (u32 x = 0; x < TileCols; ++x) {
      f32 paddingX = 10;
      f32 paddingY = 10;
      f32 MinX = paddingX + (f32)(x * TileWidth);
      f32 MinY = paddingY + (f32)(y * TileHeight);
      f32 MaxX = (f32)(MinX + TileWidth);
      f32 MaxY = (f32)(MinY + TileHeight);
      u32 Color = 0x0000ff;
      if (TileMap[y][x] == 1) {
        Color = 0xffffff;
      }
      DrawRectangle(Buffer, MinX, MaxX, MinY, MaxY, Color);
    }
  }
  PlayerRender(GameState, Buffer);
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
        u8 green = 0;
        u8 blue = 0;
        *pixel++ = (red << 16 | green << 8 | blue);
      } else {
        pixel++;
      }
    }
    row += pitch;
  }
}

internal inline i32 RoundF32ToI32(f32 x) { return (i32)(x + 0.5f); }
internal inline i32 RoundF32ToU32(f32 x) { return (i32)(x + 0.5f); }

internal void DrawRectangle(Bitmap_Buffer *Buffer, f32 MinX, f32 MaxX, f32 MinY,
                            f32 MaxY, RGB Color) {
  i32 IMaxX = RoundF32ToI32(MaxX);
  i32 IMaxY = RoundF32ToI32(MaxY);
  i32 IMinX = RoundF32ToI32(MinX);
  i32 IMinY = RoundF32ToI32(MinY);

  if (IMaxX > Buffer->Width) {
    IMaxX = Buffer->Width;
  }
  if (IMaxY > Buffer->Height) {
    IMaxY = Buffer->Height;
  }
  if (IMinX < 0) {
    IMinX = 0;
  }
  if (IMinY < 0) {
    IMinY = 0;
  }

  i32 pitch = Buffer->Width * Buffer->BytesPerPixel;
  u8 *row =
      (u8 *)(Buffer->Memory) + IMinX * Buffer->BytesPerPixel + IMinY * pitch;
  for (i32 y = IMinY; y < IMaxY; ++y) {
    u32 *pixel = (u32 *)row;
    for (i32 x = IMinX; x < IMaxX; ++x) {
      *pixel++ = Color;
    }
    row += pitch;
  }
}

internal void ClearScreen(Bitmap_Buffer *Buffer) {
  DrawRectangle(Buffer, 0, Buffer->Width, 0, Buffer->Height, 0);
}

internal void GameMovement(Game_State *GameState, Game_Input *GameInput) {
  f32 dx = 0.0f;
  f32 dy = 0.0f;
  f32 PlayerSpeed = 200.0f;
  if (GameInput->Right) {
    dx += 1.0f;
  }
  if (GameInput->Left) {
    dx -= 1.0f;
  }
  if (GameInput->Up) {
    dy -= 1.0f;
  }
  if (GameInput->Down) {
    dy += 1.0f;
  }

  // TODO: Horizontal speed will be sqrt(2)
  GameState->PlayerX += GameState->DtPerFrame * dx * PlayerSpeed;
  GameState->PlayerY += GameState->DtPerFrame * dy * PlayerSpeed;
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
void GameInit(Game_Memory *Memory, Bitmap_Buffer *Buffer) {
  Assert(sizeof(Game_State) <= Memory->PermanentStorageSize);
  Game_State *GameState = (Game_State *)Memory->PermanentStorage;
  if (!Memory->IsInitialised) {

    GameState->PlayerX = Buffer->Height / 2;
    GameState->PlayerY = Buffer->Width / 2;

    GameState->AudioPause = true;

    GameState->SamplesPerSecond = 48000;
    GameState->ToneHz = 100.0f;

    GameState->GlobalPause = false;
    GameState->AudioPause = true;
    GameState->Recording = false;
    GameState->Playback = false;

    Memory->IsInitialised = true;
  }
}

extern "C" void GameUpdate(Game_Memory *Memory, Bitmap_Buffer *Buffer,
                           Game_Input *GameInput) {

  Game_State *GameState = (Game_State *)Memory->PermanentStorage;
  GameInit(Memory, Buffer);

  if (!GameState->GlobalPause) {
    GameRender(GameState, Buffer);
    GameMovement(GameState, GameInput);
  }
}
