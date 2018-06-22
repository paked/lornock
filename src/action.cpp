#include <action.hpp>

#include <e/string_util.hpp>

#include <iostream>
#include <fstream>

std::string Action::toString() {
  std::string res = name;

  for (auto p : params) {
    res += " " + p.first + "=" + p.second;
  }

  return res;
}

int Action::getTime() {
  return getInt("t");
}

int Action::getSequence() {
  return getInt("s");
}

int Action::getInt(std::string key) {
  std::string param = params[key];

  return stoi(param);
}

Point Action::getPoint(std::string key) {
  Point p;

  std::string param = params[key];

  sscanf(param.c_str(), "(%f,%f)", &p.x, &p.y);

  return p;
}

void ActionCollector::open() {
  std::ifstream file;

  file.open("assets/data/simple.timeline");

  if (!file.is_open()) {
    printf("could not open file!\n");

    return;
  }

  if (!file.good()) {
    printf("file is not good!\n");

    return;
  }

  std::string line;

  while (file.good()) {
    getline(file, line);

    if (line.size() < 1) {
      continue;
    }

    std::string name = StringUtil::eatCharacters(line);
    StringUtil::eatWhitespace(line);

    std::map<std::string, std::string> params;

    while (!StringUtil::isLineEmpty(line)) {
      std::string key = StringUtil::eatUntil(line, '=');
      line.erase(0, 1); // kill the = sign
      std::string val = StringUtil::eatCharacters(line);
      StringUtil::eatWhitespace(line);

      params[key] = val;
    }

    Action a = {
      name,
      params,
    };

    actions.push_back(a);
  }
}

void ActionCollector::add(Action a) {
  a.params["t"] = std::to_string(time);
  a.params["s"] = std::to_string(sequence);

  int insertAt = 0;

  // TODO: do lookup backwards
  for (int i = 0; i < actions.size(); i++) {
    Action b = actions[i];

    if (a.getTime() > b.getTime()) {
      insertAt = i + 1;

      continue;
    }

    break;
  }

  actions.insert(actions.begin() + insertAt, a);

  sequence++;
}

void ActionCollector::save() {
  std::ofstream file;

  file.open("assets/data/simple.timeline", std::ios::out);
  if (!file.is_open()) {
    printf("Could not open file\n");

    return;
  }

  for (auto& a : actions) {
    file << a.toString() << "\n";
  }

  file.close();
}

bool ActionCollector::findLastAction(Action& a) {
  for (auto& b : actions) {
    if (b.getSequence() == sequence - 1) {
      a = b;

      return true;
    }
  }

  return false;
}
