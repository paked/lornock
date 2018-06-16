#pragma once

#include <fstream>
#include <vector>
#include <string>

struct CSV
{
  std::string fname;
  char delimeter;

  CSV(std::string f, char delm = ',');

  std::vector<std::vector<std::string> > getData();
  std::vector<std::vector<int> > getDataInt();
};
