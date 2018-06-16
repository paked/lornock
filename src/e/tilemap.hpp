#pragma once

#include <vector>
#include <string>

#include <SDL.h>

#include <e/spritesheet.hpp>
#include <e/tilelayer.hpp>
#include <e/group.hpp>

struct Tilemap {
  Tilemap(Spritesheet* ts);

  int loadLayer(std::string fname, float depth);
  void loadCollisionLayer(std::string fname);

  void addToGroup(Group<Entity> *g);

  Spritesheet* tileset;
  std::vector<Tilelayer*> layers;

  bool canCollide = false;
  Tilelayer::Data collisionData;
};
