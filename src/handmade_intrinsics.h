// TODO: intrincics
#include <math.h>

inline i32 RoundF32ToI32(f32 x) {
  if (x > 0.0f) {
    return (i32)(x + 0.5f);
  }
  return (i32)(x - 0.5f);
}
inline i32 FloorF32ToI32(f32 x) {
  if (x > 0.0f) {
    return (i32)(x);
  }
  return (i32)(x + 1.0f);
}
inline i32 TruncateF32ToI32(f32 x) { return (i32)(x); }
inline u32 TruncateF32ToU32(f32 x) { return (u32)(x); }
inline f32 Sin(f32 Angle) { return sinf(Angle); }
