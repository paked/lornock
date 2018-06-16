#include <e/csv.hpp>

#include <stdio.h>

#include <iostream>

CSV::CSV(std::string f, char delm) : fname(f), delimeter(delm) {}

std::vector<std::vector<std::string> > CSV::getData() {
  std::vector<std::vector<std::string> > dataList;
  std::ifstream file;

  file.open(fname);

  if (!file.is_open()) {
    printf("could not open file!\n");

    return dataList;
  }

  if (!file.good()) {
    printf("file is not good!\n");

    return dataList;
  }

  std::string line = "";

  while(file.good()) {
    getline(file, line);

    std::vector<std::string> vec;

    std::string current;
    for (auto c : line) {
      if (c == delimeter || c == '\n') {
        vec.push_back(current);
        current = "";
      } else {
        current += c;
      }
    }

    if (current.size() != 0) {
      vec.push_back(current);
    }

    if (vec.size() == 0) {
      continue;
    }

    dataList.push_back(vec);
  }

  file.close();

  return dataList;
}

std::vector<std::vector<int> > CSV::getDataInt() {
  auto data = getData();

  std::vector<std::vector<int>> result;

  for (int y = 0; y < data.size(); y++) {
    std::vector<std::string> row = data[y];

    std::vector<int> vec;
    for (int x = 0; x < row.size(); x++) {
      vec.push_back(stoi(row[x]));
    }

    result.push_back(vec);
  }

  return result;
}
