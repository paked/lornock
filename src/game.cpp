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

  // Asteroid
  asteroid = new Asteroid();
  entities->add(asteroid);

  float mapWidth = asteroid->map->layers[0]->getWidth();
  float mapHeight = asteroid->map->layers[0]->getHeight();

  Point mapCenter = Point(
      asteroid->map->x + mapWidth/2,
      asteroid->map->y + mapHeight/2
  );

  // Player
  player = new Player(mapCenter.x, mapCenter.y);
  player->sprite->center();
  entities->add(player);

  // Camera
  camera->target = mapCenter;
}

void Game::tick(float dt) {
  Collision::collide(player->sprite, asteroid->map);

  if (player->use.justDown()) {
    Collision::TileHit th = asteroid->getPickWithinOreSpace(player->sprite);

    if (th.hit) {
      asteroid->ores[th.y][th.x] = 0;

      for (int i = 0; i < player->tbItemSlots; i++) {
        if (player->tbItems[i] != 0) {
          continue;
        }

        player->tbItems[i] = th.t;

        break;
      }
    }
  }
}
