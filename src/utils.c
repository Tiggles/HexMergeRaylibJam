#ifndef UTILS_GUARD
#define UTILS_GUARD

#include <math.h>
#include "raylib.h"

// Return euclidean distance between two vectors
float vector2Distance(Vector2 a, Vector2 b) {
  return sqrtf(powf((a.x - b.x), 2) + powf((a.y - b.y), 2));
}

#endif
