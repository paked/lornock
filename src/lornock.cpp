#include <platform_common.hpp>

struct LornockData {
  int number;
};

void updateLornock(LornockMemory* m) {
  LornockData* lornockData = (LornockData*) m->permanentStorage;
  if (!m->initialized) {
    lornockData->number = 42;

    m->initialized = true;
  }
}

extern "C" {
  void update(LornockMemory* m) {
    updateLornock(m);

    logln("Updating... something else...");
  }
}
