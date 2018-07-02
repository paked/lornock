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

// Rotation states
enum {
  ROT_IDLE,
  ROT_FORWARD,
  ROT_BACKWARD,
  ROT_LEFT,
  ROT_RIGHT
};

#define ROTATION_DURATION 500

struct GameState {
  uint8 world[WORLD_HEIGHT][WORLD_DEPTH][WORLD_WIDTH];
  uint64 vertCount;

  int64 progress;

  GLuint VAO, VBO, EBO;

  quat cameraRotation;

  vec3 rotFBUp;
  vec3 rotLRUp;
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

  g->rotFBUp = vec3(1, 0, 0);
  g->rotLRUp = vec3(0, 1, 0);

  g->rotState = ROT_IDLE;
  g->rotStartTime = timeNow();

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

  g->progress = 0;

  // Asset requests
  assetsRequestShader(SHADER_default);
  assetsRequestTexture(TEXTURE_test);
}

void gameStateUpdate(State* state) {
  GameState* g = (GameState*) state->memory;

  /* updating */

  int32 rotDir = 1;
  real32 rotation = 0;

  vec3 up = vec3(0.0f, 0.0f, 0.0f);

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
          g->rotState = ROT_LEFT;

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
        up = g->rotFBUp;
        rotDir = 1;
      } break;
    case ROT_BACKWARD:
      {
        up = g->rotFBUp;
        rotDir = -1;
      } break;
    case ROT_LEFT:
      {
        up = g->rotLRUp;
        rotDir = 1;
      } break;
    case ROT_RIGHT:
      {
        up = g->rotLRUp;
        rotDir = -1;
      } break;
  }

  if (!g->rotState == ROT_IDLE) {
    if (g->rotStartTime + ROTATION_DURATION < timeNow()) {
      g->rotState = ROT_IDLE;

      /*

      vec4 fb4 = vec4(g->rotFBUp.x, g->rotFBUp.y, g->rotFBUp.z, 0);
      vec4 lr4 = vec4(g->rotLRUp.x, g->rotLRUp.y, g->rotLRUp.z, 0);

      vec4 fb = vec4Normalize(mat4Rotate(mat4d(1.0f), deg2Rad(90 * rotDir), up) * fb4);
      g->rotFBUp = vec3(fb.x, fb.y, fb.z);

      vec4 lr = vec4Normalize(mat4Rotate(mat4d(1.0f), deg2Rad(90 * rotDir), up) * lr4);
      g->rotLRUp = vec3(lr.x, lr.y, lr.z);

      logfln("%f %f %f", g->rotLRUp.x, g->rotLRUp.y, g->rotLRUp.z);*/
    } else {
      real32 start = 0;
      real32 end = 90;

      real32 pc = 1.0f - ((float) ((g->rotStartTime + ROTATION_DURATION) - timeNow())) / ROTATION_DURATION;
      logfln("%f", pc);

      rotation = pc * end;

      quat rot = quatFromAxisAngle(up, deg2Rad(rotation * rotDir));

      g->cameraRotation = g->rotStart * rot;
    }
  }

  /*
  quat diff = quatFromAxisAngle(vec3(0, 1, 0), deg2Rad(10.f));
  g->cameraRotation = quatFromAxisAngle(vec3(1, 0, 0), deg2Rad(rotation)) * diff;*/

  /* drawing */

  glClearColor(0.0f, 0.58f, 0.93f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(shader(SHADER_default).id);

  vec3 asteroidPosition = vec3(0, 0, 0);
  vec3 asteroidOffset = vec3(-1.5f, -1.5f, -1.5f);

  vec3 cameraPosition = vec3(0.0f, 0.0f, -7.0f);

  mat4 model = mat4d(1);
  model = mat4Translate(model, asteroidPosition + asteroidOffset);

  vec3 dir = vec3Normalize(cameraPosition - asteroidPosition);

  mat4 rotMat4 = quatToMat4(g->cameraRotation);

  mat4 view = mat4d(1);
  real32 deg;
  // TODO(harrison): replace with: https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Arcball
  view = mat4Translate(view, cameraPosition);
  view = view * rotMat4;

  mat4 projection = mat4Perspective(70.0f, 640.0f/480.0f, 0.1f, 10000.0f);

  shaderSetMatrix(&shader(SHADER_default), "model", model);
  shaderSetMatrix(&shader(SHADER_default), "view", view);
  shaderSetMatrix(&shader(SHADER_default), "projection", projection);

  glBindTexture(GL_TEXTURE_2D, texture(TEXTURE_test).id);
  glBindVertexArray(g->VAO);
  glDrawArrays(GL_TRIANGLES, 0, g->vertCount);

  g->progress++;
}
