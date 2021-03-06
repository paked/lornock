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

enum {
#define model(name) MODEL_ ##name,
#include <assets_models.cpp>
  MAX_MODEL
#undef model
};

enum {
#define font(name) FONT_ ##name,
#include <assets_fonts.cpp>
#undef font
  MAX_FONT
};

const char* shaderFilenames[MAX_SHADER] = {
#define shader(name) #name,
#include <assets_shaders.cpp>
#undef shader
};

const char* shaderFilename(uint32 i) {
  ensure(i >= 0 && i < MAX_SHADER);

  return shaderFilenames[i];
}

const char* textureFilenames[MAX_TEXTURE] = {
#define texture(name) #name,
#include <assets_textures.cpp>
#undef texture
};

const char* textureFilename(uint32 i) {
  ensure(i >= 0 && i < MAX_TEXTURE);

  return textureFilenames[i];
}

const char* modelFilenames[MAX_MODEL] = {
#define model(name) #name,
#include <assets_models.cpp>
#undef model
};

const char* modelFilename(uint32 i) {
  ensure(i >= 0 && i < MAX_MODEL);

  return modelFilenames[i];
}

const char* fontFilenames[MAX_MODEL] = {
#define font(name) #name,
#include <assets_fonts.cpp>
#undef font
};

const char* fontFilename(uint32 i) {
  ensure(i >= 0 && i < MAX_MODEL);

  return fontFilenames[i];
}

struct Shader {
  GLuint id;
};

Shader shader_init(void* vert, uint32 vertLen, void* frag, uint32 fragLen) {
  Shader s = {0};

  // Create shaders
  GLuint vertShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  bool failed = false;
  GLint errorLogLen = 0;

  {
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
      char errorLog[512];
      glGetShaderInfoLog(vertShaderID, errorLogLen, 0, errorLog);

      log("ERROR: Could not compile vertex shader: %s", errorLog);
    }
  }

  {
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
      char errorLog[512]; 
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
      char errorLog[512]; 
      glGetProgramInfoLog(programID, errorLogLen, 0, errorLog);

      log("ERROR: Could not link shader program: %s", errorLog);
    }
  }

  ensure(failed != true);

  glDeleteShader(fragShaderID);
  glDeleteShader(vertShaderID);

  s.id = programID;

  return s;
}

Shader shader_load(const char* name) {
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

  s = shader_init(vertData, vertLen, fragData, fragLen);

  return s;
}

bool shader_setMatrix(Shader* shader, const char* name, mat4 m) {
  GLint loc = glGetUniformLocation(shader->id, name);
  if (loc == -1) {
    logfln("ERROR: cannot find uniform location %s", name);

    return false;
  }

  glUniformMatrix4fv(loc, 1, GL_FALSE, &m.Elements[0][0]);

  return true;
}

bool shader_setVec4(Shader* shader, const char* name, vec4 v) {
  GLint loc = glGetUniformLocation(shader->id, name);
  if (loc == -1) {
    logfln("ERROR: cannot find uniform location %s", name);

    return false;
  }

  glUniform4f(loc, v.x, v.y, v.z, v.w);

  return true;
}

bool shader_setVec3(Shader* shader, const char* name, vec3 v) {
  GLint loc = glGetUniformLocation(shader->id, name);
  if (loc == -1) {
    logfln("ERROR: cannot find uniform location %s", name);

    return false;
  }

  glUniform3f(loc, v.x, v.y, v.z);

  return true;
}

bool shader_setVec2(Shader* shader, const char* name, vec2 v) {
  GLint loc = glGetUniformLocation(shader->id, name);
  if (loc == -1) {
    logfln("ERROR: cannot find uniform location %s", name);

    return false;
  }

  glUniform2f(loc, v.x, v.y);

  return true;
}

bool shader_setInt(Shader* shader, const char* name, int v) {
  GLint loc = glGetUniformLocation(shader->id, name);
  if (loc == -1) {
    logfln("ERROR: cannot find uniform location %s", name);

    return false;
  }

  glUniform1i(loc, v);

  return true;
}

bool shader_setFloat(Shader* shader, const char* name, float v) {
  GLint loc = glGetUniformLocation(shader->id, name);
  if (loc == -1) {
    logfln("ERROR: cannot find uniform location %s", name);

    return false;
  }

  glUniform1f(loc, v);

  return true;
}

void shader_clean(Shader* shader) {
  glDeleteProgram(shader->id);
  shader->id = 0;
}

struct Texture {
  GLuint id;

  uint32 w;
  uint32 h;
};

Texture texture_init(void* data, uint32 len) {
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

  glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, t.w, t.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

  free(tex_data);

  return t;
}

Texture texture_load(const char* name) {
  char textureFilename[64];
  snprintf(textureFilename, 64, "data/img/%s.png", name);

  void* data;
  uint32 dataLen;

  loadFromFile(textureFilename, &data, &dataLen);

  return texture_init(data, dataLen);
}

void texture_clean(Texture* tex) {
  glDeleteTextures(1, &tex->id);
  tex->id = 0;
}

struct Mesh {
  GLuint vao;
  GLuint vbo;

  uint64 vertCount;
  uint64 faceCount;
};

Mesh mesh_init(real32* verts, uint64 count) {
  Mesh m;

  GLuint vao, vbo;
  glGenVertexArrays(1, &vao);

  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(real32) * count, verts, GL_STATIC_DRAW);

  // TODO(harrison): add options for "non-standard" vertex attribute configurations
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  m.vao = vao;
  m.vbo = vbo;
  m.vertCount = count;
  m.faceCount = count/8;

  return m;
}

struct Model {
  GLuint vao;

  GLuint vertexBuffer;
  GLuint uvBuffer;
  GLuint normalBuffer;

  uint64 vertCount;
};

#define MAX_OBJ_VERTICES (24576)

#define MAX_VERTEX_BYTES (MAX_OBJ_VERTICES*sizeof(vec3))
#define MAX_UV_BYTES (MAX_OBJ_VERTICES*sizeof(vec2))
#define MAX_NORMAL_BYTES (MAX_OBJ_VERTICES*sizeof(vec3))

#define MAX_VERTEX_INDEX_BYTES (MAX_OBJ_VERTICES*sizeof(int32))
#define MAX_UV_INDEX_BYTES (MAX_OBJ_VERTICES*sizeof(int32))
#define MAX_NORMAL_INDEX_BYTES (MAX_OBJ_VERTICES*sizeof(int32))

#define MAX_FINAL_VERTEX_FLOATS (MAX_OBJ_VERTICES*3)
#define MAX_FINAL_VERTEX_BYTES (MAX_FINAL_VERTEX_FLOATS*sizeof(real32))

#define MAX_FINAL_UV_FLOATS (MAX_OBJ_VERTICES*2)
#define MAX_FINAL_UV_BYTES (MAX_FINAL_UV_FLOATS*sizeof(real32))

#define MAX_FINAL_NORMAL_FLOATS (MAX_OBJ_VERTICES*3)
#define MAX_FINAL_NORMAL_BYTES (MAX_FINAL_NORMAL_FLOATS*sizeof(real32))

Model model_init(const char* name) {
  Model m = { 0 };

  MemoryArena* temp = tempMemory;

  ensure(MAX_VERTEX_BYTES + MAX_UV_BYTES + MAX_NORMAL_BYTES +
      MAX_VERTEX_INDEX_BYTES + MAX_UV_INDEX_BYTES + MAX_NORMAL_INDEX_BYTES +
      MAX_FINAL_VERTEX_BYTES + MAX_FINAL_UV_BYTES + MAX_FINAL_NORMAL_BYTES
      < temp->size);

  char meshFilename[64];
  snprintf(meshFilename, 64, "data/mesh/%s.obj", name);

  void* rawData;
  uint32 len = 0;

  loadFromFile(meshFilename, &rawData, &len);

  vec3* verts = memoryArena_pushArray(temp, MAX_OBJ_VERTICES, vec3);
  vec2* uvs = memoryArena_pushArray(temp, MAX_OBJ_VERTICES, vec2);
  vec3* normals = memoryArena_pushArray(temp, MAX_OBJ_VERTICES, vec3);

  uint32 vertsCount = 0;
  uint32 normalsCount = 0;
  uint32 uvsCount = 0;

  int32* vertexIndex = memoryArena_pushArray(temp, MAX_OBJ_VERTICES, int32);
  int32* uvIndex = memoryArena_pushArray(temp, MAX_OBJ_VERTICES, int32);
  int32* normalIndex = memoryArena_pushArray(temp, MAX_OBJ_VERTICES, int32);

  uint32 faceCount = 0;

  uint32 vertIndexCount = 0;
  uint32 uvIndexCount = 0;
  uint32 normalIndexCount = 0;

  real32* finalVerts = memoryArena_pushArray(temp, MAX_FINAL_VERTEX_FLOATS, real32);
  real32* finalUVs = memoryArena_pushArray(temp, MAX_FINAL_UV_FLOATS, real32);
  real32* finalNormals = memoryArena_pushArray(temp, MAX_FINAL_NORMAL_FLOATS, real32);

  uint32 finalVertCount = 0;
  uint32 finalUVCount = 0;
  uint32 finalNormalCount = 0;

  uint32 lineLen = 0;
  char* line = memoryArena_pushArray(temp, 1024, char);

  char* data = (char*) rawData;

  uint32 elementLen = 1024;
  char* element = memoryArena_pushArray(temp, elementLen, char);

  uint32 head = 0;
  while (head < len) {
    getLine(line, &lineLen, &head, data, len);
    head += 1;

    // like head, but for the line.
    uint32 tip = 0;

    eatUntilWhitespace(element, elementLen, line, lineLen, &tip);

    if (element[0] == '#') {
      logln("found comment, skipping");

      continue;
    } else if (strcmp("v", element) == 0) {
      vec3 vert;

      int count = sscanf(line, "v %f %f %f", &vert.x, &vert.y, &vert.z);

      ensure(count == 3);

      verts[vertsCount] = vert;

      vertsCount += 1;

      continue;
    } else if (strcmp("vn", element) == 0) {
      vec3 normal;

      int count = sscanf(line, "vn %f %f %f", &normal.x, &normal.y, &normal.z);
      ensure(count == 3);

      normals[normalsCount] = normal;

      normalsCount += 1;

      continue;
    } else if (strcmp("vt", element) == 0) {
      vec2 uv;

      int count = sscanf(line, "vt %f %f", &uv.x, &uv.y);
      ensure(count == 2);

      uvs[uvsCount] = uv;

      uvsCount += 1;

      continue;
    } else if (strcmp("f", element) == 0) {
      int32 v1V;
      int32 v1VT;
      int32 v1VN;

      int32 v2V;
      int32 v2VT;
      int32 v2VN;

      int32 v3V;
      int32 v3VT;
      int32 v3VN;

      int count = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
          &v1V, &v1VT, &v1VN,
          &v2V, &v2VT, &v2VN,
          &v3V, &v3VT, &v3VN);

      ensure(count == 9);

      // populate v1, v2, v3

      vertexIndex[vertIndexCount] = v1V;
      uvIndex[uvIndexCount] = v1VT;
      normalIndex[normalIndexCount] = v1VN;

      vertIndexCount += 1;
      uvIndexCount += 1;
      normalIndexCount += 1;

      vertexIndex[vertIndexCount] = v2V;
      uvIndex[uvIndexCount] = v2VT;
      normalIndex[normalIndexCount] = v2VN;

      vertIndexCount += 1;
      uvIndexCount += 1;
      normalIndexCount += 1;

      vertexIndex[vertIndexCount] = v3V;
      uvIndex[uvIndexCount] = v3VT;
      normalIndex[normalIndexCount] = v3VN;

      vertIndexCount += 1;
      uvIndexCount += 1;
      normalIndexCount += 1;

      faceCount += 1;

      continue;
    }

    logfln("found: '%s'. don't know what to do with it", line);
  }

  logfln("processed %d verts, %d normals, %d uvs, %d faces", vertsCount, normalsCount, uvsCount, faceCount);

  for (uint32 i = 0; i < vertIndexCount; i++) {
    int32 index = vertexIndex[i] - 1;

    vec3 vert = verts[index];

    finalVerts[finalVertCount] = vert.x;
    finalVertCount += 1;
    finalVerts[finalVertCount] = vert.y;
    finalVertCount += 1;
    finalVerts[finalVertCount] = vert.z;
    finalVertCount += 1;
  }

  for (uint32 i = 0; i < uvIndexCount; i++) {
    int32 index = uvIndex[i] - 1;

    vec2 uv = uvs[index];
    finalUVs[finalUVCount] = uv.x;
    finalUVCount += 1;
    finalUVs[finalUVCount] = uv.y;
    finalUVCount += 1;
  }

  for (uint32 i = 0; i < normalIndexCount; i++) {
    int32 index = normalIndex[i] - 1;

    vec3 normal = normals[index];
    finalNormals[finalNormalCount] = normal.x;
    finalNormalCount += 1;
    finalNormals[finalNormalCount] = normal.y;
    finalNormalCount += 1;
    finalNormals[finalNormalCount] = normal.z;
    finalNormalCount += 1;
  }

  logfln("final vert count: %d, uv count: %d, normal count %d", finalVertCount, finalUVCount, finalNormalCount);

  // OpenGL time!

  glGenVertexArrays(1, &m.vao);
  glBindVertexArray(m.vao);

  {
    // Vertex buffer
    glGenBuffers(1, &m.vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m.vertexBuffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(real32) * finalVertCount, finalVerts, GL_STATIC_DRAW);

    // UV buffer
    glGenBuffers(1, &m.uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m.uvBuffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(real32) * finalUVCount, finalUVs, GL_STATIC_DRAW);

    // Normal buffer
    glGenBuffers(1, &m.normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m.normalBuffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(real32) * finalNormalCount, finalNormals, GL_STATIC_DRAW);

    // Vertex attributes
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m.vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, m.uvBuffer);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, m.normalBuffer);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  m.vertCount = finalVertCount/3;

  memoryArena_clear(temp);

  ensure(m.vao != 0);

  return m;
}

void model_clean(Model* m) {
  // TODO(harrison): implement me

  ensure(false);
}

#define FONT_CHARS 94
struct Font {
  Texture texture;

  int pixelHeight;
  float baseLine;
  int64 lineHeight;

  int x0[FONT_CHARS];
  int x1[FONT_CHARS];
  int y0[FONT_CHARS];
  int y1[FONT_CHARS];

  real32 xoff[FONT_CHARS];
  real32 yoff[FONT_CHARS];
  real32 xadv[FONT_CHARS];
};

Font font_load(const char* name) {
  Font f = {0};

  char infoFilename[64];
  snprintf(infoFilename, 64, "data/font/%s.fnt", name);

  char pngFilename[64];
  snprintf(pngFilename, 64, "data/font/%s.png", name);

  {
    void* pngData;
    uint32 pngLen;

    loadFromFile(pngFilename, &pngData, &pngLen);

    stbi_set_flip_vertically_on_load(false);
    f.texture = texture_init(pngData, pngLen);
    stbi_set_flip_vertically_on_load(true);
  }

  {
    void* metaData;
    uint32 metaLen;

    loadFromFile(infoFilename, &metaData, &metaLen);

    char* meta = (char*) metaData;

    char* line = (char*) lornockMemory->transientStorage;
    uint32 lineLen = 0;

    uint32 rawUpTo = 0;

    bool firstLine = true;

    while (rawUpTo < metaLen) {
      getLine(line, &lineLen, &rawUpTo, meta, metaLen);
      rawUpTo += 1;

      if (firstLine) {
        ensure(sscanf(line, "ph %d bl %f lh %ld",
              &f.pixelHeight,
              &f.baseLine,
              &f.lineHeight) == 3);

        firstLine = false;

        continue;
      }

      int id = 0;

      ensure(sscanf(line, "id %d", &id) == 1);

      id -= 32;

      ensure(sscanf(line,
            "id %*d x0 %d y0 %d x1 %d y1 %d xoff %f yoff %f xadv %f",
            &f.x0[id], &f.y0[id],
            &f.x1[id], &f.y1[id],
            &f.xoff[id], &f.yoff[id],
            &f.xadv[id]
            ) == 7);

      if (id == 95) {
        break;
      }
    }
  }

  return f;
}

void font_clean(Font* f) {
  // TODO: fill

  ensure(false);
}

real32 font_getStringWidth(Font f, char* text, real32 scale) {
  real32 w = 0;
  int i = 0;

  while (text[i]) {
    if (text[i] >= 32 && text[i] < 126) {
      int c = text[i] - 32;

      w += f.xadv[c] * scale;
    }

    i += 1;
  }

  return w;
}

real32 font_getStringHeight(Font f, real32 scale) {
  return f.pixelHeight * scale;
}

struct Assets {
  int32 shaderRequests[MAX_SHADER];
  Shader shaders[MAX_SHADER];

  int32 textureRequests[MAX_TEXTURE];
  Texture textures[MAX_TEXTURE];

  int32 modelRequests[MAX_MODEL];
  Model models[MAX_MODEL];

  int32 fontRequests[MAX_FONT];
  Font fonts[MAX_FONT];
};
