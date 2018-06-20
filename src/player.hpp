#pragma once

#include <e/entity.hpp>
#include <e/sprite.hpp>
#include <e/spritesheet.hpp>
#include <e/input.hpp>
#include <e/tilelayer.hpp>

#include <action.hpp>

struct Player : Entity {
  Player(float x, float y);

  void start();
  void tick(float dt);

  Tilelayer::Data getTBBGLayerData();
  Tilelayer::Data getTBItemLayerData();

  Sprite* sprite;

  // Inputs
  Input use = Input(SDL_SCANCODE_SPACE);
  Input moveLeft = Input(SDL_SCANCODE_A);
  Input moveRight = Input(SDL_SCANCODE_D);
  Input moveUp = Input(SDL_SCANCODE_W);
  Input moveDown = Input(SDL_SCANCODE_S);

  // Movement related variables
  float acceleration = 8;
  Point maxVelocity = Point(5, 5);
  Point drag = Point(0.80, 0.80);
  Point lastMove;

  // Toolbar stuffs
  int tbItemSlots = 5;
  std::vector<int> tbItems;

  Spritesheet* tbBGSprite;
  Spritesheet* tbItemSprite;
  Tilelayer* tbBGLayer;
  Tilelayer* tbItemLayer;

  bool actionDirty = false;
  Action action;
};
