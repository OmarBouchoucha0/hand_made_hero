#include "handmade.h"

internal void GameRender(BitmapBuffer Buffer, i32 XOffset, i32 YOffset) {

  int pitch = Buffer.Width * Buffer.BytesPerPixel;
  u8 *row = (u8 *)Buffer.Memory;

  for (i32 y = 0; y < Buffer.Height; ++y) {
    u32 *pixel = (u32 *)row;
    for (i32 x = 0; x < Buffer.Width; ++x) {
      u8 red = x - XOffset;
      u8 green = 0;
      u8 blue = y + YOffset;
      *pixel++ = (red << 16 | green << 8 | blue);
    }
    row += pitch;
  }
}

void GameMovement(const u8 *KeyboardState, i32 *x, i32 *y) {
  if (KeyboardState[SCANCODE_RIGHT] || KeyboardState[SCANCODE_D]) {
    *x += 1;
  }
  if (KeyboardState[SCANCODE_LEFT] || KeyboardState[SCANCODE_A]) {
    *x -= 1;
  }
  if (KeyboardState[SCANCODE_UP] || KeyboardState[SCANCODE_W]) {
    *y += 1;
  }
  if (KeyboardState[SCANCODE_DOWN] || KeyboardState[SCANCODE_S]) {
    *y -= 1;
  }
}

void GameSoundOutput(AudioState State) {
  local_persist i32 SamplesPerSecond = 48000;
  local_persist f32 ToneHz = 100.0f;
  local_persist i32 SampleIndex = 0;
  i32 right = 0;
  i32 left = 0;
  i32 Amp = 6000;
  f32 Angle = 0;
  for (i32 i = 0; i < State.SampleCount; i += 2) {
    Angle = 2.0f * (f32)PI * ToneHz * SampleIndex / (f32)SamplesPerSecond;
    left = (i16)(Amp * sinf(Angle));
    right = (i16)(Amp * sinf(Angle));
    *State.SampleOut++ = left;
    *State.SampleOut++ = right;
    ++SampleIndex;
  }
}

internal void GameUpdate(BitmapBuffer Buffer, const u8 *KeyboardState) {
  // TODO: add vec2 support
  local_persist i32 XOffset = 0;
  local_persist i32 YOffset = 0;
  GameRender(Buffer, XOffset, YOffset);
  GameMovement(KeyboardState, &XOffset, &YOffset);
}
