#include <e/tilelayer.hpp>

#include <e/core.hpp>
#include <e/math_util.hpp>

// fwd declaration for Scene
// TODO: figure out how to do this fwd declaration better
#include <e/camera.hpp>
#include <e/renderer.hpp>
#include <e/entity.hpp>
#include <e/group.hpp>

Tilelayer::Tilelayer(Spritesheet* ts, float x, float y, Data d, float dp) : x(x), y(y), tileset(ts), data(d) {
  localDepth = dp;
}

void Tilelayer::tick(float dt) {
  int tileSize = tileset->frameWidth;

  Camera* camera = entity->scene->camera;

  int rowMin = 0;
  int rowMax = data[0].size();

  int colMin = 0;
  int colMax = data.size();

  int minX = rowMin;
  int minY = colMin;
  int maxX = rowMax;
  int maxY = colMax;

  if (!hud) {
    minX = camera->realX / tileSize - 1;
    minY = camera->realY / tileSize - 1;

    minX = MathUtil::clamp(minX, rowMin, rowMax);
    minY = MathUtil::clamp(minY, colMin, colMax);

    maxX = (camera->realX + camera->getWidth()) / tileSize + 1;
    maxY = (camera->realY + camera->getHeight()) / tileSize + 1;

    maxX = MathUtil::clamp(maxX, rowMin, rowMax);
    maxY = MathUtil::clamp(maxY, colMin, colMax);
  }

  for (int ty = minY; ty < maxY; ty++) {
    auto row = data[ty];

    for (int tx = minX; tx < maxX; tx++) {
      auto tile = row[tx];

      if (tile < 0) {
        continue;
      }

      Rect dst = {
        (float) x + tx * tileSize,
        (float) y + ty * tileSize,
        (float) tileSize,
        (float) tileSize
      };

      tileset->frame = tile;
      SDL_Rect src = tileset->getSRC();

      unsigned char a = 255 * alpha;

      RenderJob j;
      j.depth = entity->getDepth() + localDepth;
      j.tex = tileset->texture;
      j.src = src;
      j.dst = camera->toView(dst, hud);
      j.alpha = a;

      entity->scene->renderer->queue.push(j);
    }
  }
}

float Tilelayer::getWidth() {
  return data[0].size() * tileset->frameWidth;
}

float Tilelayer::getHeight() {
  return data.size() * tileset->frameWidth;
}
