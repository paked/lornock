/*
 * Lornock. A time-travel survival farming game. I hope I finish it.
 *
 * This file is the starting point for the platform section of Lornocks code.
 * To see the actual logic check out `lornock.cpp`.
 */

// Include external libraries
#include <sys/mman.h>

#include <SDL.h>

// Include common
#include <platform_common.hpp>

// Unity build files:
#include <lornock.cpp>

// Config
#define WINDOW_NAME "Lornock"
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

// Globals
SDL_Window* window;
SDL_GLContext glContext;

LornockMemory lornockMemory = {0};
Platform platform = {0};

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

    updateLornock(&lornockMemory);

    glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    SDL_GL_SwapWindow(window);
  }

  SDL_DestroyWindow(window);

  SDL_Quit();

  return 0;
}
