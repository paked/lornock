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

  loadCollisionLayer(csv.getDataInt());
}

void Tilemap::loadCollisionLayer(Tilelayer::Data data) {
  canCollide = true;

  collisionData = data;
}

void Tilemap::addToGroup(Group<Entity> *g) {
  for (auto layer : layers) {
    Entity* e = new Entity();
    g->add(e);
    e->reg(layer);

    layerEntities.push_back(e);
  }
}
