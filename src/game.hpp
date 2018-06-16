#pragma once

#include <SDL.h>

#include <e/scene.hpp>

struct Game : Scene {
  Game();

  bool load();

  void start();
};
