#pragma once

#include <SDL.h>

struct Renderer;
struct Camera;
struct Entity;
template<class T> struct Group;

struct Scene {
  Scene();

  Renderer* renderer;
  Camera* camera;
  Group<Entity>* entities;

  virtual bool load();
  virtual void start();
  virtual void update(float dt);
  virtual void render(SDL_Renderer *r);

  bool shouldQuit = false;
};
