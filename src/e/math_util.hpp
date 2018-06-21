#pragma once

namespace MathUtil {
  float degreesToRadians(float degrees);

  int clamp(int x, int lower, int upper);

  // rand between 0 and 1
  float rand01();

  // rand between -1 and +1
  float randN1P1();

  float clampf(float x, float lower, float upper);
  float lerpf(float d, float a, float b);
}
