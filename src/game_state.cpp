#include <entities/actions.cpp>

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
  mat4 playerRotation;

  quat cameraRotation;

  bool rotating;
  uint32 rotatingStartTime;
  real32 rotatingPitchEnd;
  real32 rotatingYawEnd;
  real32 rotatingRollEnd;
  quat rotatingStart;

  uint8 world[WORLD_HEIGHT][WORLD_DEPTH][WORLD_WIDTH];
  Mesh worldMesh;

  Mesh cubeMesh;

  GLuint cubeVAO;
};

void gameState_setFaceDirections(GameState* g) {
  mat4 view = mat4d(1);
  view = mat4Translate(view, CAMERA_POSITION);
  view = view * quatToMat4(g->cameraRotation);

  g->faceRight = vec3(view[0][0], view[1][0], view[2][0]);
  g->faceForward = vec3(view[0][1], view[1][1], view[2][1]);
}

void gameState_init(State* state) {
  {
    Action *a1 = memoryArena_pushStruct(&lornockData->actionsArena, Action);
    Action *a2 = memoryArena_pushStruct(&lornockData->actionsArena, Action);

    *a1 = action_makeSpawn(vec3_one);
    *a2 = action_makeJump(20);
  }

  LornockMemory* m = lornockMemory;
  GameState* g = (GameState*) state->memory;

  g->currentFace = TOP;
  gameState_setFaceDirections(g); // g->faceRight, g->faceForward

  g->playerUp = faceCardinalDirections[g->currentFace][DIRECTION_UP];
  g->playerRight = faceCardinalDirections[g->currentFace][DIRECTION_RIGHT];
  g->playerForward = faceCardinalDirections[g->currentFace][DIRECTION_FORWARD];
  g->playerPos = vec3(0.0f, 1.5f, 0.0f);
  g->playerRotation = mat4d(1.0f);

  g->cameraRotation = quatFromPitchYawRoll(90.0f, 0.0f, 0.0f);

  g->rotating = false;
  g->rotatingStartTime = 0;
  g->rotatingPitchEnd = 0;
  g->rotatingYawEnd = 0;
  g->rotatingRollEnd = 0;
  g->rotatingStart = quatFromPitchYawRoll(0.0f, 0.0f, 0.0f);

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

void gameState_update(State *state) {
  {
    for (MemoryBlock* i = lornockData->actionsArena.first; i != 0; i = i->next) {
      Action* a = (Action*) i->start;
      logln("got action!");
      action_print(*a);
    }
  }

  GameState* g = (GameState*) state->memory;

  gameState_setFaceDirections(g);

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

    if (keyDown(KEY_a)) {
      g->playerPos -= g->playerRight * dt * speed;
    }

    if (keyDown(KEY_d)) {
      g->playerPos += g->playerRight * dt * speed;
    }

    if (keyDown(KEY_w)) {
      g->playerPos += g->playerForward * dt * speed;
    }

    if (keyDown(KEY_s)) {
      g->playerPos -= g->playerForward * dt * speed;
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
}
