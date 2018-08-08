/*
 * Lornock. A time-travel survival farming game. I hope I finish it.
 *
 * This file is the starting point for the platform section of Lornocks code.
 * To see the actual logic check out `lornock.cpp`.
 */

// Include external libraries
#include <SDL.h>
#include <time.h>
#include <assert.h>

// Include common
#include <platform_common.hpp>

// Include internal libraries
#define ensure(stmt) assert((stmt))
#include <memory.hpp>
#undef ensure

typedef void (*GameLibUpdateFunction)(LornockMemory*);
GameLibUpdateFunction gameLibUpdateFunction;

typedef int (*GameLibInitFunction)(Platform*);
GameLibInitFunction gameLibInitFunction;

typedef void (*GameLibCleanFunction)(LornockMemory*);
GameLibCleanFunction gameLibCleanFunction;

// Include platform code
#ifdef __linux__
#include <sdl_linux.cpp>

#define ALLOCATE_MEMORY_FUNC linux_allocateMemory
#define FREE_MEMORY_FUNC munmap

#define LOAD_FROM_FILE_FUNC linux_loadFromFile
#define WRITE_TO_FILE_FUNC linux_writeToFile
#define LIB_NEEDS_RELOADING_FUNC linux_libNeedsReloading
#define LIB_RELOAD_FUNC linux_libReload
#define LIB_IS_VALID_FUNC linux_libIsValid
#define LIB_EXISTS_FUNC linux_libExists

#elif _WIN32
#include <sdl_win32.cpp>

// Implemented
#define ALLOCATE_MEMORY_FUNC win32_allocateMemory
#define FREE_MEMORY_FUNC win32_freeMemory

// Stubbed
#define LOAD_FROM_FILE_FUNC win32_loadFromFile
#define WRITE_TO_FILE_FUNC win32_writeToFile
#define LIB_NEEDS_RELOADING_FUNC win32_libNeedsReloading
#define LIB_RELOAD_FUNC win32_libReload
#define LIB_IS_VALID_FUNC win32_libIsValid
#define LIB_EXISTS_FUNC win32_libExists

#else
#error This platform is not supported. Add the relevant sdl_<plat>.cpp implementation file with the required functions.
#endif

// Config
#define WINDOW_NAME "Lornock"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_FULLSCREEN false

// Globals
SDL_Window* window;
SDL_GLContext glContext;

LornockMemory lornockMemory = {0};
Platform platform = {0};

void loadFromFileAsArena(const char* path, MemoryArena* ma) {
  void* data;
  uint32 len;

  LOAD_FROM_FILE_FUNC(path, &data, &len);

  assert(len != 0);

  MemoryIndex arenaSize = len + sizeof(MemoryBlock) + 1;
  uint8* arenaBuffer = (uint8*) ALLOCATE_MEMORY_FUNC(arenaSize);

  memoryArena_init(ma, arenaSize, arenaBuffer);

  void* start = memoryArena_pushSize(ma, (MemoryIndex) len);

  memcpy(start, data, len);

  // TODO(harrison): free `data`
}

void writeArenaToFile(const char* path, MemoryArena* ma) {
  MemoryIndex totalSize = 0;

  {
    for (MemoryBlock* block = ma->first; block != 0; block = block->next) {
      totalSize += block->size - sizeof(MemoryBlock);
    }
  }

  MemoryIndex index = 0;
  uint8* mem = (uint8*) ALLOCATE_MEMORY_FUNC(totalSize);

  for (MemoryBlock* block = ma->first; block != 0; block = block->next) {
    MemoryIndex size = block->size - sizeof(MemoryBlock);
    memcpy(mem + index, block->start, size);

    index += size;
  }

  WRITE_TO_FILE_FUNC(path, mem, index);

  FREE_MEMORY_FUNC(mem, totalSize);
}

void showErrorBox(const char* title, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  uint64 characterCount = vsnprintf(0, 0, fmt, args)+1;
  va_end(args);

  char *text = (char *) ALLOCATE_MEMORY_FUNC(characterCount);

  va_start(args, fmt);
  vsnprintf(text, characterCount, fmt, args);
  va_end(args);

  if (SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, text, NULL) != 0) {
    logfln("ERROR: could not open message box (%s: %s)", title, text);
  }

  FREE_MEMORY_FUNC(text, characterCount);

  exit(1);
}

void toggleFullscreen() {
  if (SDL_SetWindowFullscreen(window, platform.fullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
    logfln("ERROR: Could not toggle window fullscreen: %s", SDL_GetError());

    return;
  }

  logln("toggled fullscreen");

  platform.fullscreen = !platform.fullscreen;
}

// Key conversion
uint32 keySDLToPlatform(SDL_KeyboardEvent event) {
  uint32 key = KEY_unknown;
  uint32 scancode = event.keysym.scancode;

  if (scancode == SDL_SCANCODE_LSHIFT) {
    key = KEY_shift;
  } else if (scancode == SDL_SCANCODE_GRAVE) {
    key = KEY_grave;
  } else if (scancode == SDL_SCANCODE_LCTRL) {
    key = KEY_ctrl;
  } else if (scancode == SDL_SCANCODE_TAB) {
    key = KEY_tab;
  } else if (scancode == SDL_SCANCODE_SPACE) {
    key = KEY_space;
  } else if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z) {
    key = KEY_a + (scancode - SDL_SCANCODE_A);
  } else if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_0) {
    key = KEY_1 + (scancode - SDL_SCANCODE_1);
  }

  return key;
}

int main(int argc, char** argv) {
  srand(time(NULL));

  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    logln("ERROR: Could not init SDL video");

    return 1;
  }

  // Configure OpenGL context
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

  window = SDL_CreateWindow(
      WINDOW_NAME,
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      WINDOW_WIDTH,
      WINDOW_HEIGHT,
      SDL_WINDOW_OPENGL);

  if (!window) {
    logfln("ERROR: Could not create SDL window: %s", SDL_GetError());

    return 1;
  }

  glContext = SDL_GL_CreateContext(window);

  if (!glContext) {
    logfln("ERROR: Could not create OpenGL context: %s", SDL_GetError());

    return 1;
  }

  if (SDL_GL_SetSwapInterval(1) < 0) {
    logfln("ERROR: could not set SDL swap interval: %s", SDL_GetError());

    // TODO(harrison): create an internal system for handling framerate if
    // vsync doesn't exist
  }

  if (WINDOW_FULLSCREEN) {
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
  }

  // Configure platform
  platform.fps = 60; // TODO(harrison): set this dynamically based on the actual screen refresh-rate
  platform.glLoadProc = SDL_GL_GetProcAddress;
  platform.showErrorBox = showErrorBox;
  platform.loadFromFile = LOAD_FROM_FILE_FUNC;
  platform.loadFromFileAsArena = loadFromFileAsArena;
  platform.writeToFile = WRITE_TO_FILE_FUNC;
  platform.writeArenaToFile = writeArenaToFile;
  platform.windowWidth = WINDOW_WIDTH;
  platform.windowHeight = WINDOW_HEIGHT;
  platform.time = 0;
  platform.deltaTime = 1/platform.fps;

  // Set up memory
  // TODO(harrison): Investigate if we need to `mprotect` these
  // TODO(harrison): Have error checks which verify we get this memory
  lornockMemory.permanentStorageSize = LORNOCK_PERMANENT_MEMORY_STORAGE_SIZE;
  lornockMemory.permanentStorage = ALLOCATE_MEMORY_FUNC(lornockMemory.permanentStorageSize);

  lornockMemory.transientStorageSize = LORNOCK_TRANSIENT_MEMORY_STORAGE_SIZE;
  lornockMemory.transientStorage = ALLOCATE_MEMORY_FUNC(lornockMemory.transientStorageSize);

  // Check if game lib exists
  if (!(LIB_EXISTS_FUNC())) {
    logln("ERROR: Cannot find liblornock");

    return -1;
  }

  // Load game code
  if (!(LIB_RELOAD_FUNC())) {
    logln("ERROR: Failed to load game lib. Exiting game.");

    return -1;
  }

  if (gameLibInitFunction(&platform) != 0) {
    logln("ERROR: liblornock init code was not successful");

    return -1;
  }

  uint64 perfCounterFreq = SDL_GetPerformanceFrequency();

  SDL_Event event;
  while (!platform.quit) {
    platform.time = SDL_GetTicks();

    // Copy "now" key state into the last key state buffer, and reset the new key state
    for (uint32 i = 0; i < MAX_KEY; i++) {
      platform.keyStateLast[i] = platform.keyStateNow[i];
    }

    uint64 timeFrameStart = SDL_GetPerformanceCounter();

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          {
            platform.quit = true;
          } break;
        case SDL_WINDOWEVENT:
          {
            switch (event.window.event) {
              case SDL_WINDOWEVENT_SIZE_CHANGED:
                {
                  platform.windowWidth = event.window.data1;
                  platform.windowHeight = event.window.data2;
                } break;
            }

          } break;
        case SDL_KEYUP:
        case SDL_KEYDOWN:
          {
            uint32 key = keySDLToPlatform(event.key);

            if (key == KEY_unknown) {
              logln("Could not identify key!");

              break;
            }

            platform.keyStateNow[key] = event.key.state == SDL_PRESSED;
          } break;
      }
    }

    if ((platform.keyStateNow[KEY_grave] && !platform.keyStateLast[KEY_grave])) {
      logln("Toggling fullscreen...");
      toggleFullscreen();
    }

    // TODO(harrison): only reload code in debug mode
    if (LIB_NEEDS_RELOADING_FUNC()) {
      gameLibCleanFunction(&lornockMemory);

      bool ok = LIB_RELOAD_FUNC();

      if (!ok) {
        logln("WARNING: can't load liblornock");
      } else {
        logln("INFO: Reloaded liblornock!");

        if (gameLibInitFunction(&platform) != 0) {
          logln("ERROR: liblornock init code was not successful");

          return -1;
        }
      }
    }

    if (LIB_IS_VALID_FUNC()) {
      gameLibUpdateFunction(&lornockMemory);
    }

    SDL_GL_SwapWindow(window);

    uint64 timeFrameEnd = SDL_GetPerformanceCounter();
    uint64 timeElapsed = timeFrameEnd - timeFrameStart;

    real64 deltaTime = ((((real64)timeElapsed) / (real64)perfCounterFreq));
    // real64 fps = (real64)perfCounterFreq / (real64)timeElapsed;

    // printf("%f ms/f, %.02ff/s\n", deltaTime, fps);

    platform.deltaTime = deltaTime;
  }

  SDL_GL_DeleteContext(glContext);
  SDL_DestroyWindow(window);

  SDL_Quit();

  return 0;
}
