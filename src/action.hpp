#pragma once

#include <vector>
#include <string>

#include <e/timer.hpp>
#include <e/point.hpp>

struct Action {
  std::string name;

  std::map<std::string, std::string> params;

  int getInt(std::string n);
  Point getPoint(std::string n);

  int s;
  int t;
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
