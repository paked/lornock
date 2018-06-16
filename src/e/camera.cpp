#include <e/camera.hpp>

#include <e/sprite.hpp>
#include <e/collision.hpp>
#include <e/math_util.hpp>

float lerp(float t, float a, float b){
  return (1-t)*a + t*b;
}

void Camera::tick(float dt) {
  Point p = getTarget();
  realX = lerp(0.05, realX, p.x);
  realY = lerp(0.05, realY, p.y);

  float shake = shakeTrauma*shakeTrauma;

  if (SDL_GetTicks() > shakeStopTime) {
    shakeTrauma -= 0.2 * dt;

    if (shakeTrauma < 0) {
      shakeTrauma = 0;
    }
  }

  x = realX + shakeMax * shake * MathUtil::randN1P1();
  y = realY + shakeMax * shake * MathUtil::randN1P1();
}

void Camera::shake(int duration, float st) {
  shakeTrauma = st;
  shakeStopTime = SDL_GetTicks() + duration;
}

SDL_Rect Camera::toView(Rect rect, bool global, bool scale) {
  if (!global) {
    rect.x -= x;
    rect.y -= y;
  }

  if (scale) {
    rect.w *= zoom;
    rect.h *= zoom;
  }

  rect.x *= zoom;
  rect.y *= zoom;

  return Rect::toSDL(rect);
}

bool Camera::withinViewport(Rect rect) {
  Rect me = viewport(32);

  return Collision::isOverlapping(me, rect);
}

Point Camera::point() {
  return Point(x, y);
}

Rect Camera::viewport(float buffer) {
  return Rect{
    x - buffer,
    y - buffer,
    logicalWidth + buffer*2,
    logicalHeight + buffer*2
  };
}

float Camera::getWidth() {
  return logicalWidth / zoom;
}

float Camera::getHeight() {
  return logicalHeight / zoom;
}

Point Camera::getTarget() {
  Point p = target;

  if (follow != nullptr) {
    Point c = follow->getCenter();

    p.x = c.x;
    p.y = c.y;
  }

  p.x -= getWidth()/2;
  p.y -= getHeight()/2;

  return p;
}
