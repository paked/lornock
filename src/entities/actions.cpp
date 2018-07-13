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

bool action_parse(Action* action, char* line, uint32 lineLen) {
  char actionName[64];

  char paramName[64];
  char paramValue[64];
  int64* actionTime = 0;
  uint64* actionSequence = 0;
  uint64* actionJump = 0;

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

    return false;
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

  *action = a;

  return true;
}

#define TIME_BOX_WRITE_BUFFER_SIZE (128)
#define TIME_BOX_PAGE_SIZE (128)
#define TIME_BOX_TICKS_PER_SECOND (10) // per second
#define TIME_BOX_TICK_MS_INTERVAL (1000/TIME_BOX_TICKS_PER_SECOND)

struct TimeBox {
  char* saveName;

  int64 actionIndex;
  uint32 nextTickTime;

  char* raw;
  uint32 rawLen;
  uint32 rawUpTo;

  int64 writeCount;
  Action writeBuffer[TIME_BOX_WRITE_BUFFER_SIZE];

  int64 time;
  uint64 sequence;
  uint64 jumpID;
};

void timeBox_setTime(TimeBox* tb, int64 time) {
  tb->time = time;

  uint32 lineLen = 0;
  char* line = (char*) lornockMemory->transientStorage;

  tb->rawUpTo = 0;

  while (tb->rawUpTo < tb->rawLen) {
    getLine(line, &lineLen, &tb->rawUpTo, tb->raw, tb->rawLen);

    tb->rawUpTo += 1;

    Action t;

    if (!action_parse(&t, line, lineLen)) {
      logln("Could not parse line!");

      continue;
    }

    if (t.common.time > tb->time) {
      break;
    }
  }
}

void timeBox_read(TimeBox* tb, const char* name) {
  char saveFilename[128];
  snprintf(saveFilename, 128, "data/saves/%s.timeline", name);

  void* rawData;

  loadFromFile(saveFilename, &rawData, &tb->rawLen);

  tb->raw = (char*) rawData;

  char infoFname[128];
  snprintf(infoFname, 128, "data/saves/%s.info", name);

  uint32 infoLen;
  void* infoData;

  loadFromFile(infoFname, &infoData, &infoLen);

  int64 time;
  uint64 sequence;
  uint64 jumpID;

  sscanf((char*) infoData, "%ld %lu %lu", &time, &sequence, &jumpID);

  timeBox_setTime(tb, time);

  tb->sequence = sequence;
  tb->jumpID = jumpID;
}

bool timeBox_init(TimeBox* tb, const char* name) {
  // Set TimeBox to 0
  tb->actionIndex = 0;
  tb->nextTickTime = 0;
  tb->writeCount = 0;

  tb->rawUpTo = 0;

  timeBox_read(tb, name);

  return true;
}

void timeBox_save(TimeBox* tb) {
  char* out = (char*) lornockMemory->transientStorage;
  out[0] = '\0';

  uint32 parseLineLen = 0;
  char parseLine[256];

  char outLine[256] = {0};

  uint32 upTo = 0;
  uint32 wroteTo = 0;

  while (wroteTo < tb->writeCount) {
    Action toWrite = tb->writeBuffer[wroteTo];

    uint32 tempUpTo = upTo;

    while (getLine(parseLine, &parseLineLen, &tempUpTo, tb->raw, tb->rawLen)) {
      tempUpTo += 1;

      Action a;

      if (!action_parse(&a, parseLine, parseLineLen)) {
        logln("WARNING: failed to parse action while writing");

        continue;
      }

      if (a.common.time > toWrite.common.time) {
        break;
      }

      upTo = tempUpTo;

      switch (a.type) {
        case MOVE:
          {
            snprintf(outLine, 256, "MOVE t=%ld s=%lu j=%lu pos=(%f,%f,%f)\n", a.common.time, a.common.sequence, a.common.jumpID, a.move.pos.x, a.move.pos.y, a.move.pos.z);
          } break;
        case JUMP:
          {
            snprintf(outLine, 256, "JUMP t=%ld s=%lu j=%lu dt=%ld\n", a.common.time, a.common.sequence, a.common.jumpID, a.jump.destination);
          } break;
        case SPAWN:
          {
            snprintf(outLine, 256, "SPAWN t=%ld s=%lu j=%lu pos=(%f,%f,%f)\n", a.common.time, a.common.sequence, a.common.jumpID, a.spawn.pos.x, a.spawn.pos.y, a.spawn.pos.z);
          } break;
        default:
          {
            logln("ERROR: cannot serialize unknown action type");

            continue;
          } break;
      }

      strcat(out, outLine);
    }

    switch (toWrite.type) {
      case MOVE:
        {
          snprintf(outLine, 256, "MOVE t=%ld s=%lu j=%lu pos=(%f,%f,%f)\n", toWrite.common.time, toWrite.common.sequence, toWrite.common.jumpID, toWrite.move.pos.x, toWrite.move.pos.y, toWrite.move.pos.z);
        } break;
      case JUMP:
        {
          snprintf(outLine, 256, "JUMP t=%ld s=%lu j=%lu dt=%ld\n", toWrite.common.time, toWrite.common.sequence, toWrite.common.jumpID, toWrite.jump.destination);
        } break;
      case SPAWN:
        {
          snprintf(outLine, 256, "SPAWN t=%ld s=%lu j=%lu pos=(%f,%f,%f)\n", toWrite.common.time, toWrite.common.sequence, toWrite.common.jumpID, toWrite.spawn.pos.x, toWrite.spawn.pos.y, toWrite.spawn.pos.z);
        } break;
      default:
        {
          continue;
        } break;
    }

    strcat(out, outLine);

    wroteTo += 1;
  }

  while (upTo < tb->rawLen) {
    getLine(parseLine, &parseLineLen, &upTo, tb->raw, tb->rawLen);
    upTo += 1;

    Action a;

    if (!action_parse(&a, parseLine, parseLineLen)) {
      logln("Could not parse line");

      continue;
    }

    switch (a.type) {
      case MOVE:
        {
          snprintf(outLine, 256, "MOVE t=%ld s=%lu j=%lu pos=(%f,%f,%f)\n", a.common.time, a.common.sequence, a.common.jumpID, a.move.pos.x, a.move.pos.y, a.move.pos.z);
        } break;
      case JUMP:
        {
          snprintf(outLine, 256, "JUMP t=%ld s=%lu j=%lu dt=%ld\n", a.common.time, a.common.sequence, a.common.jumpID, a.jump.destination);
        } break;
      case SPAWN:
        {
          snprintf(outLine, 256, "SPAWN t=%ld s=%lu j=%lu pos=(%f,%f,%f)\n", a.common.time, a.common.sequence, a.common.jumpID, a.spawn.pos.x, a.spawn.pos.y, a.spawn.pos.z);
        } break;
      default:
        {
          continue;
        } break;
    }

    strcat(out, outLine);
  }

  tb->writeCount = 0;
  writeToFile("data/saves/simple.timeline", (void*) out, strlen(out));

  char info[128];

  snprintf(info, 128, "%ld %lu %lu", tb->time, tb->sequence, tb->jumpID);

  writeToFile("data/saves/simple.info", info, strlen(info));
}

void timeBox_add(TimeBox* tb, Action a) {
  if (tb->writeCount >= TIME_BOX_WRITE_BUFFER_SIZE - 1) {
    logln("flushing buffer!");
    timeBox_save(tb);
  }

  tb->sequence += 1;
  a.common.time = tb->time;
  a.common.sequence = tb->sequence;
  a.common.jumpID = tb->jumpID;

  tb->writeBuffer[tb->writeCount] = a;

  tb->writeCount += 1;
}

bool timeBox_nextAction(TimeBox *tb, Action* a) {
  uint32 lineLen = 0;
  char* line = (char*) lornockMemory->transientStorage;

  while (tb->rawUpTo < tb->rawLen) {
    getLine(line, &lineLen, &tb->rawUpTo, tb->raw, tb->rawLen);

    tb->rawUpTo += 1;

    Action t;

    if (!action_parse(&t, line, lineLen)) {
      logln("Could not parse line!");

      continue;
    }

    if (t.common.time > tb->time) {
      break;
    }

    *a = t;

    return true;
  }

  return false;
}

bool timeBox_findNextActionInSequenceOfType(TimeBox *tb, Action* a, int s, uint32 type) {
  uint32 lineLen = 0;
  char* line = (char*) lornockMemory->transientStorage;

  uint32 upTo = 0;

  while (upTo < tb->rawLen) {
    getLine(line, &lineLen, &upTo, tb->raw, tb->rawLen);

    upTo += 1;

    Action t;

    if (!action_parse(&t, line, lineLen)) {
      logln("Could not parse line!");

      continue;
    }

    if (t.common.sequence == s + 1) {
      if (t.common.type == type) {
        *a = t;

        return true;
      }

      s += 1;
    }
  }

  return false;
}


bool timeBox_findNextActionInSequence(TimeBox *tb, Action* a, int s) {
  uint32 lineLen = 0;
  char* line = (char*) lornockMemory->transientStorage;

  uint32 upTo = 0;

  while (upTo < tb->rawLen) {
    getLine(line, &lineLen, &upTo, tb->raw, tb->rawLen);

    upTo += 1;

    Action t;

    if (!action_parse(&t, line, lineLen)) {
      logln("Could not parse line!");

      continue;
    }

    if (t.common.sequence == s + 1) {
      *a = t;

      return true;
    }
  }

  return false;
}

bool timeBox_findLastActionInSequenceOfType(TimeBox *tb, Action* a, int s, uint32 type) {
  uint32 lineLen = 0;
  char* line = (char*) lornockMemory->transientStorage;

  uint32 upTo = 0;

  while (upTo < tb->rawLen) {
    getLine(line, &lineLen, &upTo, tb->raw, tb->rawLen);

    upTo += 1;

    Action t;

    if (!action_parse(&t, line, lineLen)) {
      logln("Could not parse line!");

      continue;
    }

    if (t.common.sequence == s) {
      if (t.common.type == type) {
        *a = t;

        return true;
      }

      s -= 1;
    }
  }

  return false;
}
