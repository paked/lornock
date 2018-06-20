#pragma once

#include <vector>
#include <string>
#include <map>

#include <e/timer.hpp>
#include <e/point.hpp>

struct Action {
  std::string toString();

  int getInt(std::string key);
  Point getPoint(std::string key);

  int getSequence();
  int getTime();

  std::string name;
  std::map<std::string, std::string> params;
};

struct ActionCollector {
  void open();
  void add(Action a);
  void save();

  std::vector<Action> actions;

  int sequence = 0;
  int time = 0;

  const int ticks = 20;
  Timer interval = Timer(1000/ticks);
};
