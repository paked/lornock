// External libraries
// NONE

// Vendored libraries
#include <glad/glad.h>

#define HANDMADE_MATH_IMPLEMENTATION
#include <HandmadeMath.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include <stb_image.h>

// Lornock code
#include <platform_common.hpp>

Platform* platform = 0;

struct Shader {
  GLuint id;
};

struct LornockData {
  int number;

  hmm_vec2 vec;

  Shader defaultShader;

  GLuint VAO, VBO, EBO;
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
  LornockData* lornockData = (LornockData*) m->permanentStorage;
  if (!m->initialized) {
    lornockData->number = 42;

    lornockData->vec.x = 22;
    lornockData->vec.y = 23;

    glEnable(GL_DEPTH_TEST);
    // TODO(harrison): pull from platform layer
    glViewport(0, 0, 640, 480);

    void* vert;
    uint32 vertLen;
    void* frag;
    uint32 fragLen;

    loadFromFile("data/shaders/default.vert", &vert, &vertLen);
    loadFromFile("data/shaders/default.frag", &frag, &fragLen);

    // Create shaders
    GLuint vertShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    bool failed = false;
    GLint codeLen = 0;
    GLint errorLogLen = 0;

    {
      codeLen = (GLint) vertLen;
      const char* code = (char*) vert;

      logln("INFO: compiling vert shader");

      glShaderSource(vertShaderID, 1, &code, 0);
      glCompileShader(vertShaderID);
    }

    {
      GLint res;
      glGetShaderiv(vertShaderID, GL_COMPILE_STATUS, &res);

      failed |= res == GL_FALSE;

      glGetShaderiv(vertShaderID, GL_INFO_LOG_LENGTH, &errorLogLen);
      if (errorLogLen) {
        char errorLog[errorLogLen]; 
        glGetShaderInfoLog(vertShaderID, errorLogLen, 0, errorLog);

        log("ERROR: Could not compile vertex shader: %s", errorLog);
      }
    }

    {
      codeLen = (GLint) fragLen;
      const char* code = (char*) frag;

      logln("INFO: compiling frag shader");

      glShaderSource(fragShaderID, 1, &code, 0);
      glCompileShader(fragShaderID);
    }

    {
      GLint res;
      glGetShaderiv(fragShaderID, GL_COMPILE_STATUS, &res);

      failed |= res == GL_FALSE;

      glGetShaderiv(fragShaderID, GL_INFO_LOG_LENGTH, &errorLogLen);

      if (errorLogLen) {
        char errorLog[errorLogLen]; 
        glGetShaderInfoLog(vertShaderID, errorLogLen, 0, errorLog);

        log("ERROR: Could not compile fragment shader: %s", errorLog);
      }
    }

    if (failed) {
      logln("ERROR: could not load shader!");
    }

    GLuint programID = glCreateProgram();
    glAttachShader(programID, vertShaderID);
    glAttachShader(programID, fragShaderID);
    glLinkProgram(programID);

    {
      GLint res;
      glGetProgramiv(programID, GL_LINK_STATUS, &res);

      failed = res == GL_FALSE;

      glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &errorLogLen);

      if (errorLogLen) {
        char errorLog[errorLogLen]; 
        glGetProgramInfoLog(programID, errorLogLen, 0, errorLog);

        log("ERROR: Could not link shader program: %s", errorLog);
      }
    }

    if (failed) {
      logln("ERROR: Could not link shader");
    }
    glDeleteShader(fragShaderID);
    glDeleteShader(vertShaderID);

    lornockData->defaultShader.id = programID;

    // Init VBO, EBO, and VAO.
    GLuint VAO, VBO, EBO;
    float vertices[] = {
      0.5f,  0.5f, 0.0f,  // top right
      0.5f, -0.5f, 0.0f,  // bottom right
      -0.5f, -0.5f, 0.0f,  // bottom left
      -0.5f,  0.5f, 0.0f   // top left 
    };

    unsigned int indices[] = {  // note that we start from 0!
      0, 1, 3,  // first Triangle
      1, 2, 3   // second Triangle
    };
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    lornockData->VAO = VAO;
    lornockData->VBO = VBO;
    lornockData->EBO = EBO;

    m->initialized = true;
  }

  glClearColor(0.0f, 0.58f, 0.93f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(lornockData->defaultShader.id);
  glBindVertexArray(lornockData->VAO);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
