#include <e/resources.hpp>

#include <SDL_image.h>

#include <e/core.hpp>

#include <config.hpp>

std::map<std::string, SDL_Texture*> Resources::textures;

std::map<int, TTF_Font*> Resources::fonts;

bool Resources::load(std::string name) {
  if (textures.find(name) != textures.end()) {
    printf("Texture has already been loaded: %s\n", name.c_str());

    return true;
  }

  std::string fname = getPath(name);

  SDL_Surface *image = IMG_Load(fname.c_str());
  if (!image) {
    printf("Texture could not been loaded: %s\n", IMG_GetError());

    return false;
  }

  textures[name] = SDL_CreateTextureFromSurface(Core::renderer, image);

  SDL_FreeSurface(image);

  return true;
}

bool Resources::loadFont(std::string name, int fontSize) {
  std::string path = FONT_PATH + name;
  TTF_Font* font = TTF_OpenFont(path.c_str(), fontSize);
  if (!font) {
    printf("Font could not be loaded: %s\n", TTF_GetError());

    return false;
  }

  fonts[fontSize] = font;

  return true;
}

SDL_Texture* Resources::get(std::string name) {
  if (textures.find(name) == textures.end()) {
    printf("Texture has not been loaded\n");

    return nullptr;
  }

  return textures.find(name)->second;
}

SDL_Texture* Resources::get(std::string name, int* width, int* height) {
  SDL_Texture* tex = get(name);

  if (!tex) {
    return nullptr;
  }

  SDL_QueryTexture(tex, NULL, NULL, width, height);

  return tex;
}

TTF_Font* Resources::get(int fontSize) {
  if (fonts.find(fontSize) == fonts.end()) {
    printf("Font has not been loaded!\n");

    return nullptr;
  }

  return fonts.find(fontSize)->second;
}

std::string Resources::getPath(std::string name) {
  return GFX_PATH + name;
}

void Resources::clean() {
  for (auto pair : textures) {
    SDL_DestroyTexture(pair.second);
  }
}
