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
MemoryArena* tempMemory = 0;

//
// Lornock code
//

// Utilities
#include <hmm_wrapper.cpp>
#include <friendly/math.cpp>
#include <friendly/string.cpp>
#include <globals.cpp>

// Real code
#include <serializer.cpp>
#include <assets.cpp>
#include <lornock_data.cpp>
#include <draw.cpp>
#include <ui.cpp>
#include <states.cpp>

// NOTE(harrison): init is ran every time the DLL is loaded. It should not set
// any state, as we want state to persist between hot reloads.
extern "C" int lornock_init(Platform* p) {
  platform = p;

  if (!gladLoadGLLoader(p->glLoadProc)) {
    logln("ERROR: Could not load glad");

    return 1;
  }

  draw_init();

  logln("INFO: Init'd code!");

  return 0;
}

extern "C" void lornock_clean(LornockMemory* m) {
  draw_clean();

  logln("INFO: Cleaned code!");
}

extern "C" void lornock_update(LornockMemory* m) {
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

    {
      Serializer s;
      serializer_init(&s, SERIALIZER_MODE_WRITE, lornockData->tempArena);

      real32 t = 12.5f;
      serializer_real32(&s, &t);

      for (uint8 i = 20; i > 0; i--) {
        serializer_uint8(&s, &i);
      }

      writeArenaToFile("out", &s.buffer);
    }

    {

      MemoryArena arena;

      loadFromFileAsArena("out", &arena);

      Serializer s;
      serializer_init(&s, SERIALIZER_MODE_READ, arena);

      real32 res;

      serializer_real32(&s, &res);

      logfln("got: %f", res);

      {
        uint8 result = 0;

        for (uint8 i = 20; i > 0; i--) {
          serializer_uint8(&s, &result);

          logfln("result %u", result);
        }
      }

      /*
      {
        real32 result = 0;

        for (int i = 0; i < 5; i++) {
          serializer_real32(&s, &result);

          logfln("result: %f", result);
        }
      }*/
    }

    stbi_set_flip_vertically_on_load(true);

    draw_init();

    state_init(&lornockData->state, STATE_game);

    m->initialized = true;
  }

  lornockData_assetsUpdate();

  state_update(&lornockData->state);
}
