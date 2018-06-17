#pragma once

#include <vector>

struct Item {
  int id;

  int tileIndex;
  int iconIndex;

  static std::vector<Item> items;
};
