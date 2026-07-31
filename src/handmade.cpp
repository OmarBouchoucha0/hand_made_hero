#include "handmade.h"

internal void GameRender(BitmapBuffer Buffer, i32 Xoffset, i32 Yoffset) {
  int pitch = Buffer.Width * Buffer.BytesPerPixel;
  u8 *row = (u8 *)Buffer.Memory;

  for (i32 y = 0; y < Buffer.Height; ++y) {
    u32 *pixel = (u32 *)row;
    for (i32 x = 0; x < Buffer.Width; ++x) {
      u8 red = x - Xoffset;
      u8 green = 0;
      u8 blue = y + Yoffset;
      *pixel++ = (red << 16 | green << 8 | blue);
    }
    row += pitch;
  }
}

void GameSoundOutput(AudioState *State) {
  local_persist i32 SampleIndex = 0;
  i32 right = 0;
  i32 left = 0;
  i32 Amp = 6000;
  f32 Angle = 0;
  for (i32 i = 0; i < State->SampleCount; i += 2) {
    Angle = 2.0f * (f32)PI * State->ToneHz * SampleIndex /
            (f32)State->SamplesPerSecond;
    left = (i16)(Amp * sinf(Angle));
    right = (i16)(Amp * sinf(Angle));
    *State->SampleOut++ = left;
    *State->SampleOut++ = right;
    ++SampleIndex;
  }
}
