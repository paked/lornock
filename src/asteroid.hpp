#pragma once

#include <vector>

#include <e/sprite.hpp>
#include <e/collision.hpp>
#include <e/tilemap.hpp>
#include <e/entity.hpp>

struct Asteroid : Entity {
  const int size = 3;
  const int padding = 1;

  void start();
  void tick(float dt);

  Collision::TileHit getPickWithinOreSpace(Sprite* sprite);

  Tilelayer::Data getMapData();
  Tilelayer::Data getOreData();
  Tilelayer::Data getCollisionData();

  int layerBGIndex;
  int layerOreIndex;

  Tilelayer* layerBG;
  Tilelayer* layerOre;

  std::vector<std::vector<int>> ores = {
    { 0, 1, 0 },
    { 0, 0, 2 },
    { 1, 0, 0 }
  };

  Tilemap* map;
};
