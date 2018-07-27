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

void timeIndex_init(TimeIndex* index) {
  index->time = 0;
  index->sequence = index->jumpID = 0;

  index->timeDoneTo = -1;
  index->point = 0;
}

struct Timeline {
  ActionChunk toWrite;
  MemoryArena actions;
};

bool timeline_nextAction(Timeline* tb, MemoryArena* ma, TimeIndex* index, Action *a);

void timeline_init(Timeline* tb) {
  tb->toWrite.count = 0;
}

#define PLAYER_DEFAULT_SPAWN vec3(0.0f, 1.5f, 0.0f)

void timeline_create(Timeline* tb, MemoryArena* ma) {
  memoryArena_clear(ma);

  // Generate world

  // Generate player
  ActionChunk* chunk = memoryArena_pushStruct(ma, ActionChunk);

  Action a = action_makeSpawn(PLAYER_DEFAULT_SPAWN);

  a.common.time = 0;

  a.common.sequence = a.common.jumpID = 0;

  chunk->actions[0] = a;
  chunk->count += 1;
}

void timeline_load(Timeline* tb, MemoryArena* ma, const char* name) {
  char saveFilename[128];
  snprintf(saveFilename, 128, "data/saves/%s.timeline", name);

  void* rawData;
  uint32 rawLen = 0;

  loadFromFile(saveFilename, &rawData, &rawLen);

  if (rawLen == 0) {
    logln("INFO: creating file");
    timeline_create(tb, ma);

    return;
  }

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

void timeline_commit(Timeline* tb, MemoryArena* ma) {
#define commit(action) \
  if (!actionChunk_add(chunk, action)) {\
    logln("RAN OVER CHUNK. DOING ANOTHER THING.");\
    chunk = memoryArena_pushStruct(temp, ActionChunk);\
    assert(actionChunk_add(chunk, action));\
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
      while (timeline_nextAction(tb, ma, &index, &a)) {
        commit(a);
      }
    }

    commit(toWrite);

    i += 1;
  }

  index.time = INT64_MAX;

  Action a;
  while (timeline_nextAction(tb, ma, &index, &a)) {
    commit(a);
  }

#undef commit

  memoryArena_clear(ma);

  for (MemoryBlock* block = temp->first; block != 0; block = block->next) {
    chunk = memoryArena_pushStruct(ma, ActionChunk);

    ActionChunk* ac = (ActionChunk*) block->start;

    *chunk = *ac;
  }

  tb->toWrite.count = 0;
}

void timeline_save(Timeline* tb, TimeIndex worldIndex, MemoryArena* ma) {
  if (tb->toWrite.count > 0) {
    timeline_commit(tb, ma);
  }

  char* out = (char*) lornockMemory->transientStorage;
  out[0] = '\0';

  TimeIndex index;
  timeIndex_init(&index);

  index.time = INT64_MAX;

  Action a;
  while (timeline_nextAction(tb, ma, &index, &a)) {
    action_serialize(a, out);
  }

  writeToFile("data/saves/simple.timeline", (void*) out, strlen(out));
}

void timeline_add(Timeline* tb, TimeIndex* index, MemoryArena* ma, Action a) {
  if (tb->toWrite.count >= ACTION_CHUNK_MAX) {
    logln("flushing buffer!");

    timeline_commit(tb, ma);

    tb->toWrite.count = 0;
  }

  index->sequence += 1;
  a.common.time = index->time;
  a.common.sequence = index->sequence;
  a.common.jumpID = index->jumpID;

  assert(actionChunk_add(&tb->toWrite, a));
}

bool timeline_nextActionInSequence(Timeline* tb, MemoryArena* ma, uint64 sequence, Action *a) {
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

bool timeline_actionInSequence(Timeline* tb, MemoryArena* ma, uint64 sequence, Action *a) {
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


bool timeline_nextAction(Timeline* tb, MemoryArena* ma, TimeIndex* index, Action *a) {
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

bool timeline_findLastAction(Timeline* tb, MemoryArena* ma, Action* a) {
  bool found = false;

  uint64 seq = 0;
  Action latest = {0};

  for (MemoryBlock* block = lornockData->actionsArena.first; block != 0; block = block->next) {
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
