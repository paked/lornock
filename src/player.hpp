#pragma once

#include <e/entity.hpp>
#include <e/sprite.hpp>
#include <e/spritesheet.hpp>
#include <e/input.hpp>
#include <e/tilelayer.hpp>

struct Player : Entity {
  Player(float x, float y);

  void start();
  void tick(float dt);

  Tilelayer::Data getTBBGLayerData(int slots);
  Tilelayer::Data getTBItemLayerData(int slots);

  Sprite* sprite;

  // Movement related variables
  Input moveLeft = Input(SDL_SCANCODE_A);
  Input moveRight = Input(SDL_SCANCODE_D);
  Input moveUp = Input(SDL_SCANCODE_W);
  Input moveDown = Input(SDL_SCANCODE_S);

  float acceleration = 8;
  Point maxVelocity = Point(5, 5);
  Point drag = Point(0.80, 0.80);

  // Toolbar stuffs
  int tbItemSlots = 5;
  int tbActiveItemSlot = 0;

  Spritesheet* tbBGSprite;
  Spritesheet* tbItemSprite;
  Tilelayer* tbBGLayer;
  Tilelayer* tbItemLayer;
};
