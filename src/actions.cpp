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

void action_print(Action a) {
  switch (a.type) {
    case MOVE:
      {
        logfln("MOVE t=%ld s=%lu j=%lu pos=(%f,%f,%f)", a.common.time, a.common.sequence, a.common.jumpID, a.move.pos.x, a.move.pos.y, a.move.pos.z);
      } break;
    case JUMP:
      {
        logfln("JUMP t=%ld s=%lu j=%lu dt=%ld)", a.common.time, a.common.sequence, a.common.jumpID, a.jump.destination);
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

void getLine(char* line, uint32* lineLen, uint32* upTo, char* source, uint32 sourceLen) {
  line[0] = '\0';

  uint32 i = 0;

  while (i + *upTo < sourceLen) {
    line[i] = '\0';

    char c = source[i + *upTo];

    if (c == '\n') {
      break;
    }

    line[i] = c;

    i++;
  }

  *lineLen = i;
  *upTo = i + *upTo;
}

void eatUntilChar(char* paramName, uint32 len, char to, char* line, uint32 lineLen, uint32* upTo) {
  uint32 i = 0;

  while (i + *upTo < lineLen) {
    paramName[i] = '\0';

    char c = line[i + *upTo];

    if (i > len) {
      logln("WARNING: eatUntilChar buffer not big enough");

      break;
    }

    if (c == to) {
      break;
    }

    paramName[i] = c;

    i++;
  }

  *upTo = *upTo + i;
}

#define TIME_BOX_PAGE_SIZE (128)
#define TIME_BOX_TICKS_PER_SECOND (10) // per second
#define TIME_BOX_TICK_MS_INTERVAL (1000/TIME_BOX_TICKS_PER_SECOND)

struct TimeBox {
  char* saveName;

  int64 actionIndex;
  uint32 nextTickTime;

  int64 actionCount;
  Action actions[TIME_BOX_PAGE_SIZE];

  int64 time;
  uint64 sequence;
  uint64 jumpID;
};

bool timeBox_init(TimeBox* tb, const char* name) {
  // Set TimeBox to 0
  tb->actionIndex = 0;
  tb->nextTickTime = 0;
  tb->actionCount = 0;

  tb->time = 0;
  tb->sequence = 0;
  tb->jumpID = 0;

  // Load save file
  char saveFilename[128];

  snprintf(saveFilename, 128, "data/saves/%s.timeline", name);

  void* rawData;
  uint32 rawDataLen;

  loadFromFile(saveFilename, &rawData, &rawDataLen);

  char* data = (char*) rawData;

  uint32 lineLen = 0;
  char* line = (char*) lornockMemory->transientStorage;

  char actionName[64];

  char paramName[64];
  char paramValue[64];

  int64* actionTime = 0;
  uint64* actionSequence = 0;
  uint64* actionJump = 0;

  for (uint32 sourceI = 0; sourceI < rawDataLen; sourceI++) {
    getLine(line, &lineLen, &sourceI, data, rawDataLen);

    Action a;

    uint32 lineI = 0;
    for (lineI; lineI < lineLen; lineI++) {
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

      continue;
    }

    for (lineI; lineI < lineLen; lineI++) {
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

    tb->actions[tb->actionCount] = a;

    tb->actionCount++;
  }

  return true;
}

bool timeBox_nextAction(TimeBox *tb, Action* a, bool peek = false) {
  while (tb->actionIndex < tb->actionCount) {
    Action t = tb->actions[tb->actionIndex];

    if (t.common.time > tb->time) {
      break;
    }

    *a = tb->actions[tb->actionIndex];

    if (!peek) {
      tb->actionIndex += 1;
    }

    return true;
  }

  // TODO(harrison): get next page, etc.

  return false;
}

bool timeBox_nextActionInSequenceOfType(TimeBox *tb, Action* a, int s, uint32 type) {
  for (int i = 0; i < tb->actionCount; i++) {
    Action t = tb->actions[i];

    if (t.common.sequence == s + 1) {
      if (t.common.type == type) {
        *a = t;

        return true;
      }

      s += 1;
    }
  }

  // Scan next page

  return false;
}
