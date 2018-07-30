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
// Lornock code
//

// Globals
#include <platform_common.hpp>

Platform* platform = 0;
LornockMemory* lornockMemory = 0;
MemoryArena* tempMemory = 0;

// Utilities
#include <globals.cpp>
#include <hmm_wrapper.cpp>
#include <friendly/math.cpp>
#include <friendly/string.cpp>
#include <memory.hpp>

// Real code
#include <serializer.cpp>
#include <assets.cpp>
#include <lornock_data.cpp>
#include <draw.cpp>
#include <ui.cpp>
#include <states.cpp>

#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif

// NOTE(harrison): init is ran every time the DLL is loaded. It should not set
// any state, as we want state to persist between hot reloads.
EXPORT int lornock_init(Platform* p) {
  platform = p;

  if (!gladLoadGLLoader(p->glLoadProc)) {
    logln("ERROR: Could not load glad");

    return 1;
  }

  draw_init();

  logln("INFO: Init'd code!");

  return 0;
}

EXPORT void lornock_clean(LornockMemory* m) {
  draw_clean();

  logln("INFO: Cleaned code!");
}

EXPORT void lornock_update(LornockMemory* m) {
  lornockMemory = m;
  lornockData = (LornockData*) m->permanentStorage;

  tempMemory = &lornockData->tempArena;

  ensure(sizeof(*lornockData) < m->permanentStorageSize);

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

    draw_init();

    state_init(&lornockData->state, STATE_game);

    m->initialized = true;
  }

  lornockData_assetsUpdate();

  state_update(&lornockData->state);
}

#undef EXPORT
