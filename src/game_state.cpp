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

#define ROTATION_DURATION 500
enum {
  ROT_FORWARD,
  ROT_BACKWARD,
  ROT_LEFT,
  ROT_RIGHT,
  ROT_IDLE
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

struct GameState {
  uint8 world[WORLD_HEIGHT][WORLD_DEPTH][WORLD_WIDTH];
  uint64 vertCount;

  GLuint VAO, VBO, EBO;

  uint32 currentFace;

  quat cameraRotation;

  quat rotStart;
  uint32 rotState;
  uint32 rotStartTime;
};

uint64 faceLength = 30;

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
  real32 cube[] = cubeMesh;

  uint64 start = d * faceLength;
  uint64 end = start + faceLength;

  for (uint64 i = start; i < end; i++) {
    real32 v = cube[i];

    if (i % 5 < 3) {
      v += offset[i % 5];
    }

    verts[*len] = v;

    *len += 1;
  }
}

void gameStateInit(State* state) {
  LornockMemory* m = lornockMemory;
  GameState* g = (GameState*) state->memory;

  g->cameraRotation = quatFromAxisAngle(vec3(1, 0, 0), 0);

  g->rotState = ROT_IDLE;
  g->rotStartTime = timeNow();

  g->currentFace = FRONT;

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

  g->vertCount = count;

  // Init VBO, EBO, and VAO.
  GLuint VBO, EBO, VAO;
  glGenVertexArrays(1, &VAO);

  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(real32) * count, verts, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  g->VAO = VAO;
  g->VBO = VBO;

  // Asset requests
  assetsRequestShader(SHADER_default);
  assetsRequestTexture(TEXTURE_test);
}

void gameStateUpdate(State* state) {
  GameState* g = (GameState*) state->memory;

  /* updating */

  real32 rollFactor = 0;
  real32 pitchFactor = 0;
  real32 yawFactor = 0;

  switch (g->rotState) {
    case ROT_IDLE:
      {
        bool starting = false;
        if (keyJustDown(KEY_w)) {
          g->rotState = ROT_FORWARD;

          starting = true;
        } else if (keyJustDown(KEY_s)) {
          g->rotState = ROT_BACKWARD;

          starting = true;
        } else if (keyJustDown(KEY_a)) {
          g->rotState = ROT_LEFT;
          
          starting = true;
        } else if (keyJustDown(KEY_d)) {
          g->rotState = ROT_RIGHT;

          starting = true;
        }

        if (starting) {
          g->rotStart = g->cameraRotation;
          g->rotStartTime = timeNow();       
        }

        break;
      }
    case ROT_FORWARD:
      {
        rollFactor = 1.0;
      } break;
    case ROT_BACKWARD:
      {
        rollFactor = -1.0;
      } break;
    case ROT_LEFT:
      {
        pitchFactor = 1.0;
      } break;
    case ROT_RIGHT:
      {
        pitchFactor = -1.0;
      } break;
  }

  if (g->rotState != ROT_IDLE) {
    real32 pc = 1.0f - ((float) ((g->rotStartTime + ROTATION_DURATION) - timeNow())) / ROTATION_DURATION;

    if (timeNow() > g->rotStartTime + ROTATION_DURATION) {
      pc = 1.0f;

      printFace(g->currentFace);
      g->currentFace = faceRotationMap[g->currentFace][g->rotState];
      printFace(g->currentFace);

      g->rotState = ROT_IDLE;
    }

    real32 amount = deg2Rad(90) * pc;

    quat deltaRot = quatFromPitchYawRoll(rollFactor * amount, pitchFactor * amount, yawFactor * amount);

    g->cameraRotation = deltaRot * g->rotStart;
    g->cameraRotation = quatNormalize(g->cameraRotation);
  }

  /* drawing */

  glClearColor(0.0f, 0.58f, 0.93f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(shader(SHADER_default).id);

  vec3 asteroidPosition = vec3(0, 0, 0);
  vec3 asteroidOffset = vec3(-1.5f, -1.5f, -1.5f);
  vec3 cameraPosition = vec3(0.0f, 0.0f, -7.0f);

  mat4 model = mat4d(1);
  model = mat4Translate(model, asteroidPosition + asteroidOffset);

  mat4 view = mat4d(1);
  view = mat4Translate(view, cameraPosition);
  view = view * quatToMat4(g->cameraRotation);

  mat4 projection = mat4Perspective(70.0f, 640.0f/480.0f, 0.1f, 10000.0f);

  shaderSetMatrix(&shader(SHADER_default), "model", model);
  shaderSetMatrix(&shader(SHADER_default), "view", view);
  shaderSetMatrix(&shader(SHADER_default), "projection", projection);

  glBindTexture(GL_TEXTURE_2D, texture(TEXTURE_test).id);
  glBindVertexArray(g->VAO);
  glDrawArrays(GL_TRIANGLES, 0, g->vertCount);
}
