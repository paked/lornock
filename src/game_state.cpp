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
  GLuint worldVAO;
  uint32 worldVertCount;

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

  GLuint worldVAO, worldVBO;
  glGenVertexArrays(1, &worldVAO);

  glGenBuffers(1, &worldVBO);

  glBindVertexArray(worldVAO);

  glBindBuffer(GL_ARRAY_BUFFER, worldVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(real32) * count, verts, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  g->worldVAO = worldVAO;
  g->worldVertCount = count;

  // Cube
  real32 cubeData[] = CUBE_MESH_DATA;

  GLuint cubeVAO, cubeVBO;

  glGenVertexArrays(1, &cubeVAO);
  glBindVertexArray(cubeVAO);

  glGenBuffers(1, &cubeVBO);

  glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeData), cubeData, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  g->cubeVAO = cubeVAO;

  assetsRequestShader(SHADER_default);
  assetsRequestTexture(TEXTURE_test);
  assetsRequestTexture(TEXTURE_player);
  assetsRequestTexture(TEXTURE_rock);
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

  glClearColor(18.0f/255, 26.0f/255, 47.0f/255, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  mat4 projection = mat4Perspective(70.0f, (real32) getWindowWidth() / (real32) getWindowHeight(), 0.1f, 1000.0f);

  quat offset = quatFromPitchYawRoll(CAMERA_ROTATION_OFFSET, 0, 0);

  mat4 view = mat4d(1.0f);
  view = mat4Translate(view, CAMERA_POSITION);
  view = view * quatToMat4(offset);
  view = view * quatToMat4(g->cameraRotation);

  {
    Shader s = shader(SHADER_default);
    Texture t = texture(TEXTURE_test);

    glUseProgram(s.id);

    real32 scale = 0.25f;
    mat4 model = mat4d(1.0f);
    model = mat4Translate(model, g->playerPos);
    model = mat4Translate(model, -1*vec3_one/2*scale);
    model = model * g->playerRotation;
    model = mat4Scale(model, vec3_one*scale);

    shaderSetMatrix(&s, "model", model);
    shaderSetMatrix(&s, "view", view);
    shaderSetMatrix(&s, "projection", projection);

    glBindTexture(GL_TEXTURE_2D, t.id);
    glBindVertexArray(g->cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }

  {
    Shader s = shader(SHADER_default);
    Texture t = texture(TEXTURE_rock);

    vec3 asteroidPosition = vec3(0, 0, 0);
    vec3 asteroidOffset = vec3(-1.5f, -1.5f, -1.5f);

    mat4 model = mat4d(1);
    model = mat4Translate(model, asteroidPosition + asteroidOffset);

    glUseProgram(s.id);

    shaderSetMatrix(&shader(SHADER_default), "model", model);
    shaderSetMatrix(&shader(SHADER_default), "view", view);
    shaderSetMatrix(&shader(SHADER_default), "projection", projection);

    glBindTexture(GL_TEXTURE_2D, t.id);
    glBindVertexArray(g->worldVAO);
    glDrawArrays(GL_TRIANGLES, 0, g->worldVertCount);
  }
}

// TODO(harrison): Abstract OpenGL out of the state update loop. Create a `draw.cpp` file with immediate-gui-ish API
// TODO(harrison): Create "custom allocator" which can be used by the places in our game to reserve memory. Let's call it a MemoryZone. Should have two zones to start with: 1. Game memory, 2. Frame memory. Game memory should give us a place to read assets into, store action chunks, and do other things. Frame memory should be a place to store data used in calculations. It will be freed at the end of each frame. Should essentially be a linked list of MemoryBlock's, which can be co-opted at any-time and then potentially tagged with certain information (ie. CACHE which would mean that it will be destroyed when the game reaches a certain memory limit, and then re-generated at runtime when need be.
