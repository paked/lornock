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
  if (moving) {
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

    if (actionCurrent.name == "MOVE") {
      /*
       * This section is a bit of a mess, so here's an explanation until I clear it up:
       *
       * We do not always know that the next action will be a moving action (the player could always mine, jump, check inventory, etc.) so we look through the upcoming actions (skipping the ones which are not "MOVE"s in order to find the next move.
       *
       * It could (should) probably be simplified, but I am not in the right headspace to do this right now.
       */
      Action moveTarget;

      moving = false;
      int targetSequence = sequence + 1;
      for (int i = 0; i < actionCollector->actions.size(); i++) {
        Action c = actionCollector->actions[i];

        if (c.getSequence() != targetSequence) {
          continue;
        }

        if (c.name != "MOVE") {
          targetSequence++;
        }

        moving = true;
        moveTarget = c;
      }

      if (moving) {
        posCurrent = actionCurrent.getPoint("pos");
        posNext = moveTarget.getPoint("pos");

        posStartTime = SDL_GetTicks();
        posDuration = convertTicksToMS(moveTarget.getTime() - actionCurrent.getTime());
      }
    } else if (actionCurrent.name == "JUMP") {
      printf("Despawning past player...\n");
      // TODO: need to remove this object here
      active = false;
    } else {
      printf("Unknown action: %s\n", actionCurrent.name.c_str());
    }
  }

  Entity::postTick();
}

// Next action is the next SEQUENTIAL and CHRONOLOGICAL action.
void PastPlayer::findNextAction() {
  hasNextAction = false;

  actionNext = Action{"INVALID"};

  for (int i = 0; i < actionCollector->actions.size(); i++) {
    Action a = actionCollector->actions[i];

    if (a.getSequence() == sequence + 1 && actionCurrent.getTime() <= a.getTime()) {
      actionNext = a;

      hasNextAction = true;
    }
  }
}

int PastPlayer::convertTicksToMS(int t) {
  return t * (1000/actionCollector->ticks);
}
