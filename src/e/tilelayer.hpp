#pragma once

#include <e/entity.hpp>
#include <e/spritesheet.hpp>

struct Tilelayer : Entity {
  typedef std::vector<std::vector<int>> Data;

  Tilelayer(Spritesheet* ts, float x, float y, Data d, float dp);
  
  float x;
  float y;

  Spritesheet* tileset;
  Data data;

  float alpha = 1.0;

  void tick(float dt);
};
