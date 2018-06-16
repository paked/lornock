#include <e/spritesheet.hpp>

#include <e/core.hpp>

Spritesheet::Spritesheet(std::string texName, int fw, int fh, float x, float y): Sprite(texName, x, y) {
  frameWidth = fw;
  frameHeight = fh;

  width = frameWidth;
  height = frameHeight;
}

void Spritesheet::tick(float dt) {
  if (playing) {
    frame = currentAnimation[currentAnimationIndex];

    tickAnimation();
  }

  // Send job
  Sprite::tick(dt);
}

void Spritesheet::addAnimation(std::string name, Animation anim) {
  animations[name] = anim;
}

void Spritesheet::playAnimation(std::string name, bool l) {
  loop = l;

  playing = true;
  currentAnimation = animations[name];
  currentAnimationName = name;
  currentAnimationIndex = 0;
  nextFrame = SDL_GetTicks() + frameDuration;

  frame = currentAnimation[currentAnimationIndex];
}

void Spritesheet::tickAnimation() {
  if (SDL_GetTicks() < nextFrame) {
    return;
  }

  nextFrame = SDL_GetTicks() + frameDuration;

  // not at end of animation
  if (currentAnimationIndex < currentAnimation.size() - 1) {
    currentAnimationIndex++;

    return;
  }

  if (loop) {
    currentAnimationIndex = 0;

    return;
  }

  playing = false;
}

SDL_Rect Spritesheet::getSRC() {
  int rowSize = textureWidth/frameWidth;

  int yIndex = frame / rowSize;
  int xIndex = frame % rowSize;

  return SDL_Rect {
    xIndex * frameWidth,
    yIndex * frameHeight,
    frameWidth,
    frameHeight
  };
}

void Spritesheet::renderFrame(int frame, SDL_Renderer* renderer, Camera* camera) {
  int f = this->frame;

  this->frame = frame;

  render(renderer, camera);

  this->frame = f;
}
