#include <past_player.hpp>

#include <e/math_util.hpp>

PastPlayer::PastPlayer(ActionCollector* ac, Action a) : actionCollector(ac) {
  Point pos = a.getPoint("pos");
  sequence = a.getSequence();

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
  if (hasNextAction && nextAction.getTime() < actionCollector->time) {
    Action a = nextAction;

    if (nextAction.name == "MOVE") {
      printf("MAKING MOVE\n");

      posBuffer = a.getPoint("pos");
      /*
      Point pos = a.getPoint("pos");
      Point vel = a.getPoint("vel");
      Point acc = a.getPoint("acc");

      sprite->x = pos.x;
      sprite->y = pos.y;

      sprite->velocity.x = vel.x;
      sprite->velocity.y = vel.y;*/

      /*sprite->acceleration.x = acc.x;
      sprite->acceleration.y = acc.y;*/

      printf("s: %d, acc: %s\n", a.getSequence(), a.params["acc"].c_str());
    } else {
      printf("I don't know what the fuck to do with that action!\n");
    }

    sequence = a.getSequence();

    findNextAction();
  }

  sprite->x = MathUtil::lerp(0.08, sprite->x, posBuffer.x);
  sprite->y = MathUtil::lerp(0.08, sprite->y, posBuffer.y);

  Entity::tick(dt);
}

void PastPlayer::findNextAction() {
  hasNextAction = false;

  for (int i = 0; i < actionCollector->actions.size(); i++) {
    Action a = actionCollector->actions[i];

    if (a.getSequence() == sequence + 1) {
      nextAction = a;

      hasNextAction = true;
    }
  }
}
