#pragma once

#include <map>
#include <string>

#include <SDL.h>
#include <SDL_ttf.h>

struct Resources {
  static std::map<std::string, SDL_Texture*> textures;
  static std::map<int,TTF_Font*> fonts;

  static SDL_Texture* get(std::string fname);
  static SDL_Texture* get(std::string fname, int* width, int* height);
  static TTF_Font* get(int fontSize);

  static bool load(std::string fname);
  static bool loadFont(std::string fname, int fontSize);

  static std::string getPath(std::string name);

  static void clean();
};
