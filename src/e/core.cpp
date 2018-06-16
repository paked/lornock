#include <e/core.hpp>

#include <SDL_ttf.h>

#include <config.hpp>

#include <stdio.h>

SDL_Window* Core::window = nullptr;
SDL_Renderer* Core::renderer = nullptr;
SDL_Color Core::clear;

void Core::init() {
  SDL_Init(SDL_INIT_VIDEO);

  if(TTF_Init() == -1) {
    printf("Unable to initialise SDL2_TTF: %s\n", TTF_GetError());
  }

  window = SDL_CreateWindow("game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_WIDTH, 0);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  SDL_CaptureMouse(SDL_TRUE);
}
