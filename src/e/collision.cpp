#include <e/collision.hpp>

void Collision::collide(Sprite* first, Sprite* second) {
  if (!second->solid) {
    return;
  }

  collide(first, second->rect());
}

void Collision::collide(Sprite* first, Rect second) {
  if (!first->solid) {
    return;
  }

  Rect b = second;
  Rect res;

  // First case we check against next y position
  {
    Rect a = first->rect();
    a.y += first->nextPositionDelta.y;

    if (intersection(a, b, &res)) {
      int mod = (first->velocity.y < 0) ? 1 : -1;
      first->nextPositionDelta.y += res.h * mod;
    }
  }

  // Then we check against the next x position
  {
    Rect a = first->rect();
    a.x += first->nextPositionDelta.x;

    if (intersection(a, b, &res)) {
      int mod = (first->velocity.x < 0) ? 1 : -1;
      first->nextPositionDelta.x += res.w * mod;
    }
  }
}

void Collision::collide(Sprite *sprite, Tilemap *map) {
  if (!map->canCollide) {
    printf("Tilemap does not have collision data\n");

    return;
  }

  auto data = map->collisionData;
  int tileSize = map->tileset->frameWidth;

  for (int y = 0; y < data.size(); y++) {
    auto row = data[y];

    for (int x = 0; x < row.size(); x++) {
      auto tile = row[x];

      if (tile < 0) {
        continue;
      }
      
      Rect rect = {
        (float) x * tileSize,
        (float) y * tileSize,
        (float) tileSize,
        (float) tileSize
      };

      Collision::collide(sprite, rect);
    }
  }
}

bool Collision::isOverlapping(Sprite* first, Rect second) {
  Rect frect = first->rect();

  return isOverlapping(frect, second);
}

bool Collision::isOverlapping(Sprite* sprite, Tilemap *map, int layer) {
  int tileSize = map->tileset->frameWidth;

  int x = sprite->x/tileSize;
  int y = sprite->y/tileSize;

  Tilelayer* l = map->layers[layer];

  return l->data[y][x] > 0;
}

bool Collision::isOverlapping(Rect first, Rect second) {
  Point amin = { first.x, first.y };
  Point amax = { amin.x + first.w, amin.y + first.h };
  Point bmin = { second.x, second.y };
  Point bmax = { bmin.x + second.w, bmin.y + second.h };

  return bmin.x <= amax.x &&
    amin.x <= bmax.x &&
    bmin.y <= amax.y &&
    amin.y <= bmax.y;
}

bool Collision::isOverlapping(Point p, Rect r) {
  return ((p.x >= r.x) && (p.x <= r.x + r.w)) &&
    ((p.y >= r.y) && (p.y <= r.y + r.h));
}

bool Collision::intersection(Rect first, Rect second, Rect* out) {
  float x1 = second.x; //b.min.x
  float y1 = second.y; //b.min.y
  float x2 = second.x + second.w; //b.max.x
  float y2 = second.y + second.h; //b.max.y

  if (first.x >= x1) {
    x1 = first.x;
  }

  if (first.y >= y1) {
    y1 = first.y;
  }

  if (first.x + first.w <= x2) {
    x2 = first.x + first.w;
  }

  if (first.y + first.h <= y2) {
    y2 = first.y + first.h;
  }

  if (x2 <= x1 || y2 <= y1) {
    // no intersection

    return false;
  }

  out->x = x1;
  out->y = y1;
  out->w = x2 - x1;
  out->h = y2 - y1;

  return true;
}
