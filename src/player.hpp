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

  float acceleration = 8;
  Point maxVelocity = Point(5, 5);
  Point drag = Point(0.80, 0.80);

  Sprite* sprite;
};
