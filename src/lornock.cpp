//
// External libraries
//
#include <math.h>
#include <ctype.h>

//
// Vendored libraries
//
#include <glad/glad.h>

#define HANDMADE_MATH_IMPLEMENTATION
#include <HandmadeMath.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include <stb_image.h>

//
// Platform common
//
#include <platform_common.hpp>

Platform* platform = 0;
LornockMemory* lornockMemory = 0;

//
// Lornock code
//
#include <memory.hpp>

MemoryArena* tempMemory = 0;

// Utilities
#include <hmm_wrapper.cpp>
#include <friendly/math.cpp>
#include <friendly/string.cpp>
#include <globals.cpp>

// Real code
#include <assets.cpp>
#include <draw.cpp>
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

  draw_init();

  return 0;
}

extern "C" void lornockUpdate(LornockMemory* m) {
  lornockMemory = m;
  lornockData = (LornockData*) m->permanentStorage;

  tempMemory = &lornockData->tempArena;

  dbg_assert(sizeof(*lornockData) < m->permanentStorageSize);

  if (!m->initialized) {
    uint8* head = (uint8*) m->permanentStorage + sizeof(*lornockData);
    MemoryIndex arenaSize = (m->permanentStorageSize - sizeof(*lornockData))/2;

    memoryArena_init(
        &lornockData->tempArena,
        arenaSize,
        head);

    head += arenaSize;

    memoryArena_init(
        &lornockData->actionsArena,
        arenaSize,
        head);

    stbi_set_flip_vertically_on_load(true);

    stateInit(&lornockData->state, STATE_game);

    // Refresh assets
    updateAssets();

    // TODO: refactor into drawInit or something
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m->initialized = true;
  }

  updateAssets();

  stateUpdate(&lornockData->state);
}
