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

  ok |= Resources::load("test.png");

  return ok;
}

void Game::start() {
  Scene::start();

  Entity* e = new Entity();
  entities->add(e);

  Sprite* sprite = new Sprite("test.png", camera->getWidth()/2, camera->getHeight()/2);
  sprite->center();

  e->reg(sprite);
}
