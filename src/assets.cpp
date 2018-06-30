enum {
#define shader(name) SHADER_ ##name,
#include <assets_shaders.cpp>
#undef shader
  MAX_SHADER
};

const char* shaderFilenames[MAX_SHADER] = {
#define shader(name) #name
#include <assets_shaders.cpp>
#undef shader
};

const char* shaderFilename(uint32 i) {
  dbg_assert(i >= 0 && i < MAX_SHADER);

  return shaderFilenames[i];
}

struct Shader {
  GLuint id;
};

struct Assets {
  int32 shaderRequests[MAX_SHADER];
  Shader shaders[MAX_SHADER];
};

Shader shaderInit(void* vert, uint32 vertLen, void* frag, uint32 fragLen) {
  Shader s = {0};

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

  s.id = programID;

  return s;
}

Shader shaderLoad(const char* name) {
  Shader s = {0};

  void* vertData;
  uint32 vertLen;
  void* fragData;
  uint32 fragLen;

  // FIXME(harrison): size these arrays based on the max-size of the string, not 64.
  char vertFilename[64];
  char fragFilename[64];

  snprintf(vertFilename, 64, "data/shaders/%s.vert", name);
  snprintf(fragFilename, 64, "data/shaders/%s.frag", name);

  loadFromFile(vertFilename, &vertData, &vertLen);
  loadFromFile(fragFilename, &fragData, &fragLen);

  s = shaderInit(vertData, vertLen, fragData, fragLen);
}

void shaderClean(Shader* shader) {
  glDeleteProgram(shader->id);
  shader->id = 0;
}
