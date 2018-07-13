struct PastPlayer {
  bool exists;
  bool moving;

  vec3 start;
  vec3 destination;

  uint64 sequence;
  uint64 nextSequence;

  vec3 pos;

  uint32 startTime;
  uint32 duration;
};

void pastPlayer_target(PastPlayer* pp, TimeBox *tb) {
  Action next;
  
  if (!timeBox_findNextActionInSequenceOfType(tb, &next, pp->sequence, MOVE)) {
    logln("SETTING PLAYER TO IMMOBILE");
    pp->moving = false;

    return;
  }

  pp->destination = next.move.pos;
  pp->startTime = getTime();
  pp->duration = (real32) TIME_BOX_TICK_MS_INTERVAL * (real32)(next.move.time - tb->time);

  pp->nextSequence = next.move.sequence;
}

void pastPlayer_init(PastPlayer* pp, TimeBox* tb, Action first) {
  pp->exists = true;
  pp->moving = true;

  pp->sequence = first.common.sequence;

  vec3 pos = {0};

  if (first.type == MOVE) {
    pos = first.move.pos;
  } else if(first.type == SPAWN) {
    pos = first.spawn.pos;
  } else {
    logln("ERROR: invalid action to init a pastPlayer with");
  }

  pp->pos = pp->start = pos;

  pastPlayer_target(pp, tb);
}

void pastPlayer_update(PastPlayer* pp, TimeBox* tb) {
  Action a;
  if (!timeBox_findNextActionInSequence(tb, &a, pp->sequence)) {
    logfln("WARNING: PastPlayer should have been killed before this point: %ld", pp->sequence);

    pp->exists = false;
  }

  if (a.type == JUMP) {
    pp->exists = false;

    return;
  }

  if (!pp->moving) {
    return;
  }

  real32 pc = 1.0f - ((real32) ((pp->startTime + pp->duration) - getTime())) / (pp->duration); 
  pc = clamp(pc, 0, 1);

  pp->pos = vec3Lerp(pc, pp->start, pp->destination);

  if (getTime() > pp->startTime + pp->duration) {
    pp->pos = pp->destination;
    pp->sequence = pp->nextSequence;
    pp->start = pp->destination;

    pastPlayer_target(pp, tb);
  }
}


