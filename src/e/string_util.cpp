#include <e/string_util.hpp>

bool StringUtil::isLineEmpty(std::string line) {
  for (int i = 0; i < line.size(); i++) {
    char c = line[i];
    if (!isspace(c)) {
      return false;
    }
  }

  return true;
}

std::string StringUtil::eatCharacters(std::string& line) {
  std::string v;

  int i = 0;

  while (i < line.size()) {
    char c = line[i];

    if (isspace(c)) {
      break;
    }

    v += c;

    i += 1;
  }

  line.erase(0, i);

  return v;
}

std::string StringUtil::eatWhitespace(std::string& line) {
  std::string v;

  for (int i = 0; i < line.size(); i++) {
    char c = line[i];

    if (!isspace(c)) {
      line.erase(0, i);

      break;
    }

    v += c;
  }

  return v;
}

std::string StringUtil::eatLine(std::string& line) {
  std::string v;

  for (int i = 0; i < line.size(); i++) {
    char c = line[i];

    if (c == '\n') {
      line.erase(0, i);
      break;
    }

    v += c;
  }

  return v;
}

std::string StringUtil::eatUntil(std::string& line, char delim) {
  std::string v;

  for (int i = 0; i < line.size(); i++) {
    char c = line[i];

    if (c == delim) {
      line.erase(0, i);
      break;
    }

    v += c;
  }

  return v;
}
