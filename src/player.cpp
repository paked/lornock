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

Tilelayer::Data Player::getTBBGLayerData(int slots) {
  Tilelayer::Data data = { {} };

  for (int i = 0; i < slots; i++) {
    int t = 1;

    if (i == 0) {
      printf("0\n");
      t = 0;
    } else if (i == slots - 1) {
      t = 2;
    }

    data[0].push_back(t);
  }

  return data;
}

Tilelayer::Data Player::getTBItemLayerData(int slots) {
  Tilelayer::Data data = { {} };

  for (int i = 0; i < slots; i++) {
    data[0].push_back(0);
  }

  return data;
}
