#pragma once

#include <e/entity.hpp>
#include <e/sprite.hpp>
#include <e/input.hpp>

struct Player : Entity {
  Player(float x, float y);

  void start();
  void tick(float dt);

  Input moveLeft = Input(SDL_SCANCODE_A);
  Input moveRight = Input(SDL_SCANCODE_D);
  Input moveUp = Input(SDL_SCANCODE_W);
  Input moveDown = Input(SDL_SCANCODE_S);

  float acceleration = 15;
  Point maxVelocity = Point(3, 3);
  Point drag = Point(0.9995, 0.9995);

  Sprite* sprite;
};
