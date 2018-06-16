#include <e/entity.hpp>

#include <e/component.hpp>

void Entity::start() {}

void Entity::tick(float dt) {
  for (auto& comp : components) {
    if (!comp->active) {
      continue;
    }

    comp->tick(dt);
  }
}

void Entity::postTick() {}

float Entity::getDepth() {
  float depth = localDepth;

  if (parent != nullptr) {
    depth += parent->getDepth();
  }

  return depth; 
}

void Entity::reg(Component* comp) {
  components.push_back(comp);

  comp->entity = this;

  comp->start();
}
