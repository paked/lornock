#pragma once

#include <vector>
#include <string>

#include <SDL.h>

#include <e/spritesheet.hpp>
#include <e/tilelayer.hpp>
#include <e/group.hpp>

struct Tilemap {
  Tilemap(Spritesheet* ts, float x = 0, float y = 0);

  int loadLayer(std::string fname, float depth);
  int loadLayer(Tilelayer::Data data, float depth);

  void loadCollisionLayer(Tilelayer::Data data);
  void loadCollisionLayer(std::string fname);

  // FIXME: these two methods will behave super weirdly if used at the same time.
  void addToGroup(Group<Entity> *g);
  void addToEntity(Entity *e);

  float x;
  float y;

  Spritesheet* tileset;

  std::vector<Entity*> layerEntities;
  std::vector<Tilelayer*> layers;

  bool canCollide = false;
  Tilelayer::Data collisionData;
};
