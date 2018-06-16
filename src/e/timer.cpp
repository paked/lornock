#include <e/timer.hpp>

#include <SDL.h>

Timer::Timer(int dd) {
  defaultDuration = dd;
}

void Timer::go() {
  go(defaultDuration);
}

void Timer::go(int t) {
  start = now();
  duration = t;

  running = true;
}

void Timer::stop() {
  start = 0;
  duration = 0;

  running = false;
}

bool Timer::done() {
  if (!running) return false;

  if (start + duration < now()) {
    running = false;

    return true;
  }

  return false;
}

int Timer::now() {
  return SDL_GetTicks();
}

float Timer::pc() {
  return 1.0 - ((float) ((start + duration) - now())) / duration;
}
