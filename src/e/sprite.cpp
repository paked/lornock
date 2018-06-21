#include <e/sprite.hpp>

#include <e/core.hpp>
#include <e/resources.hpp>

// fwd declaration for Scene
// TODO: figure out how to do this fwd declaration better
#include <e/camera.hpp>
#include <e/renderer.hpp>
#include <e/entity.hpp>
#include <e/group.hpp>

Sprite::Sprite(std::string texName, float x, float y) : x(x), y(y) {
  texture = Resources::get(texName, &textureWidth, &textureHeight);

  width = textureWidth;
  height = textureHeight;
}

SDL_Rect Sprite::getSRC() {
  return SDL_Rect {
    0,
    0,
    textureWidth,
    textureHeight
  };
}

Rect Sprite::rect() {
  return Rect{
      x,
      y,
      width,
      height
  };
}

Point Sprite::getCenter() {
  return { x + width/2, y + height/2 };
}

void Sprite::center() {
  x -= width/2;
  y -= height/2;
}

void Sprite::tick(float dt) {
  if (entity == nullptr) {
    printf("WARNING: calling tick on an entityless component!\n");
  }

  velocity.x += (acceleration.x) * dt;
  if (velocity.x > maxVelocity.x) {
    velocity.x = maxVelocity.x;
  } else if (velocity.x < -maxVelocity.x) {
    velocity.x = -maxVelocity.x;
  }

  if (fabs(acceleration.x) < 0.1) {
    velocity.x *= drag.x;
  }

  velocity.y += (acceleration.y) * dt;
  if (velocity.y > maxVelocity.y) {
    velocity.y = maxVelocity.y;
  } else if (velocity.y < -maxVelocity.y) {
    velocity.y = -maxVelocity.y;
  }

  if (fabs(acceleration.y) == 0) {
    velocity.y *= drag.y;
  }

  nextPositionDelta.x = velocity.x * dt;
  nextPositionDelta.y = velocity.y * dt;
}

void Sprite::postTick() {
  x += nextPositionDelta.x;
  y += nextPositionDelta.y;
}

void Sprite::job(Scene* scene, float depth) {
  SDL_Rect dst = scene->camera->toView(rect(), hud);
  SDL_Rect src = getSRC();

  SDL_RendererFlip fl = SDL_FLIP_NONE;

  if (flip) {
    fl = SDL_FLIP_HORIZONTAL;
  }

  RenderJob j;
  j.depth = depth;
  j.tex = texture;
  j.src = src;
  j.dst = dst;
  j.angle = angle;
  j.flip = fl;
  j.alpha = alpha;

  scene->renderer->queue.push(j);
}

void Sprite::render() {
  if (!visible) {
    return;
  }

  job(entity->scene, entity->getDepth() + localDepth);
}
