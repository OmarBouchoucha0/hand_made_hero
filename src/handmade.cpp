#include "handmade.h"
#include <cstring>

// TODO: add this ti the memory of hte program
global_variable u32 InitialTileMap[TILE_ROWS_COUNT][TILE_COLS_COUNT] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1},
};
global_variable u32 SecondTileMap[TILE_ROWS_COUNT][TILE_COLS_COUNT] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};
global_variable Tile_Map TileMap = {};

internal void GameRender(Game_State *GameState, Bitmap_Buffer *Buffer,
                         Tile_Map TileMap) {
  ClearScreen(Buffer);
  DrawTileMap(Buffer, TileMap);
  DrawPlayer(GameState, Buffer);
}

internal u32 GetTileMapValueUnchecked(Tile_Map TileMap, i32 TileX, i32 TileY) {
  return TileMap.Map[TileY][TileX];
}
internal b32 TileMapBoundsCheckTop(Tile_Map TileMap, f32 Y) {
  if (Y < TileMap.MinY) {
    return true;
  }
  return false;
}

internal b32 TileMapBoundsCheckBottom(Tile_Map TileMap, f32 Y) {
  if (Y >= TileMap.MaxY) {
    return true;
  }
  return false;
}

internal b32 TileMapBoundsCheckLeft(Tile_Map TileMap, f32 X) {
  if (X < TileMap.MinX) {
    return true;
  }
  return false;
}

internal b32 TileMapBoundsCheckRight(Tile_Map TileMap, f32 X) {
  if (X >= TileMap.MaxX) {
    return true;
  }
  return false;
}

internal void DrawTileMap(Bitmap_Buffer *Buffer, Tile_Map TileMap) {

  for (u32 y = 0; y < TILE_ROWS_COUNT; ++y) {
    for (u32 x = 0; x < TILE_COLS_COUNT; ++x) {
      f32 MinX = TileMap.PaddingX + (f32)(x * TILE_WIDTH);
      f32 MinY = TileMap.PaddingY + (f32)(y * TILE_HEIGHT);
      f32 MaxX = (f32)(MinX + TILE_WIDTH);
      f32 MaxY = (f32)(MinY + TILE_HEIGHT);
      u32 Color = 0x0000ff;
      if (GetTileMapValueUnchecked(TileMap, x, y) == 1) {
        Color = 0xffffff;
      }
      DrawRectangleOutline(Buffer, MinX, MaxX, MinY, MaxY, Color);
    }
  }
}

internal void DrawPlayer(Game_State *GameState, Bitmap_Buffer *Buffer) {
  u32 Color = 0xFF0000;

  f32 MinX = GameState->PlayerX - GameState->PlayerWidth / 2.0f;
  f32 MinY = GameState->PlayerY - GameState->PlayerHeight / 2.0f;
  f32 MaxX = GameState->PlayerX + GameState->PlayerWidth / 2.0f;
  f32 MaxY = GameState->PlayerY + GameState->PlayerHeight / 2.0f;

  DrawRectangle(Buffer, MinX, MaxX, MinY, MaxY, Color);
}

internal inline i32 RoundF32ToI32(f32 x) {
  if (x > 0) {
    return (i32)(x + 0.5f);
  }
  return (i32)(x - 0.5f);
}
internal inline u32 RoundF32ToU32(f32 x) { return (u32)(x + 0.5f); }
internal inline i32 TruncateF32ToI32(f32 x) { return (i32)(x); }
internal inline u32 TruncateF32ToU32(f32 x) { return (u32)(x); }

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

internal void DrawRectangleOutline(Bitmap_Buffer *Buffer, f32 MinX, f32 MaxX,
                                   f32 MinY, f32 MaxY, RGB Color) {
  i32 IMaxX = RoundF32ToI32(MaxX);
  i32 IMaxY = RoundF32ToI32(MaxY);
  i32 IMinX = RoundF32ToI32(MinX);
  i32 IMinY = RoundF32ToI32(MinY);

  if (IMaxX > Buffer->Width)
    IMaxX = Buffer->Width;

  if (IMaxY > Buffer->Height)
    IMaxY = Buffer->Height;

  if (IMinX < 0)
    IMinX = 0;

  if (IMinY < 0)
    IMinY = 0;

  i32 Pitch = Buffer->Width;

  u32 *pixel = (u32 *)Buffer->Memory + IMinY * Pitch + IMinX;

  for (i32 x = IMinX; x < IMaxX; ++x) {
    *pixel++ = Color;
  }

  pixel = (u32 *)Buffer->Memory + (IMaxY - 1) * Pitch + IMinX;

  for (i32 x = IMinX; x < IMaxX; ++x) {
    *pixel++ = Color;
  }

  pixel = (u32 *)Buffer->Memory + IMinY * Pitch + IMinX;

  for (i32 y = IMinY; y < IMaxY; ++y) {
    *pixel = Color;
    pixel += Pitch;
  }

  pixel = (u32 *)Buffer->Memory + IMinY * Pitch + (IMaxX - 1);

  for (i32 y = IMinY; y < IMaxY; ++y) {
    *pixel = Color;
    pixel += Pitch;
  }
}

internal void ClearScreen(Bitmap_Buffer *Buffer) {
  DrawRectangle(Buffer, 0, Buffer->Width, 0, Buffer->Height, 0);
}

internal b32 TileMapCollision(f32 X, f32 Y, Tile_Map TileMap) {

  u32 PlayerTileX = TruncateF32ToU32((X - TileMap.PaddingX) / TILE_WIDTH);
  u32 PlayerTileY = TruncateF32ToU32((Y - TileMap.PaddingY) / TILE_HEIGHT);
  if (PlayerTileX >= TILE_COLS_COUNT || PlayerTileY >= TILE_ROWS_COUNT) {
    return false;
  }

  if (GetTileMapValueUnchecked(TileMap, PlayerTileX, PlayerTileY) == 1) {
    return true;
  }
  return false;
}

internal void GameMovement(Game_State *GameState, Game_Input *GameInput,
                           Tile_Map *TileMap) {
  f32 dx = 0.0f;
  f32 dy = 0.0f;
  f32 PlayerSpeed = 100.0f;
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
  f32 NewPlayerX =
      GameState->PlayerX + GameState->DtPerFrame * dx * PlayerSpeed;
  f32 NewPlayerY =
      GameState->PlayerY + GameState->DtPerFrame * dy * PlayerSpeed;

  // i32 PlayerTileY =
  //     RoundF32ToI32((NewPlayerY - (f32)TileMap.PaddingY) / (f32)TILE_HEIGHT);
  // i32 PlayerTileX =
  //     RoundF32ToI32((NewPlayerX - (f32)TileMap.PaddingX) / (f32)TILE_WIDTH);

  if (TileMapBoundsCheckTop(*TileMap, NewPlayerY)) {
    GameState->PlayerY = TileMap->MaxY;
    memcpy(TileMap->Map, InitialTileMap, sizeof(InitialTileMap));
    return;
  }
  if (TileMapBoundsCheckBottom(*TileMap, NewPlayerY)) {
    GameState->PlayerY = TileMap->MinY;
    memcpy(TileMap->Map, SecondTileMap, sizeof(SecondTileMap));
    return;
  }
  if (TileMapBoundsCheckLeft(*TileMap, NewPlayerX)) {
    GameState->PlayerX = TileMap->MaxX;
    return;
  }
  if (TileMapBoundsCheckRight(*TileMap, NewPlayerX)) {
    GameState->PlayerX = TileMap->MinX;
    return;
  }

  f32 left = NewPlayerX - GameState->PlayerWidth * 0.5f;
  f32 top = NewPlayerY - GameState->PlayerHeight * 0.5f;
  f32 right = NewPlayerX + GameState->PlayerWidth * 0.5f;
  f32 bottom = NewPlayerY + GameState->PlayerHeight * 0.5f;

  if (!TileMapCollision(left, top, *TileMap) &&
      !TileMapCollision(left, bottom, *TileMap) &&
      !TileMapCollision(right, top, *TileMap) &&
      !TileMapCollision(right, bottom, *TileMap)) {
    GameState->PlayerX = NewPlayerX;
    GameState->PlayerY = NewPlayerY;
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
void GameInit(Game_Memory *Memory, Tile_Map *TileMap) {
  Assert(sizeof(Game_State) <= Memory->PermanentStorageSize);
  Game_State *GameState = (Game_State *)Memory->PermanentStorage;

  if (!Memory->IsInitialised) {
    memcpy(TileMap->Map, InitialTileMap, sizeof(InitialTileMap));

    TileMap->PaddingX = 200;
    TileMap->PaddingY = 200;
    TileMap->MinX = TileMap->PaddingX;
    TileMap->MinY = TileMap->PaddingY;
    TileMap->MaxX = TileMap->MinX + (f32)(TILE_COLS_COUNT * TILE_WIDTH);
    TileMap->MaxY = TileMap->MinY + (f32)(TILE_ROWS_COUNT * TILE_HEIGHT);
    f32 CenterX =
        (f32)((f32)(TILE_COLS_COUNT * TILE_WIDTH) / 2 + (f32)TileMap->PaddingX);
    f32 CenterY = (f32)((f32)(TILE_ROWS_COUNT * TILE_HEIGHT) / 2 +
                        (f32)TileMap->PaddingY);
    GameState->PlayerX = CenterX;
    GameState->PlayerY = CenterY;
    GameState->PlayerHeight = 10;
    GameState->PlayerWidth = 10;

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
  GameInit(Memory, &TileMap);

  if (!GameState->GlobalPause) {
    GameMovement(GameState, GameInput, &TileMap);
    GameRender(GameState, Buffer, TileMap);
  }
}
