#pragma once

#include <e/entity.hpp>
#include <e/spritesheet.hpp>

struct Tilelayer : Entity {
  typedef std::vector<std::vector<int>> Data;

  Tilelayer(Spritesheet* ts, Data d, float dp);
  
  Spritesheet* tileset;
  Data data;

  float alpha = 1.0;

  void tick(float dt);
};
