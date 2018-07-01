// External libraries
// NONE

// Vendored libraries
#include <glad/glad.h>

#define HANDMADE_MATH_IMPLEMENTATION
#include <HandmadeMath.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include <stb_image.h>

// Platform common
#include <platform_common.hpp>

Platform* platform = 0;

// Lornock code
#include <hmm_wrapper.cpp>
#include <assets.cpp>
#include <lornock_data.cpp>

uint64 faceLength = 30;

enum {
  BACK,
  FRONT,
  LEFT,
  RIGHT,
  BOTTOM,
  TOP,
  MAX_FACE
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

// NOTE(harrison): init is ran every time the DLL is loaded. It should not set
// any state, as we want state to persist between hot reloads.
extern "C" int lornockInit(Platform* p) {
  platform = p;

  if (!gladLoadGLLoader(p->glLoadProc)) {
    logln("ERROR: Could not load glad");

    return 1;
  }

  return 0;
}

extern "C" void lornockUpdate(LornockMemory* m) {
  dbg_assert(sizeof(m->permanentStorage) < m->permanentStorageSize);

  lornockData = (LornockData*) m->permanentStorage;

  if (!m->initialized) {
    // Asset requests
    assetsRequestShader(SHADER_default);
    assetsRequestTexture(TEXTURE_test);

    // Refresh assets
    updateAssets();

    // TODO: refactor into drawInit or something
    glEnable(GL_DEPTH_TEST);
    // TODO(harrison): pull from platform layer
    glViewport(0, 0, 640, 480);

    // Variables
    lornockData->progress = 0;

    // init world
    for (int y = 0; y < WORLD_HEIGHT; y++) {
      for (int z = 0; z < WORLD_DEPTH; z++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
          lornockData->world[y][z][x] = 1;
        }
      }
    }

    uint64 count = 0;
    real32* verts = (real32*) m->transientStorage;

    for (int y = 0; y < WORLD_HEIGHT; y++) {
      for (int z = 0; z < WORLD_DEPTH; z++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
          if (lornockData->world[y][z][x] != 1) {
            continue;
          }

          vec3 pos = vec3(x, y, z);

          for (int d = 0; d < MAX_FACE; d++) {
            if (!voxelEmptyToThe(d, lornockData->world, x, y, z)) {
              continue;
            }

            addFaceToMesh(d, verts, &count, pos);
          }
        }
      }
    }

    lornockData->vertCount = count;

    // Init VBO, EBO, and VAO.
    GLuint VBO, EBO, VAO;
    glGenVertexArrays(1, &VAO);

    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts) * count, verts, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    lornockData->VAO = VAO;
    lornockData->VBO = VBO;

    m->initialized = true;
  }

  glClearColor(0.0f, 0.58f, 0.93f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(shader(SHADER_default).id);

  mat4 model = mat4d(1);
  model = mat4Scale(model, vec3(4, 4, 4));
  mat4 view = mat4d(1);
  view = mat4Translate(view, vec3(-1.5f, -2.0f, -(1.5 + lornockData->progress/60/2)));
  mat4 projection = mat4Perspective(70.0f, 640.0f/480.0f, 0.1f, 10000.0f);

  shaderSetMatrix(&shader(SHADER_default), "model", model);
  shaderSetMatrix(&shader(SHADER_default), "view", view);
  shaderSetMatrix(&shader(SHADER_default), "projection", projection);

  glBindTexture(GL_TEXTURE_2D, texture(TEXTURE_test).id);
  glBindVertexArray(lornockData->VAO);
  glDrawArrays(GL_TRIANGLES, 0, lornockData->vertCount);

  lornockData->progress++;
}
