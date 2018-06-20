#pragma once

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>

struct InfoItem {
  InfoItem() {};
  InfoItem(std::string v);

  std::string val;
};

struct InfoAsset {
  InfoAsset(std::ifstream& file);

  std::map<std::string, InfoItem> meta;
  std::vector<std::map<std::string, InfoItem>> body;

  private:
  bool isLineEmpty(std::string line);
  bool isSeparator(std::string line, char separator);

  static std::string eatCharacters(std::string& line);
  static std::string eatWhitespace(std::string& line);
  static std::string eatLine(std::string& line);
};
