#pragma once

#include <e/entity.hpp>

struct Component {
  virtual void start();
  virtual void tick(float dt);
  virtual void postTick();
  virtual void render();

  Entity *entity = nullptr;
  bool active = true;
};
