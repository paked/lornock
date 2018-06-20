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

#include <past_player.hpp>

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

  // Action mgmt
  actionCollector = new ActionCollector();

  actionCollector->open();

  actionCollector->interval.go();

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

  actionCollector->add({
    "SPAWN",
    {
      { "P", "0" },
      { "pos", "(35.0,35.0)" },
    }
  });

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

  if (actionCollector->interval.done()) {
    for (acUpTo; acUpTo < actionCollector->actions.size(); acUpTo++) {
      Action a = actionCollector->actions[acUpTo];

      if (a.getTime() > actionCollector->time) {
        break;
      }

      if (a.name == "SPAWN") {
        PastPlayer* pp = new PastPlayer(actionCollector, a);

        entities->add(pp);
      } else if (a.name == "MOVE") {
        // We don't handle MOVES here
      }
    }

    /*
    if (player->actionDirty) {
      actionCollector->add(player->action);

      player->actionDirty = false;
      actionCollector->sequence++;
    }*/

    actionCollector->interval.go();

    actionCollector->time++;
  }

  if (save.justDown()) {
    actionCollector->save();
  }
}
