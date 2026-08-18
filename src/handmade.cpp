#include "handmade.h"
#include "handmade_intrinsics.h"

//==========================Rendering==========================================
internal void DrawTileMap(Bitmap_Buffer *Buffer, Game_State *GameState) {
  Tile_Map TileMap = GetCurrentTileMap(GameState);
  for (u32 y = 0; y < TILE_ROWS_COUNT; ++y) {
    for (u32 x = 0; x < TILE_COLS_COUNT; ++x) {
      f32 MinX = GameState->WorldMap.TileMapPaddingX + (f32)(x * TILE_SIDE);
      f32 MinY = GameState->WorldMap.TileMapPaddingY + (f32)(y * TILE_SIDE);
      f32 MaxX = (f32)(MinX + TILE_SIDE);
      f32 MaxY = (f32)(MinY + TILE_SIDE);
      u32 Color = 0x0000ff;
      MinX *= GameState->MetersToPixels;
      MinY *= GameState->MetersToPixels;
      MaxX *= GameState->MetersToPixels;
      MaxY *= GameState->MetersToPixels;
      if (GetTileMapValueUnchecked(TileMap, x, y) == 1) {
        Color = 0xffffff;
      }
      Tile_Position TilePosition = FromCanonPosToTilePos(GameState);
      if (TilePosition.ChunkRelX == x &&
          TilePosition.ChunkRelY == (TILE_COLS_COUNT - 1 - y)) {
        Color = 0xff00ff;
      }
      DrawRectangleOutline(Buffer, MinX, MaxX, MinY, MaxY, Color);
    }
  }
}

internal void DrawPlayer(Game_State *GameState, Bitmap_Buffer *Buffer) {
  u32 Color = 0xFF0000;
  f32 HalfPlayerWidth =
      GameState->PlayerWidth / 2.0f * GameState->MetersToPixels;
  f32 HalfPlayerHeight =
      GameState->PlayerHeight / 2.0f * GameState->MetersToPixels;

  Raw_Position RawPosition = FromCanonPosToRawPos(GameState, Buffer);

  f32 MinX = RawPosition.X - HalfPlayerWidth;
  f32 MaxX = RawPosition.X + HalfPlayerWidth;
  f32 MinY = RawPosition.Y - HalfPlayerHeight;
  f32 MaxY = RawPosition.Y + HalfPlayerHeight;

  DrawRectangle(Buffer, MinX, MaxX, MinY, MaxY, Color);
}

internal void DrawScreenBorder(Bitmap_Buffer *Buffer) {
  u32 Color = 0x0f0f0f;
  f32 MinX = 10;
  f32 MaxX = GAME_RES_X - 10;
  f32 MinY = 10;
  f32 MaxY = GAME_RES_Y - 10;
  DrawRectangleOutline(Buffer, MinX, MaxX, MinY, MaxY, Color);
}

internal Raw_Position FromCanonPosToRawPos(Game_State *GameState,
                                           Bitmap_Buffer *Buffer) {
  Raw_Position RawPosition = {};
  RawPosition.X = (GameState->Pos.TileX * TILE_SIDE + GameState->Pos.TileRelX +
                   GameState->WorldMap.TileMapPaddingX) *
                  GameState->MetersToPixels;
  RawPosition.Y = (GameState->Pos.TileY * TILE_SIDE + GameState->Pos.TileRelY +
                   GameState->WorldMap.TileMapPaddingY) *
                  GameState->MetersToPixels;
  RawPosition.Y = (f32)Buffer->Height - RawPosition.Y;
  return RawPosition;
}

internal Tile_Position FromCanonPosToTilePos(Game_State *GameState) {
  Tile_Position TilePosition = {};
  TilePosition.ChunkX = GameState->Pos.TileX >> GameState->Pos.ChunckShift;
  TilePosition.ChunkY = GameState->Pos.TileY >> GameState->Pos.ChunckShift;
  TilePosition.ChunkRelX = GameState->Pos.TileX & GameState->Pos.ChunkMask;
  TilePosition.ChunkRelY = GameState->Pos.TileY & GameState->Pos.ChunkMask;
  return TilePosition;
}

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

internal void GameRender(Game_State *GameState, Bitmap_Buffer *Buffer) {
  ClearScreen(Buffer);
  DrawTileMap(Buffer, GameState);
  DrawScreenBorder(Buffer);
  DrawPlayer(GameState, Buffer);
}

//==========================TileMap==========================================
internal Tile_Map GetCurrentTileMap(Game_State *GameState) {
  Tile_Position TilePosition = FromCanonPosToTilePos(GameState);
  u32 Index = TilePosition.ChunkY * GameState->WorldMap.TileMapCountX +
              TilePosition.ChunkX;
  return GameState->WorldMap.TileMaps[Index];
}

internal inline u32 GetTileMapValueUnchecked(Tile_Map TileMap, i32 TileX,
                                             i32 TileY) {
  return TileMap.Map[TileY][TileX];
}

internal inline b32 TileMapBoundsCheckBottom(Game_State *GameState, f32 Y) {
  Tile_Position TilePosition = FromCanonPosToTilePos(GameState);
  return (TilePosition.ChunkRelY <= 0 && Y < 0);
}

internal inline b32 TileMapBoundsCheckTop(Game_State *GameState, f32 Y) {
  Tile_Position TilePosition = FromCanonPosToTilePos(GameState);
  return (TilePosition.ChunkRelY >= (i32)TILE_ROWS_COUNT - 1.0f &&
          Y > TILE_SIDE);
}

internal inline b32 TileMapBoundsCheckLeft(Game_State *GameState, f32 X) {
  Tile_Position TilePosition = FromCanonPosToTilePos(GameState);
  return (TilePosition.ChunkRelX <= 0 && X < 0);
}

internal inline b32 TileMapBoundsCheckRight(Game_State *GameState, f32 X) {
  Tile_Position TilePosition = FromCanonPosToTilePos(GameState);
  return (TilePosition.ChunkRelX >= (i32)TILE_COLS_COUNT - 1.0f &&
          X > TILE_SIDE);
}
//==========================Movement==========================================
internal void GameMovement(Game_State *GameState, Game_Input *GameInput) {
  f32 dx = 0.0f;
  f32 dy = 0.0f;
  f32 PlayerSpeed = PLAYER_SPEED;
  if (GameInput->Right) {
    dx += 1.0f;
  }
  if (GameInput->Left) {
    dx -= 1.0f;
  }
  if (GameInput->Up) {
    dy += 1.0f;
  }
  if (GameInput->Down) {
    dy -= 1.0f;
  }

  // TODO: Horizontal speed will be sqrt(2)
  f32 NewPlayerX =
      GameState->Pos.TileRelX + GameState->DtPerFrame * dx * PlayerSpeed;
  f32 NewPlayerY =
      GameState->Pos.TileRelY + GameState->DtPerFrame * dy * PlayerSpeed;

  Tile_Position TilePosition = FromCanonPosToTilePos(GameState);
  if (NewPlayerX < 0) {
    GameState->Pos.TileX -= 1;
    NewPlayerX += (f32)TILE_SIDE;
  }
  if (NewPlayerX >= TILE_SIDE) {
    GameState->Pos.TileX += 1;
    NewPlayerX -= (f32)TILE_SIDE;
  }
  if (NewPlayerY < 0) {
    GameState->Pos.TileY -= 1;
    NewPlayerY += (f32)TILE_SIDE;
  }
  if (NewPlayerY >= TILE_SIDE) {
    GameState->Pos.TileY += 1;
    NewPlayerY -= (f32)TILE_SIDE;
  }

  GameState->Pos.TileRelX = NewPlayerX;
  GameState->Pos.TileRelY = NewPlayerY;

  // f32 CenterX = TILE_SIDE / 2.0f;
  // f32 CenterY = TILE_SIDE / 2.0f;
  //
  // if (TilePosition.ChunkRelY <= 0 && NewPlayerY >= TILE_SIDE) {
  //   GameState->Pos.TileY = ((GameState->Pos.TileY >> 8) - 1) << 8;
  //   GameState->Pos.TileY |= TILE_ROWS_COUNT - 1;
  //   NewPlayerX = CenterX;
  //   NewPlayerY = CenterY;
  // }
  // if ((TilePosition.ChunkRelY >= TILE_ROWS_COUNT - 1) && NewPlayerY < 0) {
  //   GameState->Pos.TileY = ((GameState->Pos.TileY >> 8) + 1) << 8;
  //   GameState->Pos.TileY |= (0) & 0xFF;
  //   NewPlayerX = CenterX;
  //   NewPlayerY = CenterY;
  // }
  // if (TilePosition.ChunkRelX <= 0 && NewPlayerX < 0) {
  //   GameState->Pos.TileX = ((GameState->Pos.TileX >> 8) - 1) << 8;
  //   GameState->Pos.TileX |= (TILE_COLS_COUNT - 1) & 0xFF;
  //   NewPlayerX = CenterX;
  //   NewPlayerY = CenterY;
  // }
  // if ((TilePosition.ChunkRelX >= TILE_COLS_COUNT - 1) &&
  //     NewPlayerX >= TILE_SIDE) {
  //   GameState->Pos.TileX = ((GameState->Pos.TileX >> 8) + 1) << 8;
  //   GameState->Pos.TileX |= (0) & 0xFF;
  //   NewPlayerX = CenterX;
  //   NewPlayerY = CenterY;
  // }

  // // TODO: this collision detection is very bad
  // f32 Left = NewPlayerX - (f32)GameState->PlayerWidth * 0.5f;
  // f32 Right = NewPlayerX + (f32)GameState->PlayerWidth * 0.5f;
  // f32 Top = NewPlayerY + (f32)GameState->PlayerHeight * 0.5f;
  // f32 Bottom = NewPlayerY - (f32)GameState->PlayerHeight * 0.5f;

  // Tile_Map TileMap = GetCurrentTileMap(GameState);
  // if (GetTileMapValueUnchecked(TileMap, TilePosition.ChunkRelX,
  //                              TilePosition.ChunkRelY) == 0) {
  //   if (Bottom >= 0 && Top < TILE_SIDE) {
  //     GameState->Pos.TileRelY = NewPlayerY;
  //   }
  //   if (Left >= 0 && Right < TILE_SIDE) {
  //     GameState->Pos.TileRelX = NewPlayerX;
  //   }
  //
  //   if (Bottom < 0 &&
  //       GetTileMapValueUnchecked(TileMap, TilePosition.ChunkRelX,
  //                                TilePosition.ChunkRelY - 1) == 0) {
  //     GameState->Pos.TileRelY = NewPlayerY;
  //   }
  //
  //   if (Top >= TILE_SIDE &&
  //       GetTileMapValueUnchecked(TileMap, TilePosition.ChunkRelX,
  //                                TilePosition.ChunkRelY + 1) == 0) {
  //     GameState->Pos.TileRelY = NewPlayerY;
  //   }
  //
  //   if (Left < 0 &&
  //       GetTileMapValueUnchecked(TileMap, TilePosition.ChunkRelX - 1,
  //                                TilePosition.ChunkRelY) == 0) {
  //     GameState->Pos.TileRelX = NewPlayerX;
  //   }
  //
  //   if (Right >= TILE_SIDE &&
  //       GetTileMapValueUnchecked(TileMap, TilePosition.ChunkRelX + 1,
  //                                TilePosition.ChunkRelY) == 0) {
  //     GameState->Pos.TileRelX = NewPlayerX;
  //   }
  // }
}

//==========================Sound==========================================
extern "C" void GameSoundOutput(Game_State *GameState, Audio_State AudioState) {
  i16 right = 0;
  i16 left = 0;
  i32 Amp = 6000;
  f32 AngleIncrement =
      2.0f * PI * GameState->ToneHz / (f32)GameState->SamplesPerSecond;
  for (i32 i = 0; i < AudioState.SampleCount; i += 2) {
    left = (i16)((f32)Amp * Sin(GameState->TSine));
    right = left;
    *AudioState.SampleOut++ = left;
    *AudioState.SampleOut++ = right;
    GameState->TSine += AngleIncrement;
    if (GameState->TSine > 2.0f * PI) {
      GameState->TSine -= 2.0f * PI;
    }
  }
}

void GameInit(Game_Memory *Memory) {
  Assert(sizeof(Game_State) <= Memory->PermanentStorageSize);
  Game_State *GameState = (Game_State *)Memory->PermanentStorage;

  if (!Memory->IsInitialised) {

    GameState->WorldMap.TileMapPaddingX = 5;
    GameState->WorldMap.TileMapPaddingY = 5;
    GameState->WorldMap.TileMapMinX = GameState->WorldMap.TileMapPaddingX;
    GameState->WorldMap.TileMapMinY = GameState->WorldMap.TileMapPaddingY;
    GameState->WorldMap.TileMapMaxX =
        GameState->WorldMap.TileMapMinX + (f32)(TILE_COLS_COUNT * TILE_SIDE);
    GameState->WorldMap.TileMapMaxY =
        GameState->WorldMap.TileMapMinY + (f32)(TILE_ROWS_COUNT * TILE_SIDE);
    GameState->WorldMap.TileMapCountX = 1;
    GameState->WorldMap.TileMapCountY = 1;

    f32 CenterX = TILE_SIDE / 2.0f;
    f32 CenterY = TILE_SIDE / 2.0f;
    GameState->Pos.TileX = 0;
    GameState->Pos.TileY = 0;
    GameState->Pos.TileRelX = CenterX;
    GameState->Pos.TileRelY = CenterY;

    GameState->Pos.ChunckShift = 8;
    GameState->Pos.ChunkMask = 0x000000ff;

    GameState->PlayerHeight = 0.5;
    GameState->PlayerWidth = 0.5;

    GameState->MetersToPixels = 25;

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

  if (!Memory->IsInitialised) {
    GameInit(Memory);
    // Tile_Map TileMap0 = {{
    //     {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    //     {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1},
    // }};
    // Tile_Map TileMap1 = {{
    //     {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1},
    // }};
    // Tile_Map TileMap2 = {{
    //     {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0},
    //     {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    // }};
    // Tile_Map TileMap3 = {{
    //     {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1},
    //     {0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1},
    //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    //     {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    // }};

    Tile_Map TestTileMap = {{
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    }};
    // TODO: Game memory instead of local persist
    local_persist Tile_Map TileMaps[1] = {};
    TileMaps[0] = TestTileMap;

    GameState->WorldMap.TileMaps = TileMaps;
  }

  if (!GameState->GlobalPause) {
    GameMovement(GameState, GameInput);
    GameRender(GameState, Buffer);
  }
}
