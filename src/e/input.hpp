#pragma once

#include <vector>
#include <string>

#include <SDL.h>

struct Input {
  static std::vector<Input*> inputs;

  static void handle(SDL_MouseButtonEvent event);
  static void handle(SDL_MouseMotionEvent event);
  static void handle(SDL_KeyboardEvent event);
  static void handle(SDL_TextEditingEvent event);
  static void handle(SDL_TextInputEvent event);

  static void push();

  static bool mouseJustDown();
  static bool mouseJustUp();
  static bool mouseDown();
  static bool mouseUp();

  static void startTextInput();
  static void stopTextInput();

  static int mouseX;
  static int mouseY;
  static bool mouseLastFrameDown;
  static bool mouseThisFrameDown;

  static bool enteringText;
  static std::string text;

  Input(SDL_Scancode k, bool textEditFriendly = false);

  bool down();
  bool up();
  bool justDown();
  bool justUp();

  SDL_Scancode key;

  bool lastFrameDown = false;
  bool thisFrameDown = false;

  bool textEditFriendly = false;
};
