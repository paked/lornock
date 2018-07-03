enum {
#define shader(name) SHADER_ ##name,
#include <assets_shaders.cpp>
#undef shader
  MAX_SHADER
};

enum {
#define texture(name) TEXTURE_ ##name,
#include <assets_textures.cpp>
  MAX_TEXTURE
#undef texture
};

const char* shaderFilenames[MAX_SHADER] = {
#define shader(name) #name,
#include <assets_shaders.cpp>
#undef shader
};

const char* shaderFilename(uint32 i) {
  dbg_assert(i >= 0 && i < MAX_SHADER);

  return shaderFilenames[i];
}

const char* textureFilenames[MAX_TEXTURE] = {
#define texture(name) #name,
#include <assets_textures.cpp>
#undef texture
};

const char* textureFilename(uint32 i) {
  dbg_assert(i >= 0 && i < MAX_TEXTURE);

  return textureFilenames[i];
}

struct Shader {
  GLuint id;
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

bool shaderSetMatrix(Shader* shader, const char* name, mat4 m) {
  GLint loc = glGetUniformLocation(shader->id, name);
  if (loc == -1) {
    logfln("ERROR: find uniform location %s", name);

    return false;
  }

  glUniformMatrix4fv(loc, 1, GL_FALSE, &m.Elements[0][0]);

  return true;
}

bool shaderSetVec3(Shader* shader, const char* name, vec3 v) {
  GLint loc = glGetUniformLocation(shader->id, name);
  if (loc == -1) {
    logfln("ERROR: find uniform location %s", name);

    return false;
  }

  glUniform3f(loc, v.x, v.y, v.z);

  return true;
}

bool shaderSetVec2(Shader* shader, const char* name, vec2 v) {
  GLint loc = glGetUniformLocation(shader->id, name);
  if (loc == -1) {
    logfln("ERROR: find uniform location %s", name);

    return false;
  }

  glUniform2f(loc, v.x, v.y);

  return true;
}

void shaderClean(Shader* shader) {
  glDeleteProgram(shader->id);
  shader->id = 0;
}

struct Texture {
  GLuint id;

  uint32 w;
  uint32 h;
};

Texture textureInit(void* data, uint32 len) {
  Texture t;

  int w, h;
  uint8 *tex_data = stbi_load_from_memory((unsigned char *)data, (int)len, &w, &h, 0, STBI_rgb_alpha);

  if (!data) {
    logln("Data has not been loaded");
  }

  t.w = (int16)w;
  t.h = (int16)h;

  glGenTextures(1, &t.id);
  glBindTexture(GL_TEXTURE_2D, t.id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t.w, t.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

  free(tex_data);

  return t;
}

Texture textureLoad(const char* name) {
  char textureFilename[64];
  snprintf(textureFilename, 64, "data/img/%s.png", name);

  void* data;
  uint32 dataLen;

  loadFromFile(textureFilename, &data, &dataLen);

  return textureInit(data, dataLen);
}

void textureClean(Texture* tex) {
  glDeleteTextures(1, &tex->id);
  tex->id = 0;
}

struct Assets {
  int32 shaderRequests[MAX_SHADER];
  Shader shaders[MAX_SHADER];

  int32 textureRequests[MAX_TEXTURE];
  Texture textures[MAX_TEXTURE];
};
