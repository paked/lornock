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

#include <config.hpp>

Game::Game() : Scene() {}

bool Game::load() {
  bool ok = Scene::load();

  ok |= Resources::load("tilemap.png");
  ok |= Resources::load("test.png");
  ok |= Resources::load("toolbar.png");
  ok |= Resources::load("toolbar_items.png");

  ok |= worldAsset.load("assets/data/world.save");

  return ok;
}

void Game::start() {
  Scene::start();

  // Action mgmt
  actionCollector = new ActionCollector();

  actionCollector->time = worldAsset.meta["time"].asInt();
  actionCollector->sequence = worldAsset.meta["sequence"].asInt();

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

  Action a;
  if (actionCollector->findLastAction(a)) {
    Point p = a.getPoint("pos");
    player->sprite->x = p.x;
    player->sprite->y = p.y;

    printf("found last action\n");
  } else {
    printf("fuck I cannot find the last action\n");
  }

    /*
  actionCollector->add({
    "SPAWN",
    {
      { "P", "0" },
      { "pos", "(35.0,35.0)" },
    }
  });*/

  pastPlayers = new Group<PastPlayer>();
  entities->add(pastPlayers);

  // Camera
  camera->target = mapCenter;

  for (acUpTo; acUpTo < actionCollector->actions.size(); acUpTo++) {
    Action a = actionCollector->actions[acUpTo];

    if (a.getTime() > actionCollector->time) {
      break;
    }
  }
}

void Game::collisions() {
  Collision::collide(player->sprite, asteroid->map);

  for (auto& m : pastPlayers->members) {
    Collision::collide(m->sprite, asteroid->map);
  }
}

void Game::tick(float dt) {
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

  if (save.justDown()) {
    actionCollector->save();

    worldAsset.meta["time"] = std::to_string(actionCollector->time);
    worldAsset.meta["sequence"] = std::to_string(actionCollector->sequence);

    worldAsset.save("assets/data/world.save");
  }

  if (jump.justDown()) {
    actionCollector->add(
        {
          "JUMP",
          {
            { "pos", "(" + std::to_string(player->sprite->x) + "," + std::to_string(player->sprite->y) + ")" },
            { "dt", "0" }
          }
        });

    actionCollector->time = 0;

    actionCollector->add({
        "SPAWN",
        {
          { "pos", "(" + std::to_string(player->sprite->x) + "," + std::to_string(player->sprite->y) + ")" }
        }});

    acUpTo = 0;
  }
}

void Game::postTick() {
  if (actionCollector->interval.done()) {
    for (acUpTo; acUpTo < actionCollector->actions.size(); acUpTo++) {
      Action a = actionCollector->actions[acUpTo];

      if (a.getTime() > actionCollector->time) {
        break;
      }

      if (a.name == "SPAWN" && a.getSequence() != actionCollector->sequence - 1) {
        PastPlayer* pp = new PastPlayer(actionCollector, a);
        pastPlayers->add(pp);
      } else if (a.name == "MOVE") {
        // We don't handle MOVES here
      }
    }

    if (player->actionDirty) {
      actionCollector->add(player->action);

      player->actionDirty = false;
    }

    actionCollector->interval.go();

    actionCollector->time++;
  }
}
