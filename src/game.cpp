#include <game.hpp>

#include <e/entity.hpp>
#include <e/sprite.hpp>
#include <e/resources.hpp>

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

  return ok;
}

void Game::start() {
  Scene::start();

  player = new Player(camera->getWidth()/2, camera->getHeight()/2);
  player->sprite->center();

  entities->add(player);

  Spritesheet* ts = new Spritesheet("tilemap.png", 16, 16);

  Tilelayer::Data mapData = {
    { 0,  1,  1,  1,  2  },
    { 8,  9,  9,  9,  10 },
    { 8,  9,  9,  9,  10 },
    { 8,  9,  9,  9,  10 },
    { 16, 17, 17, 17, 18 },
  };

  map = new Tilemap(ts, 0, 0);
  map->loadLayer(mapData, DEPTH_BELOW);
  map->addToGroup(entities);

  camera->target = Point(5 * 16 / 2, 5 * 16 / 2);
}
