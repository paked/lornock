#pragma once

#include <vector>

#include <SDL.h>

#include <e/scene.hpp>

const float DEPTH_BG = -100;
const float DEPTH_MG = 0;
const float DEPTH_FG = 100;
const float DEPTH_UI = 200;
const float DEPTH_ABOVE = 1;
const float DEPTH_BELOW = -1;

struct Component;

struct Entity {
  virtual void start();
  virtual void tick(float dt);
  virtual void postTick();

  float getDepth();

  void reg(Component* comp);

  Entity* parent = nullptr;
  Scene* scene;
  std::vector<Component*> components;

  bool active = true;
  float localDepth = DEPTH_MG;
};
