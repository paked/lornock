#include <player.hpp>

#include <stdio.h>

#include <e/camera.hpp>

#include <item.hpp>

Player::Player(float x, float y) {
  sprite = new Sprite("test.png", x, y);

  sprite->maxVelocity = maxVelocity;
  sprite->drag = drag;

  tbBGSprite = new Spritesheet("toolbar.png", 16, 16);
  tbItemSprite = new Spritesheet("toolbar_items.png", 16, 16);

  for (int i = 0; i < tbItemSlots; i++) {
    tbItems.push_back(0);
  }

  tbBGLayer = new Tilelayer(tbBGSprite, 0, 0, getTBBGLayerData(), DEPTH_UI + DEPTH_BELOW);
  tbItemLayer = new Tilelayer(tbItemSprite, 0, 0, getTBItemLayerData(), DEPTH_UI);
}

void Player::start() {
  reg(sprite);

  reg(tbBGLayer);
  reg(tbItemLayer);

  int tbX = (scene->camera->getWidth() - tbBGLayer->getWidth())/2;
  int tbY = scene->camera->getHeight() - tbBGLayer->getHeight();

  tbBGLayer->x = tbItemLayer->x = tbX;
  tbBGLayer->y = tbItemLayer->y = tbY;
  tbBGLayer->hud = tbItemLayer->hud = true;
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

  // have pretty high tolerance here bc we're dealing with a specific set of possibilities (1, -1, 0, 0.7121*)
  if (!(fabs(dir.x - lastMove.x) < 0.01 && fabs(dir.y - lastMove.y) < 0.01)) {
    actionDirty = true;
    char buffer[512];

    sprintf(buffer,"pos=(%f,%f) vel=(%f,%f) acc=(%f,%f)",
        sprite->x, sprite->y,
        sprite->velocity.x, sprite->velocity.y,
        sprite->acceleration.x, sprite->acceleration.y);

    action = {
      "MOVE",
      std::string(buffer)
    };

    lastMove = dir;
  } else {
    printf("no changes\n");
  }

  tbItemLayer->data = getTBItemLayerData();

  Entity::tick(dt);
}

Tilelayer::Data Player::getTBBGLayerData() {
  Tilelayer::Data data = { {} };

  for (int i = 0; i < tbItemSlots; i++) {
    int t = 1;

    if (i == 0) {
      t = 0;
    } else if (i == tbItemSlots - 1) {
      t = 2;
    }

    data[0].push_back(t);
  }

  return data;
}

Tilelayer::Data Player::getTBItemLayerData() {
  Tilelayer::Data data = { {} };

  for (int i = 0; i < tbItemSlots; i++) {
    data[0].push_back(Item::items[tbItems[i]].iconIndex);
  }

  return data;
}
