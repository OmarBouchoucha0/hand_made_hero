#if !defined(HANDMADE_H)
#define HANDMADE_H

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

internal void GameRender(BitmapBuffer Buffer, i32 Xoffset, i32 Yoffset);
internal void GameSoundOutput(AudioState state);

#endif
