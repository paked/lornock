#include <e/tilemap.hpp>

#include <e/core.hpp>
#include <e/csv.hpp>

#include <config.hpp>

Tilemap::Tilemap(Spritesheet *ts, float x, float y) : tileset(ts), x(x), y(y) {};

int Tilemap::loadLayer(std::string fname, float depth) {
  CSV csv(LVL_PATH + fname);

  return loadLayer(csv.getDataInt(), depth);
}

int Tilemap::loadLayer(Tilelayer::Data d, float depth) {
  Tilelayer* layer = new Tilelayer(tileset, x, y, d, depth);
  layers.push_back(layer);

  return layers.size() - 1;
}

void Tilemap::loadCollisionLayer(std::string fname) {
  canCollide = true;

  CSV csv(LVL_PATH + fname);

  collisionData = csv.getDataInt();
}

void Tilemap::addToGroup(Group<Entity> *g) {
  for (auto layer : layers) {
    g->add(layer);
  }
}
