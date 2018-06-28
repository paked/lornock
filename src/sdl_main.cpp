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

// Include SDL
#include <SDL.h>

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
const char* gameLibPath = "./liblornock.so";
typedef void (*GameLibFunction)(LornockMemory*);

bool gameLibValid = false;
ino_t gameLibFileID = 0;
void* gameLibHandle = 0;

GameLibFunction gameLibUpdateFunction;

ino_t getFileID(const char *fname) {
  struct stat attr;

  if (stat(fname, &attr) != 0) {
    logfln("ERROR: Can't get file ID of '%s'", fname);

    attr.st_ino = 0;
  }

  return attr.st_ino;
}

bool gameLibNeedsToReload() {
  return getFileID(gameLibPath) != gameLibFileID;
}

void gameLibUnload() {
  if (gameLibHandle == 0) {
    return;
  }

  dlclose(gameLibHandle);

  gameLibHandle = 0;
  gameLibValid = false;
}

bool gameLibLoad() {
  if (gameLibHandle != 0) {
    gameLibUnload();
  }

  gameLibHandle = dlopen(gameLibPath, RTLD_NOW | RTLD_LOCAL);

  if (gameLibHandle == 0) {
    logln("ERROR: Cannot load game liblornock");

    return false;
  }

  gameLibUpdateFunction = (GameLibFunction) dlsym(gameLibHandle, "update");

  if (!gameLibUpdateFunction) {
    logln("ERROR: Could not find update function");

    return false;
  }

  gameLibValid = true;

  return true;
}

int main(void) {
  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    logln("ERROR: Could not init SDL video");

    return 1;
  }

  SDL_GL_LoadLibrary(0);

  // Configure OpenGL context
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
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
    logln("ERROR: Could not create OpenGL context");

    return 1;
  }

  if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
    logln("ERROR: Could not load glad");

    return 1;
  }

  // Load game code
  if (getFileID(gameLibPath) == 0) {
    logln("ERROR: Game lib path does not exist");

    return -1;
  }

  if (!gameLibLoad()) {
    logln("ERROR: Failed to load game lib. Exiting game.");

    return -1;
  }

  // Set up memory
  // TODO(harrison): Investigate if we need to `mprotect` these
  lornockMemory.permanentStorageSize = megabytes(uint64(LORNOCK_PERMANENT_MEMORY_STORAGE_SIZE));
  lornockMemory.permanentStorage = mmap(0, lornockMemory.permanentStorageSize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

  lornockMemory.transientStorageSize = megabytes(uint64(LORNOCK_TRANSIENT_MEMORY_STORAGE_SIZE));
  lornockMemory.transientStorage = mmap(0, lornockMemory.transientStorageSize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

  SDL_Event event;
  while (!platform.quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        platform.quit = true;
      }
    }

    if (gameLibNeedsToReload()) {
      bool ok = gameLibLoad();

      if (!ok) {
        logln("WARNING: can't load liblornock");
      }
    }

    if (gameLibValid) {
      gameLibUpdateFunction(&lornockMemory);
    }

    glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    SDL_GL_SwapWindow(window);
  }

  SDL_DestroyWindow(window);

  SDL_Quit();

  return 0;
}
