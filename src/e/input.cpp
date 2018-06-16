#include <e/input.hpp>

#include <config.hpp>

std::vector<Input*> Input::inputs;

int Input::mouseX = 0;
int Input::mouseY = 0;

bool Input::mouseLastFrameDown = false;
bool Input::mouseThisFrameDown = false;

bool Input::enteringText = false;
std::string Input::text = "";

void Input::handle(SDL_KeyboardEvent event) {
  for (auto input : inputs) {
    if (Input::enteringText && !input->textEditFriendly) {
      continue;
    }

    if (event.keysym.scancode == input->key) {
      input->thisFrameDown = event.state == SDL_PRESSED;
    }
  }

  // Rest of this function does text related things
  if (!enteringText) {
    return;
  }

  // Delete from string with backspace
  if (event.state == SDL_PRESSED &&
      event.keysym.scancode == SDL_SCANCODE_BACKSPACE &&
      text.begin() != text.end()) {
    text.erase(text.end() - 1);
  }
}

void Input::handle(SDL_MouseMotionEvent event) {
  mouseX = event.x;
  mouseY = event.y;
}

void Input::handle(SDL_MouseButtonEvent event) {
  if (event.button != SDL_BUTTON_LEFT) {
    // TODO: properly handle mouse buttons

    return;
  }

  mouseThisFrameDown = event.state == SDL_PRESSED;
}

void Input::handle(SDL_TextEditingEvent edit) {
  // TODO: this event does not get fired, and I'm not really sure why.
  // text = std::string(edit.text);
}

void Input::handle(SDL_TextInputEvent input) {
  text += input.text;
}

void Input::push() {
  mouseLastFrameDown = mouseThisFrameDown;

  for (auto input : inputs) {
    input->lastFrameDown = input->thisFrameDown;
  }
}

void Input::startTextInput() {
  SDL_StartTextInput();

  text = "";
  enteringText = true;
}

void Input::stopTextInput() {
  SDL_StopTextInput();

  enteringText = false;
}

bool Input::mouseJustDown() {
  return !mouseLastFrameDown && mouseThisFrameDown;
}

bool Input::mouseJustUp() {
  return mouseLastFrameDown && !mouseThisFrameDown;
}

bool Input::mouseDown() {
  return mouseThisFrameDown;
}

bool Input::mouseUp() {
  return !mouseThisFrameDown;
}

Input::Input(SDL_Scancode k, bool textEditFriendly) : key(k), textEditFriendly(textEditFriendly) {
  inputs.push_back(this);
}

bool Input::down() {
  return thisFrameDown;
}

bool Input::up() {
  return !thisFrameDown;
}

bool Input::justDown() {
  return !lastFrameDown && thisFrameDown;
}

bool Input::justUp() {
  return lastFrameDown && !thisFrameDown;
}
