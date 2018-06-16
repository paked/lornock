#pragma once

#include <e/sprite.hpp>

typedef std::vector<int> Animation;

struct Spritesheet : Sprite {
  Spritesheet(std::string texName, int frameWidth, int frameHeight, float x=0, float y=0);

  void tick(float dt);
  void tickAnimation();

  SDL_Rect getSRC();

  void playAnimation(std::string name, bool loop = true);
  void addAnimation(std::string name, Animation anim);

  void renderFrame(int frame, SDL_Renderer* renderer, Camera* camera);

  int frameDuration = 1000/20;
  int frameWidth;
  int frameHeight;

  Animation currentAnimation;
  std::string currentAnimationName;
  std::map<std::string, Animation> animations;
  bool playing = false;

  int frame = 0;
  int currentAnimationIndex = 0;
  bool loop = false;
  int nextFrame = 0;
};
