#pragma once

#include <SDL.h>

#include <e/scene.hpp>
#include <e/input.hpp>
#include <e/info_asset.hpp>

#include <player.hpp>
#include <past_player.hpp>
#include <asteroid.hpp>
#include <action.hpp>

struct Game : Scene {
  Game();

  bool load();
  void start();
  void collisions();
  void tick(float dt);
  void postTick();

  void jumpTo(int t);

  Player* player;
  Asteroid* asteroid;

  Group<PastPlayer>* pastPlayers;

  Input save = Input(SDL_SCANCODE_RETURN);
  Input jump = Input(SDL_SCANCODE_TAB);

  InfoAsset worldAsset;

  ActionCollector* actionCollector;
};
