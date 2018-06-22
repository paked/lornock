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

  actionCollector->open();

  actionCollector->setTime(worldAsset.meta["time"].asInt());
  actionCollector->sequence = worldAsset.meta["sequence"].asInt();
  actionCollector->currentJump = worldAsset.meta["jump"].asInt();

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
    worldAsset.meta["jump"] = std::to_string(actionCollector->currentJump);

    worldAsset.save("assets/data/world.save");
  }

  if (jump.justDown()) {
    jumpTo(15);
  }
}

void Game::postTick() {
  if (actionCollector->interval.done()) {
    for (actionCollector->processedToIndex; actionCollector->processedToIndex < actionCollector->actions.size(); actionCollector->processedToIndex++) {
      Action a = actionCollector->actions[actionCollector->processedToIndex];

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

void Game::jumpTo(int t) {
  actionCollector->add(
      {
      "JUMP",
      {
      { "pos", "(" + std::to_string(player->sprite->x) + "," + std::to_string(player->sprite->y) + ")" },
      { "dt", std::to_string(t) }
      }
      });

  actionCollector->setTime(t);

  actionCollector->currentJump += 1;

  // go through all actions to date, find any SPAWNS which have not been JUMP'd, remember last MOVE for these timelines.
  // instantiate all these SPAWNS as new players with the last move being their seeding action.
  std::map<int, Action> seeds;

  for (int i = 0; i < actionCollector->actions.size(); i++) {
    Action a = actionCollector->actions[i];

    if (a.getTime() > actionCollector->time) {
      break;
    }

    if (a.name == "SPAWN") {
      seeds[a.getJump()] = a;
    } else if (a.name == "JUMP") {
      seeds.erase(a.getJump());
    } else {
      seeds[a.getJump()] = a;
    }
  }

  for (auto& seed : seeds) {
    Action a = seed.second;

    PastPlayer* pp = new PastPlayer(actionCollector, a);
    pastPlayers->add(pp);
  }

  actionCollector->add({
      "SPAWN",
      {
      { "pos", "(" + std::to_string(player->sprite->x) + "," + std::to_string(player->sprite->y) + ")" }
      }});
}
