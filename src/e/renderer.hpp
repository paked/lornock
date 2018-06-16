#pragma once

#include <queue>

#include <SDL.h>

struct RenderJob {
  SDL_Texture* tex;
  SDL_Rect src;
  SDL_Rect dst;
  
  SDL_RendererFlip flip = SDL_FLIP_NONE;
  double angle = 0;
  float depth = 0;

  unsigned char alpha = 255;
};

struct DepthSort
{
  bool operator()(const RenderJob& lhs, const RenderJob& rhs) const
  {
    return lhs.depth > rhs.depth;
  }
};

struct Renderer {
  std::priority_queue<RenderJob, std::vector<RenderJob>, DepthSort> queue;

  void render(SDL_Renderer* renderer);
};
