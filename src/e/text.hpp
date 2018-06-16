#pragma once

#include <string>

#include <SDL.h>

#include <e/component.hpp>

struct Text : Component {
  enum AlignmentMode {
    LEFT,
    CENTER
  };

  // TODO: currently fonts are literally just tied to a size. This should not be the case, it's dumb.
  Text(std::string text, int font = 30, float x = 0, float y = 0);
  Text(std::string text, int font, float x, float y, int width);
  ~Text();

  void center();
  void tick(float dt);
  void start();
  void gen();

  std::string text;
  SDL_Color color = White;

  float localDepth = 0;
  float x;
  float y;
  bool hud = false;
  int font;

  float alpha = 1.0f;

  AlignmentMode alignment = LEFT;

  bool wrap = false;
  int width;

  std::string textureText;
  SDL_Texture* texture;
  SDL_Rect rect;

  static SDL_Color White;
  static SDL_Color Black;
};
