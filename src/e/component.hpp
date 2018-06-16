#pragma once

#include <e/entity.hpp>

struct Component {
  virtual void start();
  virtual void tick(float dt);

  Entity *entity = nullptr;
  bool active = true;
};
