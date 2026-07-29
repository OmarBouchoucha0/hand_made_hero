#include "handmade.h"

internal void Render(BitmapBuffer Buffer, i32 Xoffset, i32 Yoffset) {
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
