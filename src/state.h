#define STATE_MEMORY_SIZE (megabytes(2))

typedef int8 StateType;

struct State {
  StateType type;

  int8 memory[STATE_MEMORY_SIZE];
};
