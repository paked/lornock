#include <action.hpp>

#include <e/string_util.hpp>

#include <iostream>
#include <fstream>

int Action::getInt(std::string n) {
  int v = 0;

  std::string prms = " " + params;
  std::size_t i = prms.find(" " + n + "=");

  if (i == std::string::npos) {
    printf("That doesn't exist\n");

    return i;
  }

  prms.erase(0, i);

  std::size_t equals = prms.find_first_of('=');
  std::size_t space = prms.find_first_of(' ');

  if (equals == std::string::npos ||
      space == std::string::npos) {
    printf("can't find int\n");

    return i;
  }

  std::string s = prms.substr(equals + 1, space - (equals + 1));

  printf("n: [%s] is: [%s] params: [%s]\n", n.c_str(), s.c_str(), params.c_str());

  v = stoi(s);

  return v;
}

Point Action::getPoint(std::string n) {
  Point p;

  std::string prms = params;

  // eg params. = `pos=(35.000000,35.000000) vel=(0.000000,0.000000) acc=(8.000000,0.000000) t=12 s=12`
  std::size_t i = prms.find(n + "=");
  if (i == std::string::npos) {
    printf("That doesn't exist\n");

    return p;
  }

  prms.erase(0, i - 1);

  std::size_t obracket = prms.find_first_of('(');
  std::size_t comma = prms.find_first_of(',');
  std::size_t cbracket = prms.find_first_of(')');

  if (obracket == std::string::npos ||
      comma == std::string::npos ||
      cbracket == std::string::npos) {
    printf ("could not find open bracket, comma, or closing bracket\n");

    return p;
  }

  std::string f1 = prms.substr(obracket + 1, comma - (obracket + 1));
  std::string f2 = prms.substr(comma + 1, cbracket - (comma + 1));

  // printf("v1: [%s], v2: [%s]\n", f1.c_str(), f2.c_str());

  p.x = stof(f1);
  p.y = stof(f2);

  return p;
}

void ActionCollector::open() {
  std::ifstream file;

  file.open("assets/data/example.timeline");

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
    std::string params = StringUtil::eatLine(line);

    Action a = {
      name,
      params,
    };

    a.s = a.getInt("s");
    a.t = a.getInt("t");

    actions.push_back(a);

    printf("%s: %s\n", name.c_str(), params.c_str());
  }
}

void ActionCollector::add(Action a) {
  a.t = time;
  a.s = sequence;

  // TODO: if action happens before HEAD then insert it in the correct order
  actions.push_back(a);
}

void ActionCollector::save() {
  std::ofstream file;

  file.open("out.data", std::ios::out);
  if (!file.is_open()) {
    printf("Could not open file\n");

    return;
  }

  for (auto& a : actions) {
    file << a.name << " " << a.params << " t=" << a.t << " s=" << a.s << "\n";
  }

  file.close();
}
