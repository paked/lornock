#pragma once

#include <vector>
#include <string>

#include <SDL.h>

#include <e/spritesheet.hpp>
#include <e/tilelayer.hpp>
#include <e/group.hpp>

struct Tilemap {
  Tilemap(Spritesheet* ts, float x, float y);

  int loadLayer(std::string fname, float depth);
  int loadLayer(Tilelayer::Data data, float depth);

  void loadCollisionLayer(Tilelayer::Data data);
  void loadCollisionLayer(std::string fname);

  void addToGroup(Group<Entity> *g);

  float x;
  float y;

  Spritesheet* tileset;

  std::vector<Entity*> layerEntities;
  std::vector<Tilelayer*> layers;

  bool canCollide = false;
  Tilelayer::Data collisionData;
};
