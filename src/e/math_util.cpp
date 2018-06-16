#include <e/math_util.hpp>

#include <random>

float MathUtil::degreesToRadians(float degrees) {
  return 3.14159265359 / 180.0;
}

float MathUtil::rand01() {
  return (float) rand()/RAND_MAX;
}

float MathUtil::randN1P1() {
  return rand01() * 2 - 1;
}

int MathUtil::clamp(int x, int lower, int upper) {
  if (x > upper) {
    return upper;
  } else if (x < lower) {
    return lower;
  }

  return x;
}
