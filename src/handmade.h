#if !defined(HANDMADE_H)
#define HANDMADE_H

#include <math.h>
#include <stdint.h>

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
#if HANDMADE_SLOW
#define Assert(Expression)                                                     \
  if (!(Expression)) {                                                         \
    *(int *)0 = 0;                                                             \
  }
#else
#define Assert(Expression)
#endif

#define global_variable static
#define local_persist static
#define internal static
#define PI 3.14159265358979323846f
#define GAME_RES_X 960
#define GAME_RES_Y 540

#define TILE_SIDE_PIXELS 30
#define TILE_SIDE 1.0f

#define TILE_ROWS_COUNT 9
#define TILE_COLS_COUNT 17

#define PLAYER_SPEED 10.0f

#define AUDIO_FREQ 48000
#define AUDIO_S16 0x8010 /**< Signed 16-bit samples */
#define AUDIO_FORMAT AUDIO_S16
#define AUDIO_CHANNELS 2
#define AUDIO_SAMPLES Kilobytes(4)

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

typedef u32 RGB;

#define Kilobytes(x) ((u64)(x) * 1024)
#define Megabytes(x) ((u64)(x) * 1024 * 1024)
#define Gigabytes(x) ((u64)(x) * 1024 * 1024 * 1024)
#define Terabytes(x) ((u64)(x) * 1024 * 1024 * 1024 * 1024)

struct Game_Input {
  b32 Right;
  b32 Left;
  b32 Down;
  b32 Up;
};

struct Game_Memory {
  b32 IsInitialised;
  u64 PermanentStorageSize;
  void *PermanentStorage; // NOTE: must be initilize to 0
  u64 TransiantStorageSize;
  void *TransiantStorage; // NOTE: must be initilize to 0
};

struct Tile_Map {
  u32 Map[TILE_ROWS_COUNT][TILE_COLS_COUNT];
};

struct World_Map {
  f32 TileMapPaddingX;
  f32 TileMapPaddingY;
  f32 TileMapMinX;
  f32 TileMapMinY;
  f32 TileMapMaxX;
  f32 TileMapMaxY;
  u32 TileMapCountX;
  u32 TileMapCountY;
  Tile_Map *TileMaps;
};
struct Position {
  // NOTE: world relative x and y
  i32 WorldMapX;
  i32 WorldMapY;
  // NOTE: chunk relative x and y
  i32 TileMapX;
  i32 TileMapY;
  // NOTE: tile relative x and y
  f32 PlayerX;
  f32 PlayerY;
};

struct Game_State {
  struct {
    Position Pos;
    f32 PlayerHeight;
    f32 PlayerWidth;
    f32 DtPerFrame;
  };
  struct {
    i32 SamplesPerSecond;
    i32 ToneHz;
    f32 TSine;
    i32 SampleIndex;
  };
  struct {
    b32 GlobalPause;
    b32 AudioPause;
    b32 Recording;
    b32 Playback;
  };
  struct {
    World_Map WorldMap;
    f32 MetersToPixels;
  };
};

struct Bitmap_Buffer {
  void *Memory;
  u32 MemorySize;
  i32 Width;
  i32 Height;
  u8 BytesPerPixel;
};

//  TODO: add sampleoffset
struct Audio_State {
  i16 *SampleOut;
  i32 SampleCount;
};

#if HANDMADE_INTERNAL
typedef struct {
  void *Memory;
  u32 MemorySize; // NOTE: u32 only supports up to 4gb files
} DEBUG_File_Slice;

#define FILE_WRITE_FAIL 0x0000
#define FILE_WRITE_SUCCES 0x0001
typedef i32 FILE_WRITE_STATUS;

#define FILE_READ_FAIL 0x0000
#define FILE_READ_SUCCES 0x0001
typedef i32 FILE_READ_STATUS;

DEBUG_File_Slice DEBUGPlatformReadEntireFile(const char *FileName);
void DEBUGPlatformFreeEntireFile(DEBUG_File_Slice *File);
FILE_WRITE_STATUS DEBUGPlatformWriteEntireFile(const char *FileName,
                                               DEBUG_File_Slice *File);
FILE_WRITE_STATUS DEBUGPlatformAppendToFile(const char *FileName, void *Memory);

#endif

//==========================Rendering==========================================
internal void DrawRectangle(Bitmap_Buffer *Buffer, f32 MinX, f32 MaxX, f32 MinY,
                            f32 MaxY, RGB Color);
internal void DrawRectangleOutline(Bitmap_Buffer *Buffer, f32 MinX, f32 MaxX,
                                   f32 MinY, f32 MaxY, RGB Color);
internal void DrawTileMap(Bitmap_Buffer *Buffer, Game_State *GameState,
                          Tile_Map TileMap);
internal void DrawPlayer(Game_State *GameState, Bitmap_Buffer *Buffer);
internal void ClearScreen(Bitmap_Buffer *Buffer);
internal void DrawScreenBorder(Bitmap_Buffer *Buffer);
internal void GameRender(Game_State *GameState, Bitmap_Buffer *Buffer);
//==========================TileMap==========================================
internal Tile_Map GetCurrentTileMap(Game_State *GameState);
internal inline u32 GetTileMapValueUnchecked(Tile_Map TileMap, i32 TileX,
                                             i32 TileY);
internal inline b32 TileMapBoundsCheckTop(Game_State *GameState, f32 X, f32 Y);
internal inline b32 TileMapBoundsCheckBottom(Game_State *GameState, f32 X,
                                             f32 Y);
internal inline b32 TileMapBoundsCheckLeft(Game_State *GameState, f32 X, f32 Y);
internal inline b32 TileMapBoundsCheckRight(Game_State *GameState, f32 X,
                                            f32 Y);
internal b32 TileMapCollision(Game_State *GameState, f32 Left, f32 Right,
                              f32 Top, f32 Bottom);
//==========================Movement==========================================
internal void FromCanonPositionToRaw(Game_State *GameState, f32 *X, f32 *Y);
internal void GameMovement(Game_State *GameState, Game_Input *GameInput);
//==========================Sound==========================================
extern "C" void GameSoundOutput(Game_State *GameState, Audio_State AudioState);
//==========================Update==========================================
void GameInit(Game_Memory *Memory);
extern "C" void GameUpdate(Game_Memory *Memory, Bitmap_Buffer *Buffer,
                           Game_Input *GameInput);

#endif
