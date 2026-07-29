#if !defined(HANDMADE_H)
#define HANDMADE_H

typedef struct {
  void *Memory;
  i32 MemorySize;
  i32 Width;
  i32 Height;
  u8 BytesPerPixel;
} BitmapBuffer;

internal void Render(BitmapBuffer Buffer, i32 Xoffset, i32 Yoffset);
#endif
