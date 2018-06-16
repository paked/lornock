#include <e/scene.hpp>

#include <e/renderer.hpp>
#include <e/camera.hpp>
#include <e/group.hpp>

#include <e/core.hpp>
#include <e/input.hpp>

Scene::Scene() {
  renderer = new Renderer();
  camera = new Camera();
  entities = new Group<Entity>();

  entities->scene = this;
}

bool Scene::load() {
  return true;
}

void Scene::start() {}

void Scene::update(float dt) {
  SDL_Event event;

  Input::push();
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        shouldQuit = true;

        break;
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        Input::handle(event.key);

        break;
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        Input::handle(event.button);

        break;
      case SDL_MOUSEMOTION:
        Input::handle(event.motion);

        break;
      case SDL_TEXTINPUT:
        Input::handle(event.text);

        break;
      case SDL_TEXTEDITING:
        Input::handle(event.edit);

        break;
    }
  }

  entities->tick(dt);

  camera->tick(dt);
}

void Scene::render(SDL_Renderer *r) {
  renderer->render(r);
}
