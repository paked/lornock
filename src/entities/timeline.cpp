#define TIMELINE_WRITE_BUFFER_SIZE (128)
#define TIMELINE_PAGE_SIZE (128)
#define TIMELINE_TICKS_PER_SECOND (10)
#define TIMELINE_TICK_MS_INTERVAL (1000/TIMELINE_TICKS_PER_SECOND)

struct TimeIndex {
  int64 time;
  uint64 sequence;
  uint64 jumpID;

  int64 timeDoneTo;
  uint64 point;
};

// TODO(harrison): convert this into the `TimeIndex timeIndex_init()` form.
void timeIndex_init(TimeIndex* index) {
  index->time = 0;
  index->sequence = index->jumpID = 0;

  index->timeDoneTo = -1;
  index->point = 0;
}

typedef uint8 Environment[MAX_FACE][WORLD_HEIGHT][WORLD_WIDTH];

void environment_copy(Environment* from, Environment* to) {
  memcpy(to, from, sizeof(uint8) * MAX_FACE * WORLD_HEIGHT * WORLD_WIDTH);
}

void environment_handle(Environment* e, bool place, uint32 face, int x, int y) {
  ensure(
      (face >= BACK && face < MAX_FACE) &&
      (x >= 0 && x < WORLD_WIDTH) &&
      (y >= 0 && y < WORLD_HEIGHT));

  uint8* ptr = *(*(*e + face) + y) + x;

  if (place) {
    *ptr = BLOCK_COAL;
  } else {
    *ptr = BLOCK_NONE;
  }
}

void environment_handle(Environment* e, TouchAction ta) {
  environment_handle(e, ta.place, ta.face, ta.x, ta.y);
}

struct TimelineInfo {
  Environment initialState;

  // TODO(harrison): everything from here down needs to be moved into a
  // separate `game save` specific struct. it will need to be shared between
  // timelines.
  Inventory inventory;

  quat camera;

  vec3 up;
  vec3 right;
  vec3 forward;
};

void timelineInfo_serialize(TimelineInfo* tli, Serializer* s) {
  // Serialize environment
  for (int face = 0; face < MAX_FACE; face++) {
    for (int y = 0; y < WORLD_HEIGHT; y++) {
      for (int x = 0; x < WORLD_WIDTH; x++) {
        serializer_uint8(s, &tli->initialState[face][y][x]);
      }
    }
  }

  serializer_quat(s, &tli->camera);

  serializer_vec3(s, &tli->up);
  serializer_vec3(s, &tli->right);
  serializer_vec3(s, &tli->forward);

  for (int i = 0; i < INVENTORY_SIZE; i++) {
    serializer_uint32(s, &tli->inventory[i]);
  }
}

struct Timeline {
  TimelineInfo info;

  ActionChunk toWrite;
  MemoryArena* arena;
};

bool timeline_nextAction(Timeline* tb, TimeIndex* index, Action *a);

void timeline_init(Timeline* tb, MemoryArena* ma) {
  tb->toWrite.count = 0;

  tb->arena = ma;
}

void timeline_create(Timeline* tb) {
  memoryArena_clear(tb->arena);

  // Generate world
  {
    for (int face = 0; face < MAX_FACE; face++) {
      for (int y = 0; y < WORLD_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
          tb->info.initialState[face][y][x] = ((real32)rand() / (real32)RAND_MAX) > 0.6f ? 1 : 0;
        }
      }
    }

    tb->info.up = faceCardinalDirections[TOP][DIRECTION_UP];
    tb->info.right = faceCardinalDirections[TOP][DIRECTION_RIGHT];
    tb->info.forward = faceCardinalDirections[TOP][DIRECTION_FORWARD];

    tb->info.camera = quatFromPitchYawRoll(90.0f, 0.0f, 0.0f);
  }

  // Generate player spawn action
  {
    ActionChunk* chunk = memoryArena_pushStruct(tb->arena, ActionChunk);
    Action a = action_makeSpawn(PLAYER_DEFAULT_SPAWN);

    a.common.time = 0;

    a.common.sequence = a.common.jumpID = 0;

    chunk->actions[0] = a;
    chunk->count += 1;
  }

  // Create inventory
  {
    for (int i = 0; i < INVENTORY_SIZE; i++) {
      tb->info.inventory[i] = BLOCK_NONE;
    }
  }
}

void timeline_load(Timeline* tb, const char* name) {
  char saveFilename[128];
  snprintf(saveFilename, 128, "data/saves/%s.timeline", name);

  void* rawData;
  uint32 rawLen = 0;

  loadFromFile(saveFilename, &rawData, &rawLen);

  if (rawLen == 0) {
    logln("INFO: creating file");
    timeline_create(tb);

    return;
  }

  {
    char infoFilename[128];
    snprintf(infoFilename, 128, "data/saves/%s.info", name);

    MemoryArena arena;
    loadFromFileAsArena((const char*) infoFilename, &arena);

    Serializer s;
    serializer_init(&s, SERIALIZER_MODE_READ, arena);

    timelineInfo_serialize(&tb->info, &s);
  }

  uint32 rawUpTo = 0;
  char *raw = (char*) rawData;

  memoryArena_clear(tb->arena);

  ActionChunk* chunk = memoryArena_pushStruct(tb->arena, ActionChunk);
  chunk->count = 0;

  // TODO(harrison): ensure that new memory gets zeroed out.
  // TODO(harrison): test this solution handling multi-chunk things

  char* line = (char*) lornockMemory->transientStorage;
  uint32 lineLen = 0;
  while (rawUpTo < rawLen) {
    getLine(line, &lineLen, &rawUpTo, raw, rawLen);
    rawUpTo += 1;

    Action a;

    ensure(action_parse(&a, line, lineLen));

    chunk->actions[chunk->count] = a;
    chunk->count += 1;

    if (chunk->count >= ACTION_CHUNK_MAX) {
      chunk = memoryArena_pushStruct(tb->arena, ActionChunk);
      chunk->count = 0;
    }
  }
}

void timeline_commit(Timeline* tb) {
#define commit(action) \
  if (!actionChunk_add(chunk, action)) {\
    logln("RAN OVER CHUNK. DOING ANOTHER THING.");\
    chunk = memoryArena_pushStruct(temp, ActionChunk);\
    ensure(actionChunk_add(chunk, action));\
  }

  TimeIndex index;
  timeIndex_init(&index);

  MemoryArena* temp = &lornockData->tempArena;

  memoryArena_clear(temp);

  ActionChunk* chunk = memoryArena_pushStruct(temp, ActionChunk);
  chunk->count = 0;

  uint32 i = 0;
  while (i < tb->toWrite.count) {
    Action toWrite = tb->toWrite.actions[i];

    index.time = toWrite.common.time;

    {
      Action a;
      while (timeline_nextAction(tb, &index, &a)) {
        commit(a);
      }
    }

    commit(toWrite);

    i += 1;
  }

  index.time = INT64_MAX;

  Action a;
  while (timeline_nextAction(tb, &index, &a)) {
    commit(a);
  }

#undef commit

  memoryArena_clear(tb->arena);

  for (MemoryBlock* block = temp->first; block != 0; block = block->next) {
    chunk = memoryArena_pushStruct(tb->arena, ActionChunk);

    ActionChunk* ac = (ActionChunk*) block->start;

    *chunk = *ac;
  }

  tb->toWrite.count = 0;
}

void timeline_save(Timeline* tb, TimeIndex worldIndex) {
  if (tb->toWrite.count > 0) {
    timeline_commit(tb);
  }

  char* out = (char*) lornockMemory->transientStorage;
  out[0] = '\0';

  TimeIndex index;
  timeIndex_init(&index);

  index.time = INT64_MAX;

  Action a;
  while (timeline_nextAction(tb, &index, &a)) {
    action_serialize(a, out);
  }

  writeToFile("data/saves/simple.timeline", (void*) out, strlen(out));

  {
    memoryArena_clear(&lornockData->tempArena);

    Serializer s;
    serializer_init(&s, SERIALIZER_MODE_WRITE, lornockData->tempArena);

    timelineInfo_serialize(&tb->info, &s);

    writeArenaToFile("data/saves/simple.info", &s.buffer);
  }
}

void timeline_add(Timeline* tb, TimeIndex* index, Action a) {
  index->sequence += 1;
  a.common.time = index->time;
  a.common.sequence = index->sequence;
  a.common.jumpID = index->jumpID;

  if (!actionChunk_add(&tb->toWrite, a)) {
    timeline_commit(tb);

    ensure(actionChunk_add(&tb->toWrite, a));
  }
}

bool timeline_nextActionInSequence(Timeline* tb, uint64 sequence, Action *a) {
  for (MemoryBlock* block = tb->arena->first; block != 0; block = block->next) {
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

bool timeline_actionInSequence(Timeline* tb, uint64 sequence, Action *a) {
  for (MemoryBlock* block = tb->arena->first; block != 0; block = block->next) {
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


bool timeline_nextAction(Timeline* tb, TimeIndex* index, Action *a) {
  uint64 p = 0;
  int64 lastTime = -1;

  for (MemoryBlock* block = tb->arena->first; block != 0; block = block->next) {
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

bool timeline_findLastAction(Timeline* tb, Action* a) {
  bool found = false;

  uint64 seq = 0;
  Action latest = {0};

  for (MemoryBlock* block = tb->arena->first; block != 0; block = block->next) {
    ActionChunk* ac = (ActionChunk*) block->start;

    for (uint64 i = 0; i < ac->count; i++) {
      Action t = ac->actions[i];

      if (t.common.sequence >= seq) {
        seq = t.common.sequence;

        latest = t;
        found = true;
      }
    }
  }

  *a = latest;

  return found;
}

void timeline_getEnvironmentStateAt(Timeline *tb, Environment* env, int64 time) {
  environment_copy(&tb->info.initialState, env);

  TimeIndex index;
  timeIndex_init(&index);

  index.time = time;

  Action a;
  while (timeline_nextAction(tb, &index, &a)) {
    if (a.type != TOUCH) {
      continue;
    }

    environment_handle(env, a.touch);
  }
}
