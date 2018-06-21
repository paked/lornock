#pragma once

#include <e/entity.hpp>
#include <e/sprite.hpp>

#include <action.hpp>

struct PastPlayer : Entity {
  PastPlayer(ActionCollector* ac, Action a);

  void start();
  void tick(float dt);

  void findNextAction();

  ActionCollector* actionCollector;
  Action nextAction;

  bool hasNextAction = false;

  int sequence;

  Sprite* sprite;

  Point posBuffer;
};
