#include <e/string_util.hpp>

std::string StringUtil::eatCharacters(std::string& line) {
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

std::string StringUtil::eatWhitespace(std::string& line) {
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

std::string StringUtil::eatLine(std::string& line) {
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
