#include <player.hpp>

Player::Player(float x, float y) {
  sprite = new Sprite("test.png", x, y);

  sprite->maxVelocity = maxVelocity;
  sprite->drag = drag;
}

void Player::start() {
  reg(sprite);
}

void Player::tick(float dt) {
  Point dir = Point(0, 0);

  if (moveLeft.down()) {
    dir.x = -1;
  }

  if (moveRight.down()) {
    dir.x = 1;
  }

  if (moveDown.down()) {
    dir.y = 1;
  }

  if (moveUp.down()) {
    dir.y = -1;
  }

  dir = dir.unit();

  sprite->acceleration.x = acceleration * dir.x;
  sprite->acceleration.y = acceleration * dir.y;

  Entity::tick(dt);
}
