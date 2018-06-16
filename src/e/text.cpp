#include <e/text.hpp>

#include <SDL_ttf.h>

#include <e/resources.hpp>
#include <e/core.hpp>

// fwd declaration for Scene
// TODO: figure out how to do this fwd declaration better
#include <e/camera.hpp>
#include <e/renderer.hpp>
#include <e/entity.hpp>
#include <e/group.hpp>

Text::Text(std::string text, int font, float x, float y) : text(text), font(font), x(x), y(y) {
  gen();
}

Text::Text(std::string text, int font, float x, float y, int width) : text(text), font(font), x(x), y(y), width(width) {
  wrap = true;
}

void Text::start() {
  gen();
}

void Text::center() {
  x -= rect.w/2;
  y -= rect.h/2;
}

void Text::tick(float dt) {
  Component::tick(dt);

  // Regen the text if there has been a change to it
  if (textureText != text) {
    gen();
  }

  float zoom = entity->scene->camera->zoom;

  float alignedX = x;
  float alignedY = y;

  switch (alignment) {
    case LEFT:
      alignedX = x;
      alignedY = y;

      break;
    case CENTER:
      // We don't scale text with screen size in order to preserve it's aesthetic. We compensate for that here
      // TODO: we really need to create a divison between local and world in this text class. Maybe something like srcRect and rect. The latter being the rect in world space.
      alignedX = x - (rect.w/zoom)/2;
      alignedY = y - (rect.h/zoom)/2;

      break;
    default:
      printf("Could not render text, invalid alignment!");

      return;
  }

  Rect dst = {
    alignedX,
    alignedY,
    (float) rect.w,
    (float) rect.h
  };

  RenderJob j;
  j.depth = entity->getDepth() + localDepth;
  j.tex = texture;
  j.src = rect;
  j.dst = entity->scene->camera->toView(dst, hud, false);
  j.alpha = (unsigned char) (alpha * 255);

  entity->scene->renderer->queue.push(j);
}

void Text::gen() {
  textureText = text;

  TTF_Font* f = Resources::get(font);

  const char* t = text.c_str();

  SDL_Surface* surface = nullptr;
  if (wrap) {
    surface = TTF_RenderText_Blended_Wrapped(f, t, color, width * entity->scene->camera->zoom);
  } else {
    surface = TTF_RenderText_Solid(f, t, color);
  }

  texture = SDL_CreateTextureFromSurface(Core::renderer, surface);

  rect.x = rect.y = 0;
  SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);

  SDL_FreeSurface(surface);
}

Text::~Text() {
  SDL_DestroyTexture(texture);
}

SDL_Color Text::White = {255, 255, 255};
SDL_Color Text::Black = {0, 0, 0};
