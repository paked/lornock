#pragma once

#include <SDL.h>

#include <e/scene.hpp>

#include <player.hpp>
#include <e/tilemap.hpp>


struct Game : Scene {
  Game();

  bool load();
  void start();

  Player* player;
  Tilemap* map;
};
