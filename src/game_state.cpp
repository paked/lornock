#include <entities/actions.cpp>
#include <entities/past_player.cpp>

// Voxel direction/faces
enum {
  BACK,
  FRONT,
  LEFT,
  RIGHT,
  BOTTOM,
  TOP,
  MAX_FACE
};

void printFace(uint32 f) {
  switch(f) {
    case BACK:
      { logln("back face"); } break;
    case FRONT:
      { logln("front face"); } break;
    case LEFT:
      { logln("left face"); } break;
    case RIGHT:
      { logln("right face"); } break;
    case BOTTOM:
      { logln("bottom face"); } break;
    case TOP:
      { logln("top face"); } break;
    default:
      { logln("unknown face"); } break;
  }
}

enum {
  ROT_FORWARD,
  ROT_BACKWARD,
  ROT_LEFT,
  ROT_RIGHT,
  ROT_IDLE
};

void printRot(uint32 r) {
  switch (r) {
    case ROT_FORWARD:
      { logln("forward rotating"); } break;
    case ROT_BACKWARD:
      { logln("backward rotating"); } break;
    case ROT_LEFT:
      { logln("left rotating"); } break;
    case ROT_RIGHT:
      { logln("right rotating"); } break;
    case ROT_IDLE:
      { logln("idle rotating"); } break;
  }
}

enum {
  DIRECTION_UP,
  DIRECTION_RIGHT,
  DIRECTION_FORWARD,
  MAX_DIRECTION
};

vec3 faceCardinalDirections[MAX_FACE][MAX_DIRECTION] = {
  // UP                       Right                     Forward
  { vec3(0.0f, 0.0f, 1.0f),  vec3(1.0f, 0.0f, 0.0f),   vec3(0.0f, 1.0f, 0.0f) },  // Back face
  { vec3(0.0f, 0.0f, -1.0f),   vec3(1.0f, 0.0f, 0.0f),   vec3(0.0f, -1.0f, 0.0f) },   // Front face

  { vec3(-1.0f, 0.0f, 0.0f),  vec3(0.0f, 1.0f, 0.0f),  vec3(0.0f, 0.0f, -1.0f) },  // Left face
  { vec3(1.0f, 0.0f, 0.0f),   vec3(0.0f, -1.0f, 0.0f),  vec3(0.0f, 0.0f, -1.0f) },  // Right face

  { vec3(0.0f, -1.0f, 0.0f),  vec3(1.0f, 0.0f, 0.0f),  vec3(0.0f, 0.0f, 1.0f) },  // Bottom face
  { vec3(0.0f, 1.0f, 0.0f),   vec3(1.0f, 0.0f, 0.0f),   vec3(0.0f, 0.0f, -1.0f) }  // Top face
};

uint32 faceRotationMap[MAX_FACE][ROT_IDLE] = {
  //  FWD     BWD     LWD     RWD
  {   BOTTOM, TOP,    LEFT,   RIGHT },  // Back face
  {   TOP,    BOTTOM, LEFT,   RIGHT },  // Front face
  {   TOP,    BOTTOM, BACK,   FRONT },  // Left face
  {   TOP,    BOTTOM, FRONT,  BACK  },  // Right face
  {   FRONT,  BACK,   LEFT,   RIGHT },  // Bottom face
  {   BACK,   FRONT,  LEFT,   RIGHT }   // Top face
};

bool voxelEmptyToThe(int d, uint8 w[WORLD_HEIGHT][WORLD_DEPTH][WORLD_WIDTH], int x, int y, int z) {
  int offsetX = 0;
  int offsetY = 0;
  int offsetZ = 0;

  switch(d) {
    case BACK:
      {
        offsetZ = -1;
      } break;
    case FRONT:
      {
        offsetZ = 1;
      } break;
    case LEFT:
      {
        offsetX = -1;
      } break;
    case RIGHT:
      {
        offsetX = 1;
      } break;
    case BOTTOM:
      {
        offsetY = -1;
      } break;
    case TOP:
      {
        offsetY = 1;
      } break;
    default:
      {
        logln("WARNING: unknown voxel face type");

        return false;
      } break;
  }

  int posX = x + offsetX;
  int posY = y + offsetY;
  int posZ = z + offsetZ;

  if (posX < 0 || posX > WORLD_WIDTH - 1 ||
      posY < 0 || posY > WORLD_HEIGHT - 1||
      posZ < 0 || posZ > WORLD_DEPTH - 1) {
    return true;
  }

  return w[posY][posZ][posX] != 1;
}

void addFaceToMesh(uint32 d, real32* verts, uint64* len, vec3 offset) {
  real32 cube[] = CUBE_MESH_DATA;

  uint64 start = d * CUBE_MESH_DATA_FACE_LENGTH;
  uint64 end = start + CUBE_MESH_DATA_FACE_LENGTH;

  for (uint64 i = start; i < end; i++) {
    real32 v = cube[i];

    if (i % 5 < 3) {
      v += offset[i % 5];
    }

    verts[*len] = v;

    *len += 1;
  }
}

#define CAMERA_ROTATION_OFFSET (-30)
#define CAMERA_POSITION (vec3(0, 0.0f, -7.25f))
#define MAX_PAST_PLAYERS 10
#define ROTATION_DURATION 400

struct GameState {
  uint32 currentFace;
  vec3 faceRight;
  vec3 faceForward;

  vec3 playerUp;
  vec3 playerRight;
  vec3 playerForward;
  vec3 playerPos;
  vec3 playerLastMove;
  mat4 playerRotation;

  quat cameraRotation;

  bool rotating;
  uint32 rotatingStartTime;
  real32 rotatingPitchEnd;
  real32 rotatingYawEnd;
  real32 rotatingRollEnd;
  quat rotatingStart;

  PastPlayer pastPlayers[MAX_PAST_PLAYERS];

  TimeIndex timeIndex;
  uint64 timeBoxNextTickTime;
  TimeBox timeBox;

  uint8 world[WORLD_HEIGHT][WORLD_DEPTH][WORLD_WIDTH];
  Mesh worldMesh;

  Mesh cubeMesh;

  GLuint cubeVAO;
};

void gameState_spawnNecessaryPastPlayers(GameState *g, TimeBox* tb, TimeIndex* worldIndex, MemoryArena *ma);

void gameState_setFaceDirections(GameState* g) {
  mat4 view = mat4d(1);
  view = mat4Translate(view, CAMERA_POSITION);
  view = view * quatToMat4(g->cameraRotation);

  g->faceRight = vec3(view[0][0], view[1][0], view[2][0]);
  g->faceForward = vec3(view[0][1], view[1][1], view[2][1]);
}

int gameState_getFirstPastPlayer(GameState* g) {
  int id = -1;

  for (int i = 0; i < MAX_PAST_PLAYERS; i++) {
    if (!g->pastPlayers[i].exists) {
      id = i;

      break;
    }
  }

  return id;
}

void gameState_disableAllPastPlayers(GameState* g) {
  for (int i = 0; i < MAX_PAST_PLAYERS; i++) {
    g->pastPlayers[i].exists = false;
  }
}

void gameState_init(State* state) {
  LornockMemory* m = lornockMemory;
  GameState* g = (GameState*) state->memory;

  g->currentFace = TOP;
  gameState_setFaceDirections(g); // g->faceRight, g->faceForward

  g->playerUp = faceCardinalDirections[g->currentFace][DIRECTION_UP];
  g->playerRight = faceCardinalDirections[g->currentFace][DIRECTION_RIGHT];
  g->playerForward = faceCardinalDirections[g->currentFace][DIRECTION_FORWARD];
  g->playerPos = vec3(0.0f, 1.5f, 0.0f);
  g->playerLastMove = vec3_zero;
  g->playerRotation = mat4d(1.0f);

  g->cameraRotation = quatFromPitchYawRoll(90.0f, 0.0f, 0.0f);

  g->rotating = false;
  g->rotatingStartTime = 0;
  g->rotatingPitchEnd = 0;
  g->rotatingYawEnd = 0;
  g->rotatingRollEnd = 0;
  g->rotatingStart = quatFromPitchYawRoll(0.0f, 0.0f, 0.0f);

  gameState_disableAllPastPlayers(g);

  timeBox_load(&g->timeBox, &g->timeIndex, &lornockData->actionsArena, "simple");
  g->timeBoxNextTickTime = 0;

  for (int y = 0; y < WORLD_HEIGHT; y++) {
    for (int z = 0; z < WORLD_DEPTH; z++) {
      for (int x = 0; x < WORLD_WIDTH; x++) {
        g->world[y][z][x] = 1;
      }
    }
  }

  // init world
  for (int y = 0; y < WORLD_HEIGHT; y++) {
    for (int z = 0; z < WORLD_DEPTH; z++) {
      for (int x = 0; x < WORLD_WIDTH; x++) {
        g->world[y][z][x] = 1;
      }
    }
  }

  uint64 count = 0;
  real32* verts = (real32*) m->transientStorage;

  for (int y = 0; y < WORLD_HEIGHT; y++) {
    for (int z = 0; z < WORLD_DEPTH; z++) {
      for (int x = 0; x < WORLD_WIDTH; x++) {
        if (g->world[y][z][x] != 1) {
          continue;
        }

        vec3 pos = vec3(x, y, z);

        for (int d = 0; d < MAX_FACE; d++) {
          if (!voxelEmptyToThe(d, g->world, x, y, z)) {
            continue;
          }

          addFaceToMesh(d, verts, &count, pos);
        }
      }
    }
  }

  g->worldMesh = mesh_init(verts, count);

  real32 cubeData[] = CUBE_MESH_DATA;
  g->cubeMesh = mesh_init(cubeData, sizeof(cubeData)/sizeof(real32));

  assetsRequestShader(SHADER_default);
  assetsRequestTexture(TEXTURE_test);
  assetsRequestTexture(TEXTURE_player);
  assetsRequestTexture(TEXTURE_rock);

  draw.clear = vec4(18.0f, 26.0f, 47.0f, 1.0f);


  gameState_spawnNecessaryPastPlayers(g, &g->timeBox, &g->timeIndex, &lornockData->actionsArena);

  {
    Action a = { 0 };
    bool found = false;
    uint64 current = g->timeIndex.sequence;

    while (timeBox_actionInSequence(&g->timeBox, &lornockData->actionsArena, current, &a)) {
      if (a.common.type == MOVE || a.common.type == SPAWN) {
        found = true;

        break;
      }

      current -= 1;
    }

    if (found) {
      if (a.common.type == MOVE) {
        g->playerPos = a.move.pos;
      } else if (a.common.type == SPAWN) {
        g->playerPos = a.spawn.pos;
      }

      logfln("starting at: %f %f %f", a.move.pos.x, a.move.pos.y, a.move.pos.z);
    } else {
      logln("ERROR: COULD NOT FIND THE LAST ACTION, AND AM SUBSEQUENTLY UNABLET O SET THE POSITION OF THE PLAYER PROPERLY.");
    }
  }
}

void gameState_rotate(GameState* g, uint32 direction) {
  real32 pitchFactor = 0.0f;
  real32 yawFactor = 0.0f;

  mat4 rot;

  switch(direction) {
    case ROT_FORWARD:
      {
        pitchFactor = 1.0f;

        rot = HMM_Rotate(-90.0f, g->playerRight);

        vec4 forward = rot * vec4FromVec3(g->playerForward);
        g->playerForward = vec3FromVec4(forward);

        vec4 up = rot * vec4FromVec3(g->playerUp);
        g->playerUp = vec3FromVec4(up);
      } break;
    case ROT_BACKWARD:
      {
        pitchFactor = -1.0f;

        rot = HMM_Rotate(90.0f, g->playerRight);

        vec4 forward = rot * vec4FromVec3(g->playerForward);
        g->playerForward = vec3FromVec4(forward);

        vec4 up = rot * vec4FromVec3(g->playerUp);
        g->playerUp = vec3FromVec4(up);
      } break;
    case ROT_LEFT:
      {
        yawFactor = 1.0f;

        rot = HMM_Rotate(-90.0f, g->playerForward);

        vec4 right = rot * vec4FromVec3(g->playerRight);
        g->playerRight = vec3FromVec4(right);

        vec4 up = rot * vec4FromVec3(g->playerUp);
        g->playerUp = vec3FromVec4(up);
      } break;
    case ROT_RIGHT:
      {
        yawFactor = -1.0f;

        rot = HMM_Rotate(90.0f, g->playerForward);

        vec4 right = rot * vec4FromVec3(g->playerRight);
        g->playerRight = vec3FromVec4(right);

        vec4 up = rot * vec4FromVec3(g->playerUp);
        g->playerUp = vec3FromVec4(up);
      } break;
    default:
      {
        logln("ERROR: unknown rotation direction");

        return;
      } break;
  }

  logln("old:");
  printFace(g->currentFace);
  printRot(direction);

  g->playerRotation = g->playerRotation * rot;
  g->currentFace = faceRotationMap[g->currentFace][direction];

  g->rotating = true;
  g->rotatingStart = g->cameraRotation;
  g->rotatingStartTime = getTime();
  g->rotatingPitchEnd = 90.0f * pitchFactor;
  g->rotatingYawEnd = 90.0f * yawFactor;
  g->rotatingRollEnd = 0.0f;

  logln("new:");
  printFace(g->currentFace);
  printRot(direction);
}

void gameState_spawnNecessaryPastPlayers(GameState *g, TimeBox* tb, TimeIndex* worldIndex, MemoryArena *ma) {
  gameState_disableAllPastPlayers(g);

  TimeIndex index = *worldIndex;
  index.timeDoneTo = -1;

  Action spawnAction;
  while (timeBox_nextAction(tb, ma, &index, &spawnAction)) {
    if (spawnAction.type != SPAWN || spawnAction.common.jumpID == index.jumpID) {
      continue;
    }

    bool unresolved = true;

    Action lastMove = spawnAction;
    Action jumpAction;
    uint64 seq = spawnAction.common.sequence + 1;
    while (timeBox_actionInSequence(tb, ma, seq, &jumpAction)) {
      seq += 1;

      if (jumpAction.type == MOVE) {
        lastMove = jumpAction;
      }

      if (jumpAction.common.time > index.time) {
        break;
      }

      if (jumpAction.type == JUMP) {
        unresolved = false;

        break;
      }
    }

    if (unresolved) {
      logln("SPAWNING PAST PLAYER!");
      int id = gameState_getFirstPastPlayer(g);
      if (id < 0) {
        logln("WARNING: too many active past players.");

        continue;
      }

      pastPlayer_init(&g->pastPlayers[id], tb, worldIndex, ma, lastMove);
    }
  }
}

void gameState_timeJump(GameState *g, int64 destination) {
  TimeBox* tb = &g->timeBox;
  TimeIndex* index = &g->timeIndex;
  MemoryArena* ma = &lornockData->actionsArena;

  timeBox_add(tb, index, ma, action_makeJump(destination));
  index->jumpID += 1;

  timeBox_save(tb, *index, ma);
  timeBox_load(tb, index, ma, "simple");

  // TODO(harrison): what happens if the save or load fails?

  index->time = index->timeDoneTo = destination;

  timeBox_add(tb, index, ma, action_makeSpawn(g->playerPos));
  timeBox_save(tb, *index, ma);

  gameState_spawnNecessaryPastPlayers(g, tb, index, ma);
}

void gameState_update(State *state) {
  GameState* g = (GameState*) state->memory;
  TimeBox* tb = &g->timeBox;

  if (getTime() > g->timeBoxNextTickTime) {
    g->timeBoxNextTickTime = getTime() + TIME_BOX_TICK_MS_INTERVAL;

    Action a;
    while (timeBox_nextAction(tb, &lornockData->actionsArena, &g->timeIndex, &a)) {
      switch (a.type) {
        case SPAWN:
          {
            logln("SPAWNING!");
            int id = gameState_getFirstPastPlayer(g);
            if (id < 0) {
              logln("WARNING: too many active past players!");

              continue;
            }

            pastPlayer_init(&g->pastPlayers[id], tb, &g->timeIndex, &lornockData->actionsArena, a);
          } break;
        default:
          {
          } break;
      }
    }

    g->timeIndex.time += 1;
  }

  for (int i = 0; i < MAX_PAST_PLAYERS; i++) {
    if (!g->pastPlayers[i].exists) {
      continue;
    }

    pastPlayer_update(&g->pastPlayers[i], tb, &g->timeIndex, &lornockData->actionsArena);
  }

  gameState_setFaceDirections(g);

  if (keyJustDown(KEY_tab)) {
    gameState_timeJump(g, 2);
  }

  if (keyJustDown(KEY_l)) {
    logln("saving!");
    timeBox_save(tb, g->timeIndex, &lornockData->actionsArena);
  }

  if (g->rotating) {
    real32 pc = 1.0f - ((float) ((g->rotatingStartTime + ROTATION_DURATION) - getTime())) / ROTATION_DURATION;

    if (getTime() > g->rotatingStartTime + ROTATION_DURATION) {
      pc = 1.0f;
      g->rotating = false;
    }

    pc = pc * pc;

    g->cameraRotation = quatFromPitchYawRoll(
        g->rotatingPitchEnd * pc,
        g->rotatingYawEnd * pc,
        g->rotatingRollEnd * pc) * g->rotatingStart;

    g->cameraRotation = quatNormalize(g->cameraRotation);
  } else {
    real32 speed = 2.0f;
    real32 dt = getDt();

    vec3 move = vec3_zero;

    if (keyDown(KEY_a)) {
      move -= g->playerRight;
    }

    if (keyDown(KEY_d)) {
      move += g->playerRight;
    }

    if (keyDown(KEY_w)) {
      move += g->playerForward;
    }

    if (keyDown(KEY_s)) {
      move -= g->playerForward;
    }

    g->playerPos += move * dt * speed;

    if (!vec3AlmostEqual(move, g->playerLastMove)) {
      timeBox_add(tb, &g->timeIndex, &lornockData->actionsArena, action_makeMove(g->playerPos));

      g->playerLastMove = move;
    }

    if (keyJustDown(KEY_shift)) {
      vec3 realRight = g->faceRight;
      vec3 realForward = g->faceForward * -1;

      real32 rightLen = vec3Sum(g->playerPos * realRight);
      real32 forwardLen = vec3Sum(g->playerPos * realForward);

      logfln("playerRightlen: %f, forwardlen: %f", rightLen, forwardLen);

      if (fabs(rightLen) > 1.49f) {
        gameState_rotate(g, (rightLen > 0) ? ROT_RIGHT : ROT_LEFT);
      } else if (fabs(forwardLen) > 1.49f) {
        gameState_rotate(g, (forwardLen < 0) ? ROT_FORWARD : ROT_BACKWARD);
      }
    }

    g->playerPos.x = clamp(g->playerPos.x, -1.5f, 1.5f);
    g->playerPos.y = clamp(g->playerPos.y, -1.5f, 1.5f);
    g->playerPos.z = clamp(g->playerPos.z, -1.5f, 1.5f);
  }

  draw_begin(); // Clear buffers, set viewport size, etc.

  draw_3d_begin(70.0f);
  
  // TODO(harrison): refactor into OrbitalCamera struct.
  {
    mat4 view = mat4d(1.0f);
    view = mat4Translate(view, CAMERA_POSITION);
    view = view * quatToMat4(quatFromPitchYawRoll(CAMERA_ROTATION_OFFSET, 0, 0));
    view = view * quatToMat4(g->cameraRotation);

    draw.view = view;
  }

  {
    draw_setShader(shader(SHADER_default));

    vec3 asteroidPosition = vec3(0, 0, 0);
    vec3 asteroidOffset = vec3(-1.5f, -1.5f, -1.5f);

    mat4 model = mat4d(1);
    model = mat4Translate(model, asteroidOffset);
    model = mat4Translate(model, asteroidPosition);

    draw_3d_mesh(g->worldMesh, model, texture(TEXTURE_rock));
  }

  {
    draw_setShader(shader(SHADER_default));

    real32 scale = 0.25f;

    mat4 model = mat4d(1.0f);
    model = mat4Translate(model, g->playerPos);
    model = mat4Translate(model, -1*vec3_one/2*scale);
    model = mat4Scale(model, vec3_one*scale);

    draw_3d_mesh(g->cubeMesh, model, texture(TEXTURE_test));
  }

  {
    draw_setShader(shader(SHADER_default));

    for (int i = 0; i < MAX_PAST_PLAYERS; i++) {
      PastPlayer pp = g->pastPlayers[i];

      if (!pp.exists) {
        continue;
      }

      real32 scale = 0.25f;

      mat4 model = mat4d(1.0f);
      model = mat4Translate(model, pp.pos);
      model = mat4Translate(model, -1*vec3_one/2*scale);
      model = mat4Scale(model, vec3_one*scale);

      draw_3d_mesh(g->cubeMesh, model, texture(TEXTURE_test));
    }
  }
}
