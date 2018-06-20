#pragma once

#include <string>

namespace StringUtil {
  bool isLineEmpty(std::string line);

  std::string eatCharacters(std::string& line);
  std::string eatWhitespace(std::string& line);
  std::string eatLine(std::string& line);

  std::string eatUntil(std::string& line, char delim);
}
