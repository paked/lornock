enum {
  STATE_null,
#define stateType(name) STATE_ ## name,
#include <state_types.cpp>
#undef stateType
};

// Include states here
#include <game_state.cpp>

void stateInit(State *state, StateType type) {
  switch (type) {
#define stateType(name) case STATE_ ## name: { name ## StateInit(state); break; }
#include <state_types.cpp>
#undef stateType
    default: break;
  }

  state->type = type;
}

// TODO(harrison): handle state clean

void stateUpdate(State *state) {
  // TODO(harrison): handle state changing
  switch (state->type) {
#define stateType(name) case STATE_ ## name: { name ## StateUpdate(state); } break;
#include <state_types.cpp>
#undef stateType
    default: break;
  }
}
