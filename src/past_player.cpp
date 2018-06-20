#include <past_player.hpp>

PastPlayer::PastPlayer(ActionCollector* ac, Action a) : actionCollector(ac) {
  Point pos = a.getPoint("pos");
  sequence = a.s;

  sprite = new Sprite("test.png", pos.x, pos.y);
}

void PastPlayer::start() {
  Point maxVelocity = Point(5, 5);
  Point drag = Point(0.80, 0.80);

  sprite->maxVelocity = maxVelocity;
  sprite->drag = drag;

  reg(sprite);
}

void PastPlayer::tick(float dt) {
  Entity::tick(dt);

  if (hasNextAction && actionCollector->time >= nextAction.t) {
    Action a = nextAction;

    if (nextAction.name == "MOVE") {
      Point pos = a.getPoint("pos");
      Point vel = a.getPoint("vel");
      Point acc = a.getPoint("acc");

      sprite->x = pos.x;
      sprite->y = pos.y;

      sprite->velocity.x = vel.x;
      sprite->velocity.y = vel.y;

      sprite->acceleration.x = acc.x;
      sprite->acceleration.y = acc.y;
    } else {
      printf("I don't know what the fuck to do with that action!\n");
    }

    findNextAction();
  }
}

void PastPlayer::findNextAction() {
  hasNextAction = false;

  for (int i = 0; i < actionCollector->actions.size(); i++) {
    Action a = actionCollector->actions[i];

    if (a.s == sequence + 1) {
      nextAction = a;

      hasNextAction = true;
    }
  }
}
