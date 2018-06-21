#pragma once

#include <e/entity.hpp>
#include <e/sprite.hpp>
#include <e/timer.hpp>

#include <action.hpp>

struct PastPlayer : Entity {
  PastPlayer(ActionCollector* ac, Action a);

  void start();
  void tick(float dt);
  void postTick();

  void findNextAction();

  ActionCollector* actionCollector;
  Action actionCurrent;
  Action actionNext;
  bool hasNextAction = false;

  int sequence;

  Sprite* sprite;

  int posStartTime;
  int posDuration;
  Point posCurrent;
  Point posNext;

  int convertTicksToMS(int t);
};
