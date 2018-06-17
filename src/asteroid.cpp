#include <asteroid.hpp>

#include <item.hpp>

void Asteroid::start() {
  Spritesheet* ts = new Spritesheet("tilemap.png", 16, 16);

  map = new Tilemap(ts);

  map->loadCollisionLayer(getCollisionData());

  layerBGIndex = map->loadLayer(getMapData(), DEPTH_BELOW*2);
  layerOreIndex = map->loadLayer(getOreData(), DEPTH_BELOW);

  layerBG = map->layers[layerBGIndex];
  layerOre = map->layers[layerOreIndex];

  map->addToEntity(this);
}

void Asteroid::tick(float dt) {
  Entity::tick(dt);

  layerOre->data = getOreData();
}

Tilelayer::Data Asteroid::getMapData() {
  Tilelayer::Data data;

  int totalSize = size + 2*padding;

  for (int y = 0; y < totalSize; y++) {
    std::vector<int> row;
    for (int x = 0; x < totalSize; x++) {
      int t = 0;

      if ((x < padding || x > totalSize - padding - 1) ||
          (y < padding || y > totalSize - padding - 1)) {
        t = -1;
      }

      row.push_back(t);
    }

    data.push_back(row);
  }

  return data;
}

Tilelayer::Data Asteroid::getOreData() {
  Tilelayer::Data data;

  int totalSize = size + 2*padding;

  for (int y = 0; y < totalSize; y++) {
    std::vector<int> row;
    for (int x = 0; x < totalSize; x++) {
      int t;

      if ((x < padding || x > totalSize - padding - 1) ||
          (y < padding || y > totalSize - padding - 1)) {
        t = -1;
      } else {
        int item = ores[y - padding][x - padding];

        t = Item::items[item].tileIndex;
      }

      row.push_back(t);
    }

    data.push_back(row);
  }

  return data;
}

Tilelayer::Data Asteroid::getCollisionData() {
  Tilelayer::Data data;

  int totalSize = size + 2*padding;

  for (int y = 0; y < totalSize; y++) {
    std::vector<int> row;
    for (int x = 0; x < totalSize; x++) {
      int t = -1;

      if ((x < padding || x > totalSize - padding - 1) ||
          (y < padding || y > totalSize - padding - 1)) {
        t = 1;
      }

      row.push_back(t);
    }

    data.push_back(row);
  }

  return data;
}

Collision::TileHit Asteroid::getPickWithinOreSpace(Sprite* sprite) {
  Collision::TileHit th;

  Collision::TileHit worldTh = Collision::getTileHit(sprite, map, layerOreIndex);

  // Make sure the hit is within "ore space"
  th.hit = (worldTh.x >= padding && worldTh.x <= size) &&
           (worldTh.y >= padding && worldTh.y <= size);

  th.x = worldTh.x - padding;
  th.y = worldTh.y - padding;
  th.t = ores[th.y][th.x];

  return th;
}
