#pragma once

#include <e/component.hpp>
#include <e/spritesheet.hpp>

struct Tilelayer : Component {
  typedef std::vector<std::vector<int>> Data;

  Tilelayer(Spritesheet* ts, float x, float y, Data d, float dp = 0);

  void tick(float dt);
  float getWidth();
  float getHeight();
  
  float x;
  float y;
  float alpha = 1.0;
  bool hud = false;

  float localDepth;

  Spritesheet* tileset;
  Data data;
};
