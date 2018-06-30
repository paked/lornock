// External libraries
// NONE

// Vendored libraries
#include <glad/glad.h>

#define HANDMADE_MATH_IMPLEMENTATION
#include <HandmadeMath.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include <stb_image.h>

// Lornock code
#include <platform_common.hpp>

Platform* platform = 0;

struct LornockData {
  int number;

  hmm_vec2 vec;
};

// NOTE(harrison): init is ran every time the DLL is loaded. It should not set
// any state, as we want state to persist between hot reloads.
extern "C" int lornockInit(Platform* p) {
  platform = p;

  if (!gladLoadGLLoader(p->glLoadProc)) {
    logln("ERROR: Could not load glad");

    return 1;
  }

  return 0;
}

extern "C" void lornockUpdate(LornockMemory* m) {
  LornockData* lornockData = (LornockData*) m->permanentStorage;
  if (!m->initialized) {
    lornockData->number = 42;

    lornockData->vec.x = 22;
    lornockData->vec.y = 23;

    glEnable(GL_DEPTH_TEST);
    // TODO(harrison): pull from platform layer
    glViewport(0, 0, 640, 480);

    void* data;
    uint32 len;

    loadFromFile("data/shaders/default.frag", &data, &len);

    char* str = (char*) data;
    logfln("INFO: loaded str: %s", str);

    m->initialized = true;
  }

  glClearColor(0.59f, 0.58f, 0.93f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}
