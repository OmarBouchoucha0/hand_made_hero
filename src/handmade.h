#if !defined(HANDMADE_H)
#define HANDMADE_H

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

#define Kilobytes(x) ((u64)(x) * 1024)
#define Megabytes(x) ((u64)(x) * 1024 * 1024)
#define Gigabytes(x) ((u64)(x) * 1024 * 1024 * 1024)
#define Terabytes(x) ((u64)(x) * 1024 * 1024 * 1024 * 1024)

typedef enum Scancode {
  // arrow keys
  SCANCODE_RIGHT = 79,
  SCANCODE_LEFT = 80,
  SCANCODE_DOWN = 81,
  SCANCODE_UP = 82,

  // wasd
  SCANCODE_W = 26,
  SCANCODE_A = 4,
  SCANCODE_S = 22,
  SCANCODE_D = 7,

  // spacebar
  SCANCODE_SPACE = 44,
} Scancode;

typedef struct {
  b32 IsInitialised;
  u64 PermanentStorageSize;
  void *PermanentStorage; // NOTE: must be initilize to 0
  u64 TransiantStorageSize;
  void *TransiantStorage; // NOTE: must be initilize to 0
} GameMemory;

typedef struct {
  struct {
    i32 XOffset;
    i32 YOffset;
  };
  struct {
    i32 SamplesPerSecond;
    f32 ToneHz;
    i32 SampleIndex;
  };
} GameState;

typedef struct {
  void *Memory;
  i32 MemorySize;
  i32 Width;
  i32 Height;
  u8 BytesPerPixel;
} BitmapBuffer;

//  TODO: add sampleoffset
typedef struct {
  i16 *SampleOut;
  i32 SampleCount;
} AudioState;

internal void GameRender(GameState *GameState, BitmapBuffer Buffer);
internal void GameSoundOutput(GameState *GameState, AudioState AudioState);
internal void GameMovement(GameState *GameState, const u8 *KeyboardState);
internal void GameUpdate(GameMemory *Memory, BitmapBuffer Buffer,
                         const u8 *KeyboardState);

#endif
