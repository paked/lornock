#pragma once

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>

struct InfoItem {
  InfoItem() {};
  InfoItem(std::string v);

  int asInt();

  std::string val;
};

struct InfoAsset {
  bool load(std::string fname);
  bool load(std::ifstream& file);

  void save(std::string fname);

  std::map<std::string, InfoItem> meta;
  std::vector<std::map<std::string, InfoItem>> body;

  bool isSeparator(std::string line, char separator);
};
