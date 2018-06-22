#include <past_player.hpp>

#include <e/math_util.hpp>

PastPlayer::PastPlayer(ActionCollector* ac, Action a) : actionCollector(ac) {
  actionCurrent = a;
  sequence = a.getSequence();

  Point pos = a.getPoint("pos");

  sprite = new Sprite("test.png", pos.x, pos.y);
}

void PastPlayer::start() {
  Point maxVelocity = Point(5, 5);
  Point drag = Point(0.80, 0.80);

  sprite->maxVelocity = maxVelocity;
  sprite->drag = drag;

  findNextAction();

  reg(sprite);
}

void PastPlayer::tick(float dt) {
  if (hasNextAction) {
    float pc =  1.0 - ((float) ((posStartTime + posDuration) - SDL_GetTicks())) / (posDuration);

    if (pc >= 0 && pc <= 1) {
      sprite->x = MathUtil::lerpf(pc, posCurrent.x, posNext.x);
      sprite->y = MathUtil::lerpf(pc, posCurrent.y, posNext.y);   
    }
  }

  Entity::tick(dt);
}

void PastPlayer::postTick() {
  while (hasNextAction && actionNext.getTime() < actionCollector->time) {
    actionCurrent = actionNext;

    sequence = actionCurrent.getSequence();

    findNextAction();

    if (actionNext.name == "MOVE") {
      posCurrent = actionCurrent.getPoint("pos");
 
      posNext = actionNext.getPoint("pos");

      posStartTime = SDL_GetTicks();
      posDuration = convertTicksToMS(actionNext.getTime() - actionCurrent.getTime());
    } else {
      printf("Unknown action\n");
    }
  }

  Entity::postTick();
}

void PastPlayer::findNextAction() {
  hasNextAction = false;

  for (int i = 0; i < actionCollector->actions.size(); i++) {
    Action a = actionCollector->actions[i];

    if (a.getSequence() == sequence + 1) {
      actionNext = a;

      hasNextAction = true;
    }
  }
}

int PastPlayer::convertTicksToMS(int t) {
  return t * (1000/actionCollector->ticks);
}
