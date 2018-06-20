#pragma once

#include <SDL.h>

#include <e/scene.hpp>
#include <e/input.hpp>

#include <player.hpp>
#include <asteroid.hpp>
#include <action.hpp>

struct Game : Scene {
  Game();

  bool load();
  void start();
  void tick(float dt);

  Player* player;
  Asteroid* asteroid;

  Input save = Input(SDL_SCANCODE_RETURN);

  ActionCollector* actionCollector;
  int acUpTo = 0;
};
