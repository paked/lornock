#pragma once

#include <SDL.h>

#include <config.hpp>

// Pretty much a global dump for SDL level things
struct Core {
  static SDL_Window* window;
  static SDL_Renderer* renderer;

  static SDL_Color clear;

  static void init();
};
