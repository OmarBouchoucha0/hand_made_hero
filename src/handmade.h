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
#define AUDIO_S16 0x8010 /**< Signed 16-bit samples */

#define AUDIO_FREQ 48000
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

#define Kilobytes(x) ((u64)(x) * 1024)
#define Megabytes(x) ((u64)(x) * 1024 * 1024)
#define Gigabytes(x) ((u64)(x) * 1024 * 1024 * 1024)
#define Terabytes(x) ((u64)(x) * 1024 * 1024 * 1024 * 1024)

enum Scan_Code {
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
};

struct Game_Memory {
  b32 IsInitialised;
  u64 PermanentStorageSize;
  void *PermanentStorage; // NOTE: must be initilize to 0
  u64 TransiantStorageSize;
  void *TransiantStorage; // NOTE: must be initilize to 0
};

struct Game_State {
  struct {
    i32 XOffset;
    i32 YOffset;
  };
  struct {
    i32 SamplesPerSecond;
    i32 ToneHz;
    i32 SampleIndex;
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

internal void GameRender(Game_State *GameState, Bitmap_Buffer Buffer);
 extern "C" void GameSoundOutput(Game_State *GameState, Audio_State AudioState);
internal void GameMovement(Game_State *GameState, const u8 *KeyboardState);
extern "C" void GameUpdate(Game_Memory *Memory, Bitmap_Buffer Buffer,
                           const u8 *KeyboardState);
#if HANDMADE_INTERNAL
typedef struct {
  void *Memory;
  u32 MemorySize;
} DEBUG_File_Slice;

// NOTE: return NULL on failure
DEBUG_File_Slice DEBUGPlatformReadEntireFile(const char *FileName);
void DEBUGPlatformFreeEntireFile(void *Memory);
// NOTE: u64 only supports up to 4gb files
void DEBUGPlatformWriteEntireFile(const char *FileName, DEBUG_File_Slice File);
#endif

#endif
