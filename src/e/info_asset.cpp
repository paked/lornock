#include <e/info_asset.hpp>

#include <e/string_util.hpp>

InfoItem::InfoItem(std::string v) : val(v) {};

int InfoItem::asInt() {
  return stoi(val);
}

/*
Example of how to implement a retriever
glm::vec3 InfoItem::asVec3() {
  std::string line = val;

  glm::vec3 p;

  std::string v = eatCharacters(line);
  p.x = stoi(v);

  eatWhitespace(line);

  v = eatCharacters(line);
  p.y = stoi(v);

  eatWhitespace(line);

  v = eatCharacters(line);
  p.z = stoi(v);

  return p;
}*/

bool InfoAsset::load(std::string fname) {
  std::ifstream file;

  file.open(fname);

  if (!file.is_open()) {
    printf("could not open file!\n");

    return false;
  }

  if (!file.good()) {
    printf("file is not good!\n");

    return false;
  }

  return load(file);
}

bool InfoAsset::load(std::ifstream& file) {
  std::string line;

  // populate meta info
  while (getline(file, line)) {
    if (StringUtil::isLineEmpty(line)) continue;
    if (line[0] == '#') continue;
    if (isSeparator(line, '=')) break;

    std::string key = StringUtil::eatCharacters(line);
    StringUtil::eatWhitespace(line);
    std::string val = StringUtil::eatLine(line);

    meta[key] = InfoItem(val);

  }

  // build body
  while (true) {
    bool diff = false;
    std::map<std::string, InfoItem> item;

    while (getline(file, line)) {
      if (StringUtil::isLineEmpty(line)) continue;
      if (isSeparator(line, '-')) break;

      std::string key = StringUtil::eatCharacters(line);
      StringUtil::eatWhitespace(line);
      std::string val = StringUtil::eatLine(line);

      item[key] = InfoItem(val);
      diff = true;
    }

    if (!diff) break;

    body.push_back(item);
  }

  return true;
}

void InfoAsset::save(std::string fname) {
  std::ofstream file;

  file.open(fname, std::ios::out);
  if (!file.is_open()) {
    printf("Could not open file\n");

    return;
  }

  for (auto& item : meta) {
    file << item.first << " " << item.second.val << "\n";
  }

  file.close();
}

bool InfoAsset::isSeparator(std::string line, char separator) {
  for (int i = 0; i < 2 && i < line.size(); i++) {
    char c = line[i];

    if (c != separator) {
      return false;
    }
  }

  return true;
}
