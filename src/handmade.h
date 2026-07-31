#if !defined(HANDMADE_H)
#define HANDMADE_H

typedef struct {
  void *Memory;
  i32 MemorySize;
  i32 Width;
  i32 Height;
  u8 BytesPerPixel;
} BitmapBuffer;

//  TODO: add sampleoffset later on
typedef struct {
  i16 *SampleOut;
  i32 SampleCount;
  i32 SamplesPerSecond;
  f32 ToneHz;
} AudioState;

internal void GameRender(BitmapBuffer Buffer, i32 Xoffset, i32 Yoffset);
internal void GameSoundOutput(AudioState *state);

#endif
