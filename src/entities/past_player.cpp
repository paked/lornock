struct PastPlayer {
  bool exists;
  bool moving;

  // TODO(harrison): refactor these into a TimeIndex?
  uint64 sequence;
  int64 lastTime;

  vec3 pos;
  vec3 start;
  vec3 destination;

  uint32 startTime;
  uint32 duration;
};

void pastPlayer_setupNextMove(PastPlayer* pp, Timeline* tb, TimeIndex* worldIndex, MemoryArena* ma);

void pastPlayer_init(PastPlayer* pp, Timeline *tb, TimeIndex* worldIndex, MemoryArena* ma, Action first) {
  pp->exists = true;
  pp->moving = true;

  pp->sequence = first.common.sequence;

  vec3 pos = {0};

  if (first.type == MOVE) {
    pos = first.move.pos;
  } else if(first.type == SPAWN) {
    pos = first.spawn.pos;
  } else {
    assert(false);

    logln("ERROR: invalid action to init a pastPlayer with");
  }

  pp->pos = pos;
  pp->lastTime = first.common.time;

  pp->startTime = 0;
  pp->duration = 0;

  pastPlayer_setupNextMove(pp, tb, worldIndex, ma);
}

void pastPlayer_update(PastPlayer* pp, Timeline* tb, TimeIndex* worldIndex, MemoryArena* ma) {
  if (!pp->exists) {
    return;
  }

  Action a;
  while (timeline_nextActionInSequence(tb, ma, pp->sequence, &a) && a.common.time < worldIndex->time) {
    pp->sequence = a.common.sequence;

    switch (a.type) {
      case MOVE:
        {
          pp->pos = a.move.pos;
          pp->lastTime = a.common.time;

          pastPlayer_setupNextMove(pp, tb, worldIndex, ma);
        } break;
      case JUMP:
        {
          pp->moving = pp->exists = false;
        } break;
    }
  }

  if (!pp->moving) {
    return;
  }

  real32 time = pp->startTime + pp->duration;

  if (time < getTime()) {
    return;
  }

  real32 pc = 1.0f - ((real32) (time - getTime())) / (pp->duration); 
  pc = clamp(pc, 0, 1);

  pp->pos = vec3Lerp(pc, pp->start, pp->destination);
}

void pastPlayer_setupNextMove(PastPlayer* pp, Timeline* tb, TimeIndex* worldIndex, MemoryArena* ma) {
  Action next = { 0 };
  uint64 current = pp->sequence;

  while (next.common.type != MOVE) {
    if (!timeline_nextActionInSequence(tb, ma, current, &next)) {
      pp->moving = false;

      break;
    }

    current += 1;
  }

  if (!pp->moving) {
    return;
  }

  pp->start = pp->pos;
  pp->destination = next.move.pos;
  pp->startTime = getTime();
  pp->duration = (real32) TIMELINE_TICK_MS_INTERVAL * (real32)((next.common.time + 1) - pp->lastTime);
}
