#include <game.hpp>

#include <e/entity.hpp>
#include <e/sprite.hpp>
#include <e/resources.hpp>
#include <e/collision.hpp>

// fwd declaration for Scene
// TODO: figure out how to do this fwd declaration better
#include <e/camera.hpp>
#include <e/renderer.hpp>
#include <e/entity.hpp>
#include <e/group.hpp>

Game::Game() : Scene() {}

bool Game::load() {
  bool ok = Scene::load();

  ok |= Resources::load("tilemap.png");
  ok |= Resources::load("test.png");
  ok |= Resources::load("toolbar.png");
  ok |= Resources::load("toolbar_items.png");

  return ok;
}

void Game::start() {
  Scene::start();

  Spritesheet* ts = new Spritesheet("tilemap.png", 16, 16);

  Tilelayer::Data mapData = {
    { -1, -1, -1, -1, -1 },
    { -1, 0, 0, 0, -1 },
    { -1, 0, 0, 0, -1 },
    { -1, 0, 0, 0, -1 },
    { -1, -1, -1, -1, -1 },
  };

  Tilelayer::Data oreData = {
    { -1, -1, -1, -1, -1 },
    { -1, -1, 8, -1, -1 },
    { -1, -1, -1, -1, -1 },
    { -1, 8, -1, 9, -1 },
    { -1, -1, -1, -1, -1 },
  };

  Tilelayer::Data collisionData = {
    { 1, 1, 1, 1, 1 },
    { 1, -1, -1, -1, 1 },
    { 1, -1, -1, -1, 1 },
    { 1, -1, -1, -1, 1 },
    { 1, 1, 1, 1, 1 },
  };

  map = new Tilemap(ts, 50, 50);
  int layer = map->loadLayer(mapData, DEPTH_BELOW*2);
  map->loadLayer(oreData, DEPTH_BELOW);
  map->loadCollisionLayer(collisionData);

  map->addToGroup(entities);

  float mapWidth = map->layers[layer]->getWidth();
  float mapHeight = map->layers[layer]->getHeight();
  Point mapCenter = Point(map->x + mapWidth/2, map->y + mapHeight/2);

  player = new Player(mapCenter.x, mapCenter.y);
  player->sprite->center();

  entities->add(player);

  camera->target = mapCenter;
}

void Game::tick(float dt) {
  Collision::collide(player->sprite, map);
}
