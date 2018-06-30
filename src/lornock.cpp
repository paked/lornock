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
  lornockData = (LornockData*) m->permanentStorage;

  if (!m->initialized) {
    // Asset requests
    assetsRequestShader(SHADER_default);

    // Refresh assets
    updateAssets();

    // TODO: refactor into drawInit or something
    glEnable(GL_DEPTH_TEST);
    // TODO(harrison): pull from platform layer
    glViewport(0, 0, 640, 480);

    float vertices[] = cubeMesh;

    // Init VBO, EBO, and VAO.
    GLuint VBO, EBO, VAO;
    glGenVertexArrays(1, &VAO);

    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

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
  view = mat4Translate(view, vec3(0.0f, 0.0f, -2.0f));
  mat4 projection = mat4Perspective(70.0f, 640.0f/480.0f, 0.1f, 10000.0f);

  shaderSetMatrix(&shader(SHADER_default), "model", model);
  shaderSetMatrix(&shader(SHADER_default), "view", view);
  shaderSetMatrix(&shader(SHADER_default), "projection", projection);

  glBindVertexArray(lornockData->VAO);
  glDrawArrays(GL_TRIANGLES, 0, 36);
}
