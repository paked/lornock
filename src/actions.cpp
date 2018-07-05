#define ACTION_COMMON_FIELDS \
  uint32 type; \
  int64 time; \
  uint64 sequence; \
  uint64 jump;

enum ActionType {
  NONE,
  MOVE,
  JUMP,
  SPAWN
};

struct MoveAction {
  ACTION_COMMON_FIELDS

  vec3 pos;
};

struct JumpAction {
  ACTION_COMMON_FIELDS

  int64 destination;
};

struct SpawnAction {
  ACTION_COMMON_FIELDS

  vec3 pos;
};

union Action {
  uint32 type;

  SpawnAction spawn;
  MoveAction move;
  JumpAction jump;
};

void action_print(Action a) {
  switch (a.type) {
    case MOVE:
      {
        logfln("MOVE t=%ld s=%lu j=%lu pos=(%f,%f,%f)", a.move.time, a.move.sequence, a.move.jump, a.move.pos.x, a.move.pos.y, a.move.pos.z);
      } break;
    case JUMP:
      {
        logfln("JUMP t=%ld s=%lu j=%lu dt=%ld)", a.jump.time, a.jump.sequence, a.jump.jump, a.jump.destination);
      } break;
    case SPAWN:
      {
        logfln("SPAWN t=%ld s=%lu j=%lu pos=(%f,%f,%f)", a.spawn.time, a.spawn.sequence, a.spawn.jump, a.spawn.pos.x, a.spawn.pos.y, a.spawn.pos.z);
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

#define PAGE_SIZE 128

struct TimeBox {
  char* saveFile;

  int64 actionCount;
  Action actions[PAGE_SIZE];

  int64 time;
  uint64 sequence;
  uint64 jumpID;
};

bool timeBox_init(TimeBox* tb, const char* name) {
  tb->actionCount = 0;

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
      a.type = MOVE;

      actionTime = &a.move.time;
      actionSequence = &a.move.sequence;
      actionJump = &a.move.jump;
    } else if (strcmp("JUMP", actionName) == 0) {
      logfln("what: '%s'", actionName);
      a.type = JUMP;

      actionTime = &a.jump.time;
      actionSequence = &a.jump.sequence;
      actionJump = &a.jump.jump;
    } else if (strcmp("SPAWN", actionName) == 0) {
      a.type = SPAWN;

      actionTime = &a.spawn.time;
      actionSequence = &a.spawn.sequence;
      actionJump = &a.spawn.jump;
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

  for (int i = 0; i < tb->actionCount; i++) {
    action_print(tb->actions[i]);
  }

  return true;
}
