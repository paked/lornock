/*
 * Lornock. A time-travel survival farming game. I hope I finish it.
 *
 * This file is the starting point for the platform section of Lornocks code.
 * To see the actual logic check out `lornock.cpp`.
 */

// Include external libraries
#include <sys/mman.h> // mmap
#include <sys/stat.h> // stat
#include <dlfcn.h> // dlopen,dlsym
#include <unistd.h> // pread
#include <fcntl.h>
#include <errno.h>

#include <SDL.h>

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

// File IO
void linuxLoadFromFile(const char* path, void** data, uint32* len) {
  *data = 0;
  *len = 0;

  struct stat attr;

  if (stat(path, &attr) != 0) {
    logfln("ERROR: failed to find file: %s", path);
    return;
  }

  int64 readSizeExpected = attr.st_size;

  int fd = open(path, O_RDONLY);
  if (fd == -1) {
    logfln("ERROR: unable to open file. fd: %d", fd);
  }

  void* readData = mmap(0, readSizeExpected, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  int64 readSizeGot = read(fd, readData, readSizeExpected);

  if (readSizeGot != readSizeExpected) {
    logfln("ERROR: could not read file (read: %zd wanted: %zd) (errno: %d)!", readSizeExpected, readSizeGot, errno);
  }

  *data = readData;
  *len = (uint32) readSizeExpected;

  close(fd);
}

// Hot reloading
// TODO(harrison): Refactor this out into a separate platform_linux.h (and create a platform_win32.h)
const char* gameLibPath = "./liblornock.so";

bool gameLibValid = false;
time_t gameLibFileTime;
void* gameLibHandle = 0;

typedef void (*GameLibUpdateFunction)(LornockMemory*);
GameLibUpdateFunction gameLibUpdateFunction;
typedef int (*GameLibInitFunction)(Platform*);
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

  gameLibUpdateFunction = (GameLibUpdateFunction) dlsym(gameLibHandle, "lornockUpdate");

  if (!gameLibUpdateFunction) {
    logln("ERROR: Could not find update function");

    return false;
  }

  gameLibInitFunction = (GameLibInitFunction) dlsym(gameLibHandle, "lornockInit");

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

  if (SDL_GL_SetSwapInterval(1) < 0) {
    logfln("ERROR: could not set SDL swap interval %s", SDL_GetError());

    // TODO(harrison): create an internal system for handling framerate if
    // vsync doesn't exist
  }

  // Configure platform
  platform.fps = 60; // TODO(harrison): set this dynamically based on the actual screen refresh-rate
  platform.glLoadProc = SDL_GL_GetProcAddress;
  platform.loadFromFile = linuxLoadFromFile;
  platform.deltaTime = 1/platform.fps;

  // Set up memory
  // TODO(harrison): Investigate if we need to `mprotect` these
  // TODO(harrison): Have error checks which verify we get this memory
  lornockMemory.permanentStorageSize = LORNOCK_PERMANENT_MEMORY_STORAGE_SIZE;
  lornockMemory.permanentStorage = mmap(0, lornockMemory.permanentStorageSize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

  lornockMemory.transientStorageSize = LORNOCK_TRANSIENT_MEMORY_STORAGE_SIZE;
  lornockMemory.transientStorage = mmap(0, lornockMemory.transientStorageSize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

  // Check if game lib exists
  if (getFileID(gameLibPath) == 0) {
    logln("ERROR: Cannot find liblornock");

    return -1;
  }

  // Load game code
  if (!gameLibLoad()) {
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
    uint64 timeFrameStart = SDL_GetPerformanceCounter();

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

        if (gameLibInitFunction(&platform) != 0) {
          logln("ERROR: liblornock init code was not successful");

          return -1;
        }
      }
    }

    if (gameLibValid) {
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
