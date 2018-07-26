#define ACTION_COMMON_FIELDS \
  uint32 type; \
  int64 time; \
  uint64 sequence; \
  uint64 jumpID;

enum ActionType {
  NONE,
  MOVE,
  JUMP,
  SPAWN
};

struct CommonAction {
  ACTION_COMMON_FIELDS;
};

struct MoveAction {
  ACTION_COMMON_FIELDS;

  vec3 pos;
};

struct JumpAction {
  ACTION_COMMON_FIELDS;

  int64 destination;
};

struct SpawnAction {
  ACTION_COMMON_FIELDS;

  vec3 pos;
};

struct Action {
  uint32 type;

  union {
    CommonAction common;
    SpawnAction spawn;
    MoveAction move;
    JumpAction jump;
  };
};

Action action_makeMove(vec3 pos) {
  Action a;

  a.type = MOVE;
  a.move.pos = pos;

  return a;
}

Action action_makeSpawn(vec3 pos) {
  Action a;

  a.type = SPAWN;
  a.spawn.pos = pos;

  return a;
}

Action action_makeJump(int64 destination) {
  Action a;

  a.type = JUMP;
  a.jump.destination = destination;

  return a;
}

void action_print(Action a) {
  switch (a.type) {
    case MOVE:
      {
        logfln("MOVE t=%ld s=%lu j=%lu pos=(%f,%f,%f)", a.common.time, a.common.sequence, a.common.jumpID, a.move.pos.x, a.move.pos.y, a.move.pos.z);
      } break;
    case JUMP:
      {
        logfln("JUMP t=%ld s=%lu j=%lu dt=%ld", a.common.time, a.common.sequence, a.common.jumpID, a.jump.destination);
      } break;
    case SPAWN:
      {
        logfln("SPAWN t=%ld s=%lu j=%lu pos=(%f,%f,%f)", a.common.time, a.common.sequence, a.common.jumpID, a.spawn.pos.x, a.spawn.pos.y, a.spawn.pos.z);
      } break;
    default:
      {
        // well shit
      } break;
  }
}

void action_serialize(Action a, char* out) {
  char line[256] = {0};

  switch (a.type) {
    case MOVE:
      {
        snprintf(line, 256, "MOVE t=%ld s=%lu j=%lu pos=(%f,%f,%f)\n", a.common.time, a.common.sequence, a.common.jumpID, a.move.pos.x, a.move.pos.y, a.move.pos.z);
      } break;
    case JUMP:
      {
        snprintf(line, 256, "JUMP t=%ld s=%lu j=%lu dt=%ld\n", a.common.time, a.common.sequence, a.common.jumpID, a.jump.destination);
      } break;
    case SPAWN:
      {
        snprintf(line, 256, "SPAWN t=%ld s=%lu j=%lu pos=(%f,%f,%f)\n", a.common.time, a.common.sequence, a.common.jumpID, a.spawn.pos.x, a.spawn.pos.y, a.spawn.pos.z);
      } break;
    default:
      {
        logln("ERROR: cannot serialize unknown action type");

        assert(false);
        return;
      } break;
  }

  strcat(out, line);
}

bool action_parse(Action* action, char* line, uint32 lineLen) {
  char actionName[64];

  char paramName[64];
  char paramValue[64];
  int64* actionTime = 0;
  uint64* actionSequence = 0;
  uint64* actionJump = 0;

  Action a;

  uint32 lineI = 0;

  for (; lineI < lineLen; lineI++) {
    actionName[lineI] = '\0';

    char c = line[lineI];
    if (isspace(c)) {
      lineI++;

      break;
    }

    actionName[lineI] = line[lineI];
  }

  if (strcmp("MOVE", actionName) == 0) {
    a.type = a.move.type = MOVE;

    actionTime = &a.common.time;
    actionSequence = &a.common.sequence;
    actionJump = &a.common.jumpID;
  } else if (strcmp("JUMP", actionName) == 0) {
    a.type = JUMP;
    a.type = a.jump.type = JUMP;

    actionTime = &a.common.time;
    actionSequence = &a.common.sequence;
    actionJump = &a.common.jumpID;
  } else if (strcmp("SPAWN", actionName) == 0) {
    a.type = a.spawn.type = SPAWN;

    actionTime = &a.common.time;
    actionSequence = &a.common.sequence;
    actionJump = &a.common.jumpID;
  } else {
    logfln("WARNING: unknown action '%s'", actionName);

    return false;
  }

  for (; lineI < lineLen; lineI++) {
    eatUntilChar(paramName, 64 , '=', line, lineLen, &lineI);
    lineI++; // skip '=' sign
    eatUntilChar(paramValue, 64 , ' ', line, lineLen, &lineI);

    // TODO(harrison): implement error checking. We're only processing two data types at the moment, so we'll do this in two different ways:
    // 1. Checking whether end is set, if it is that means something went wrong with the stroull conversion
    // 2. Checking the return value of sscanf (provides error code if it can't complete the thing)

    char* end;

    if (strcmp("t", paramName) == 0) {
      *actionTime = strtoll(paramValue, &end, 10);

      continue;
    } else if (strcmp("s", paramName) == 0) {
      *actionSequence = strtoull(paramValue, &end, 10);

      continue;
    } else if (strcmp("j", paramName) == 0) {
      *actionJump = strtoull(paramValue, &end, 10);

      continue;
    }

    switch (a.type) {
      case MOVE:
        {
          if (strcmp("pos", paramName) == 0) {
            sscanf(paramValue, "(%f,%f,%f)", &a.move.pos.x, &a.move.pos.y, &a.move.pos.z);

            continue;
          }
        } break;
      case JUMP:
        {
          if (strcmp("dt", paramName) == 0) {
            a.jump.destination = strtoll(paramValue, &end, 10);

            continue;
          }

        } break;
      case SPAWN:
        {
          if (strcmp("pos", paramName) == 0) {
            sscanf(paramValue, "(%f,%f,%f)", &a.spawn.pos.x, &a.spawn.pos.y, &a.spawn.pos.z);

            continue;
          }
        } break;
      default:
        {
          logfln("WARNING: Unknown action parameter '%s' with value '%s' for action '%s'", paramName, paramValue, actionName);
          break;
        }
    }
  }

  *action = a;

  return true;
}

#define ACTION_CHUNK_MAX 128

struct ActionChunk {
  Action actions[ACTION_CHUNK_MAX];
  uint64 count;
};

#define TIME_BOX_WRITE_BUFFER_SIZE (128)
#define TIME_BOX_PAGE_SIZE (128)
#define TIME_BOX_TICKS_PER_SECOND (10)
#define TIME_BOX_TICK_MS_INTERVAL (1000/TIME_BOX_TICKS_PER_SECOND)

struct TimeIndex {
  int64 time;
  uint64 sequence;
  uint64 jumpID;

  int64 timeDoneTo;
  uint64 point;
};

void timeIndex_init(TimeIndex* index) {
  index->time = 0;
  index->sequence = index->jumpID = 0;

  index->timeDoneTo = -1;
  index->point = 0;
}

struct TimeBox {
  ActionChunk toWrite;
};

bool timeBox_nextAction(TimeBox* tb, MemoryArena* ma, TimeIndex* index, Action *a);

void timeBox_init(TimeBox* tb) {
  tb->toWrite.count = 0;
}

void timeBox_load(TimeBox* tb, TimeIndex* index, MemoryArena* ma, const char* name) {
  char saveFilename[128];
  snprintf(saveFilename, 128, "data/saves/%s.timeline", name);

  void* rawData;
  uint32 rawLen = 0;

  loadFromFile(saveFilename, &rawData, &rawLen);

  assert(rawLen > 0);

  uint32 rawUpTo = 0;
  char *raw = (char*) rawData;

  memoryArena_clear(ma);

  ActionChunk* chunk = memoryArena_pushStruct(ma, ActionChunk);
  chunk->count = 0;

  // TODO(harrison): ensure that new memory gets zeroed out.
  // TODO(harrison): test this solution handling multi-chunk things

  char* line = (char*) lornockMemory->transientStorage;
  uint32 lineLen = 0;
  while (rawUpTo < rawLen) {
    getLine(line, &lineLen, &rawUpTo, raw, rawLen);
    rawUpTo += 1;

    Action a;

    assert(action_parse(&a, line, lineLen));

    chunk->actions[chunk->count] = a;
    chunk->count += 1;

    if (chunk->count >= ACTION_CHUNK_MAX) {
      chunk = memoryArena_pushStruct(ma, ActionChunk);
      chunk->count = 0;
    }
  }
}

void timeBox_save(TimeBox* tb, TimeIndex worldIndex, MemoryArena* ma) {
  TimeIndex index;
  timeIndex_init(&index);

  char* out = (char*) lornockMemory->transientStorage;
  out[0] = '\0';

  uint32 i = 0;

  while (i < tb->toWrite.count) {
    Action toWrite = tb->toWrite.actions[i];

    index.time = toWrite.common.time;

    {
      Action a;
      while (timeBox_nextAction(tb, ma, &index, &a)) {
        action_serialize(a, out);
      }
    }

    action_serialize(toWrite, out);

    i += 1;
  }

  index.time = INT64_MAX;

  Action a;
  while (timeBox_nextAction(tb, ma, &index, &a)) {
    action_serialize(a, out);
  }

  tb->toWrite.count = 0;

  writeToFile("data/saves/simple.timeline", (void*) out, strlen(out));
}

void timeBox_add(TimeBox* tb, TimeIndex* index, MemoryArena* ma, Action a) {
  if (tb->toWrite.count >= ACTION_CHUNK_MAX) {
    logln("flushing buffer!");

    timeBox_save(tb, *index, ma);

    tb->toWrite.count = 0;
  }

  index->sequence += 1;
  a.common.time = index->time;
  a.common.sequence = index->sequence;
  a.common.jumpID = index->jumpID;

  tb->toWrite.actions[tb->toWrite.count] = a;

  tb->toWrite.count += 1;
}

bool timeBox_nextActionInSequence(TimeBox* tb, MemoryArena* ma, uint64 sequence, Action *a) {
  for (MemoryBlock* block = lornockData->actionsArena.first; block != 0; block = block->next) {
    ActionChunk* ac = (ActionChunk*) block->start;

    for (uint64 i = 0; i < ac->count; i++) {
      Action t = ac->actions[i];

      if (t.common.sequence == sequence + 1) {
        *a = t;

        return true;
      }
    }
  }

  return false;
}

bool timeBox_actionInSequence(TimeBox* tb, MemoryArena* ma, uint64 sequence, Action *a) {
  for (MemoryBlock* block = lornockData->actionsArena.first; block != 0; block = block->next) {
    ActionChunk* ac = (ActionChunk*) block->start;

    for (uint64 i = 0; i < ac->count; i++) {
      Action t = ac->actions[i];

      if (t.common.sequence == sequence) {
        *a = t;

        return true;
      }
    }
  }

  return false;
}


bool timeBox_nextAction(TimeBox* tb, MemoryArena* ma, TimeIndex* index, Action *a) {
  uint64 p = 0;
  int64 lastTime = -1;

  for (MemoryBlock* block = lornockData->actionsArena.first; block != 0; block = block->next) {
    ActionChunk* ac = (ActionChunk*) block->start;

    for (uint64 i = 0; i < ac->count; i++) {
      Action t = ac->actions[i];

      if (lastTime == t.common.time) {
        p += 1;
      } else {
        p = 0;
      }

      lastTime = t.common.time;

      if (t.common.time < index->timeDoneTo) {
        continue;
      }

      if (t.common.time == index->timeDoneTo && p <= index->point) {
        continue;
      }

      if (t.common.time > index->time) {
        return false;
      }

      if (index->timeDoneTo != t.common.time) {
        index->timeDoneTo = t.common.time;
        index->point = 0;
      } else {
        index->point = p;
      }

      *a = t;

      return true;
    }
  }

  return false;
}

bool timeBox_findLastAction(TimeBox* tb, MemoryArena* ma, Action* a) {
  bool found = false;

  uint64 seq = 0;
  Action latest = {0};

  for (MemoryBlock* block = lornockData->actionsArena.first; block != 0; block = block->next) {
    ActionChunk* ac = (ActionChunk*) block->start;

    for (uint64 i = 0; i < ac->count; i++) {
      Action t = ac->actions[i];

      if (t.common.sequence > seq) {
        seq = t.common.sequence;

        latest = t;
        found = true;
      }
    }
  }

  *a = latest;

  return found;
}
