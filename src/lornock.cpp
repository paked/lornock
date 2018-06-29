// External libraries
// NONE

// Vendored libraries
#include <glad/glad.h>
#define HANDMADE_MATH_IMPLEMENTATION
#include <HandmadeMath.h>

// Lornock code
#include <platform_common.hpp>

struct LornockData {
  int number;

  hmm_vec2 vec;
};

extern "C" int initLornock(Platform* p) {
  if (!gladLoadGLLoader(p->glLoadProc)) {
    logln("ERROR: Could not load glad");

    return 1;
  }

  return 0;
}

extern "C" void updateLornock(LornockMemory* m) {
  LornockData* lornockData = (LornockData*) m->permanentStorage;
  if (!m->initialized) {
    lornockData->number = 42;

    lornockData->vec.x = 22;
    lornockData->vec.y = 23;

    m->initialized = true;
  }

  glClearColor(0.59f, 0.58f, 0.93f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}
