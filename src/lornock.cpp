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

    real32 referenceCube[] = cubeMesh;

    for (int y = 0; y < WORLD_HEIGHT; y++) {
      for (int z = 0; z < WORLD_DEPTH; z++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
          if (lornockData->world[y][z][x] != 1) {
            continue;
          }

          vec3 offset = vec3(x, y, z);

          for (int d = 0; d < MAX_FACE; d++) {
            uint64 start = d * 30;
            uint64 end = start + 30;

            for (uint64 i = start; i < end; i++) {
              real32 v = referenceCube[i];

              if (i % 5 < 3) {
                v += offset[i % 5];
              }

              verts[count] = v;

              count++;
            }
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
  model = mat4Rotate(model, 30, vec3(1.0f, 0.0f, 0.0f));
  mat4 view = mat4d(1);
  view = mat4Translate(view, vec3(-1.5f, 0.0f, -8.0f));
  mat4 projection = mat4Perspective(70.0f, 640.0f/480.0f, 0.1f, 10000.0f);

  shaderSetMatrix(&shader(SHADER_default), "model", model);
  shaderSetMatrix(&shader(SHADER_default), "view", view);
  shaderSetMatrix(&shader(SHADER_default), "projection", projection);

  glBindTexture(GL_TEXTURE_2D, texture(TEXTURE_test).id);
  glBindVertexArray(lornockData->VAO);
  glDrawArrays(GL_TRIANGLES, 0, lornockData->vertCount);
}
