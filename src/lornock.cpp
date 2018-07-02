// External libraries
#include <math.h>

// Vendored libraries
#include <glad/glad.h>

#define HANDMADE_MATH_IMPLEMENTATION
#include <HandmadeMath.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include <stb_image.h>

// Platform common
#include <platform_common.hpp>

Platform* platform = 0;
LornockMemory* lornockMemory = 0;

// Lornock code
#include <hmm_wrapper.cpp>
#include <assets.cpp>
#include <lornock_data.cpp>
#include <states.cpp>

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
  dbg_assert(sizeof(m->permanentStorage) < m->permanentStorageSize);

  lornockMemory = m;

  lornockData = (LornockData*) m->permanentStorage;

  if (!m->initialized) {
    stateInit(&lornockData->state, STATE_game);

    // Refresh assets
    updateAssets();

    // TODO: refactor into drawInit or something
    glEnable(GL_DEPTH_TEST);
    // TODO(harrison): pull from platform layer
    glViewport(0, 0, 640, 480);

    m->initialized = true;
  }

  updateAssets();

  stateUpdate(&lornockData->state);
}
