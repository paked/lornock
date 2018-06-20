#include <e/info_asset.hpp>

InfoItem::InfoItem(std::string v) : val(v) {};

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

InfoAsset::InfoAsset(std::ifstream& file) {
  std::string line;

  // populate meta info
  while (getline(file, line)) {
    if (isLineEmpty(line)) continue;
    if (line[0] == '#') continue;
    if (isSeparator(line, '=')) break;

    std::string key = eatCharacters(line);
    eatWhitespace(line);
    std::string val = eatLine(line);

    meta[key] = InfoItem(val);
  }

  // build body
  while (true) {
    bool diff = false;
    std::map<std::string, InfoItem> item;

    while (getline(file, line)) {
      if (isLineEmpty(line)) continue;
      if (isSeparator(line, '-')) break;

      std::string key = eatCharacters(line);
      eatWhitespace(line);
      std::string val = eatLine(line);

      item[key] = InfoItem(val);
      diff = true;
    }

    if (!diff) break;

    body.push_back(item);
  }
}

bool InfoAsset::isLineEmpty(std::string line) {
  for (int i = 0; line[i]; i++) {
    char c = line[i];
    if (!isspace(c)) {
      return false;
    }
  }

  return true;
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

std::string InfoAsset::eatCharacters(std::string& line) {
  std::string v;

  for (int i = 0; line[i]; i++) {
    char c = line[i];

    if (isspace(c)) {
      line.erase(0, i);
      break;
    }

    v += c;
  }

  return v;
}

std::string InfoAsset::eatWhitespace(std::string& line) {
  std::string v;

  for (int i = 0; line[i]; i++) {
    char c = line[i];

    if (!isspace(c)) {
      line.erase(0, i);
      break;
    }

    v += c;
  }

  return v;
}

std::string InfoAsset::eatLine(std::string& line) {
  std::string v;

  for (int i = 0; line[i]; i++) {
    char c = line[i];

    if (c == '\n') {
      line.erase(0, i);
      break;
    }

    v += c;
  }

  return v;   
}
