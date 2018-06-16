#include <e/tilelayer.hpp>

#include <e/core.hpp>
#include <e/math_util.hpp>

// fwd declaration for Scene
// TODO: figure out how to do this fwd declaration better
#include <e/camera.hpp>
#include <e/renderer.hpp>
#include <e/entity.hpp>
#include <e/group.hpp>

Tilelayer::Tilelayer(Spritesheet* ts, Data d, float dp) : tileset(ts), data(d) {
  localDepth = dp;
}

void Tilelayer::tick(float dt) {
  int tileSize = tileset->frameWidth;

  int rowMin = 0;
  int rowMax = data[0].size();

  int colMin = 0;
  int colMax = data.size();

  int minX = scene->camera->realX / tileSize - 1;
  int minY = scene->camera->realY / tileSize - 1;

  minX = MathUtil::clamp(minX, rowMin, rowMax);
  minY = MathUtil::clamp(minY, colMin, colMax);

  int maxX = (scene->camera->realX + scene->camera->getWidth()) / tileSize + 1;
  int maxY = (scene->camera->realY + scene->camera->getHeight()) / tileSize + 1;

  maxX = MathUtil::clamp(maxX, rowMin, rowMax);
  maxY = MathUtil::clamp(maxY, colMin, colMax);

  for (int y = minY; y < maxY; y++) {
    auto row = data[y];

    for (int x = minX; x < maxX; x++) {
      auto tile = row[x];

      if (tile < 0) {
        continue;
      }

      Rect dst = {
        (float) x * tileSize,
        (float) y * tileSize,
        (float) tileSize,
        (float) tileSize
      };

      /*
      if (!scene->camera->withinViewport(dst)) {
        // don't need to render if the thing isn't on screen

        continue;
      }*/

      tileset->frame = tile;
      SDL_Rect src = tileset->getSRC();

      unsigned char a = 255 * alpha;

      RenderJob j;
      j.depth = getDepth();
      j.tex = tileset->texture;
      j.src = src;
      j.dst = scene->camera->toView(dst);
      j.alpha = a;

      scene->renderer->queue.push(j);
    }
  }
}
