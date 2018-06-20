#pragma once

#include <string>

namespace StringUtil {
  std::string eatCharacters(std::string& line);
  std::string eatWhitespace(std::string& line);
  std::string eatLine(std::string& line);
}
