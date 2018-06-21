#pragma once

#include <SDL.h>

struct Renderer;
struct Camera;
struct Entity;
template<class T> struct Group;

struct Scene {
  Scene();


  // Load is an overrideable function where you should preload any assets used in the Scene
  virtual bool load();
  // Start is an overrideable function called after all assets have been succesfully loaded, it is for initialising Entities
  virtual void start();
  // Tick is an overrideable function called after inputs have been updated, but before any entities are individually tick'd. Use it for any gameplay logic which can't be abstracted into an Entity easily.
  virtual void tick(float dt);
  virtual void postTick();
  virtual void collisions();

  // Update refreshes all inputs, calls the `Scene::tick` function then ``Entity::tick` and `Camera::tick`
  void update(float dt);
  // Render takes the render list produced by each entity during the `update` function and draws it to a canvas.
  void render(SDL_Renderer *r);

  Renderer* renderer;
  Camera* camera;
  Group<Entity>* entities;

  bool shouldQuit = false;
};
