/*
 * Lornock. A time-travel survival farming game. I hope I finish it.
 *
 * This file is the starting point for the platform section of Lornocks code.
 * To see the actual logic check out `lornock.cpp`.
 */

// Include external libraries
#include <sys/mman.h>
#include <sys/stat.h>
#include <dlfcn.h>

// Include internal libraries

// Include common
#include <platform_common.hpp>

// Config
#define WINDOW_NAME "Lornock"
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

// Globals
SDL_Window* window;
SDL_GLContext glContext;

LornockMemory lornockMemory = {0};
Platform platform = {0};

// Hot reloading
// TODO(harrison): Refactor this out into a separate platform_linux.h (and create a platform_win32.h)
const char* gameLibPath = "./liblornock.so";

bool gameLibValid = false;
time_t gameLibFileTime;
void* gameLibHandle = 0;

typedef void (*GameLibUpdateFunction)(LornockMemory*);
GameLibUpdateFunction gameLibUpdateFunction;
typedef int (*GameLibInitFunction)();
GameLibInitFunction gameLibInitFunction;

ino_t getFileID(const char *fname) {
  struct stat attr;

  if (stat(fname, &attr) != 0) {
    logfln("ERROR: Can't get file ID of '%s'", fname);

    attr.st_ino = 0;
  }

  return attr.st_ino;
}

time_t getFileTime(const char *fname) {
  struct stat attr;

  if (stat(fname, &attr) != 0) {
    logfln("ERROR: Can't get file ID of '%s'", fname);

    attr.st_mtime = 0;
  }

  return attr.st_mtime;
}

bool gameLibNeedsToReload() {
  return getFileTime(gameLibPath) > gameLibFileTime;
}

void gameLibUnload() {
  if (gameLibHandle == 0) {
    return;
  }

  dlclose(gameLibHandle);

  gameLibValid = false;
  gameLibHandle = 0;
  gameLibFileTime = 0;
}

bool gameLibLoad() {
  if (gameLibHandle != 0) {
    SDL_Delay(200);
    gameLibUnload();
  }

  gameLibHandle = dlopen(gameLibPath, RTLD_NOW | RTLD_LOCAL);

  if (gameLibHandle == 0) {
    logln("ERROR: Cannot load game liblornock");

    return false;
  }

  gameLibUpdateFunction = (GameLibUpdateFunction) dlsym(gameLibHandle, "updateLornock");

  if (!gameLibUpdateFunction) {
    logln("ERROR: Could not find update function");

    return false;
  }

  gameLibInitFunction = (GameLibInitFunction) dlsym(gameLibHandle, "initLornock");

  if (!gameLibUpdateFunction) {
    logln("ERROR: Could not find init function");

    return false;
  }

  gameLibValid = true;
  gameLibFileTime = getFileTime(gameLibPath);

  return true;
}

int main(void) {
  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    logln("ERROR: Could not init SDL video");

    return 1;
  }

  // Configure OpenGL context
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  window = SDL_CreateWindow(
      WINDOW_NAME,
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      WINDOW_WIDTH,
      WINDOW_HEIGHT,
      SDL_WINDOW_OPENGL);

  if (!window) {
    logln("ERROR: Could not create SDL window");

    return 1;
  }

  glContext = SDL_GL_CreateContext(window);

  if (!glContext) {
    logfln("ERROR: Could not create OpenGL context: %s", SDL_GetError());

    return 1;
  }

  // Set up memory
  // TODO(harrison): Investigate if we need to `mprotect` these
  lornockMemory.permanentStorageSize = megabytes(uint64(LORNOCK_PERMANENT_MEMORY_STORAGE_SIZE));
  lornockMemory.permanentStorage = mmap(0, lornockMemory.permanentStorageSize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

  lornockMemory.transientStorageSize = megabytes(uint64(LORNOCK_TRANSIENT_MEMORY_STORAGE_SIZE));
  lornockMemory.transientStorage = mmap(0, lornockMemory.transientStorageSize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

  if (getFileID(gameLibPath) == 0) {
    logln("ERROR: Game lib path does not exist");

    return -1;
  }

  // Load game code
  if (!gameLibLoad()) {
    logln("ERROR: Failed to load game lib. Exiting game.");

    return -1;
  }

  // Run init code
  if (gameLibInitFunction() != 0) {
    logln("ERROR: liblornock init code was not successful");

    return -1;
  }

  SDL_Event event;
  while (!platform.quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        platform.quit = true;
      }
    }

    // TODO(harrison): only reload code in debug mode
    if (gameLibNeedsToReload()) {
      bool ok = gameLibLoad();

      if (!ok) {
        logln("WARNING: can't load liblornock");
      } else {
        logln("INFO: Reloaded liblornock!");

        // Run init code
        if (gameLibInitFunction() != 0) {
          logln("ERROR: liblornock init code was not successful");

          return -1;
        }
      }
    }

    if (gameLibValid) {
      gameLibUpdateFunction(&lornockMemory);
    }

    SDL_GL_SwapWindow(window);
  }

  SDL_DestroyWindow(window);

  SDL_Quit();

  return 0;
}
