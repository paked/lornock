#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include <e/core.hpp>
#include <e/resources.hpp>
#include <e/scene.hpp>

#include <config.hpp>
#include <game.hpp>

Scene *game;

const float frameTimeMs = 1000.0/FPS;

void hook();

int main(int argc, char *argv[]) {
  printf("Initialising SDL... ");
  Core::init();
  printf("Done.\n");

  printf("Initialising game... ");
  game = new Game();
  printf("Done\n");

  printf("Loading game resources...\n");
  if (!game->load()) {
    printf("Failed.\n");

    return -1;
  }

  printf("Starting game...\n");
  game->start();
  printf("Done.\n");

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(hook, FPS, 1);
#else
  const int frameDelay = 1000/FPS;

  unsigned int frameStart;
  int frameTime;
  while (!game->shouldQuit) {
    frameStart = SDL_GetTicks();

    // int frameStart = SDL_GetTicks();

    hook();

    frameTime = SDL_GetTicks() - frameStart;

    if (frameDelay > frameTime) {
      SDL_Delay(frameDelay - frameTime);
    }

    /*
    int frameEnd = SDL_GetTicks();
    int frameDuration = frameEnd - frameStart;

    if (frameDuration < frameTimeMs) {
      int delayFor = frameTimeMs - frameDuration;

      SDL_Delay(frameTimeMs - frameDuration);
    }*/
  }
#endif

  Resources::clean();

  return 0;
}

void hook() {
  // int frameStart = SDL_GetTicks();

  game->update((frameTimeMs)/100);
/*
  int tickEnd = SDL_GetTicks();
  int tickDuration = tickEnd - frameStart;
*/
  SDL_Renderer* renderer = Core::renderer;

  SDL_SetRenderDrawColor(renderer, Core::clear.r, Core::clear.g, Core::clear.b, 255);
  SDL_RenderClear(renderer);

  game->render(renderer);
  /*
  int renderEnd = SDL_GetTicks();

  int renderDuration = renderEnd - tickEnd;
  int frameDuration = SDL_GetTicks() - frameStart;
  */

  SDL_RenderPresent(renderer);

  /*
  if (frameDuration > 16) {
    printf("WARNING: frame took too long to complete (%d vs %d) (t: %d, r: %d).\n", frameDuration, 16, tickDuration, renderDuration);
  }*/
}

int startGame() {
  return 0;
}
