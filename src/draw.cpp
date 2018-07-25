#define DRAW_3D_NEAR_PLANE (0.1f)
#define DRAW_3D_FAR_PLANE (100.0f)

struct {
  vec4 clear;

  mat4 view;
  mat4 projection;

  Shader activeShader;

  GLuint quadVAO;
  GLuint quadVertexBuffer;
  GLuint quadUVBuffer;
} draw;

void draw_init() {
  draw.view = mat4d(1.0f);

	GLfloat vertices[] = { 
		0.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 0.0f
	};

  GLfloat uvs[] = {
    0.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,

    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f
  };

  glGenVertexArrays(1, &draw.quadVAO);
  glBindVertexArray(draw.quadVAO);

  {
    glGenBuffers(1, &draw.quadVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, draw.quadVertexBuffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &draw.quadUVBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, draw.quadUVBuffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, draw.quadVertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, draw.quadUVBuffer);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  glBindVertexArray(0);
}

void draw_clean() {
  glDeleteBuffers(1, &draw.quadVertexBuffer);
  glDeleteBuffers(1, &draw.quadUVBuffer);
  glDeleteVertexArrays(1, &draw.quadVAO);
}

void draw_begin() {
  // TODO(harrison): how much of a performance hit does this cost?
  glViewport(0, 0, getWindowWidth(), getWindowHeight());

  glClearColor(draw.clear.x/255.0f, draw.clear.y/255.0f, draw.clear.z/255.f, draw.clear.w);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  draw.view = mat4d(1.0f);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void draw_setShader(Shader s) {
  glUseProgram(s.id);

  draw.activeShader = s;
}

void draw_3d_begin(real32 fov) {
  glDisable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  draw.projection = mat4Perspective(fov,
      (real32) getWindowWidth() / (real32) getWindowHeight(),
      DRAW_3D_NEAR_PLANE,
      DRAW_3D_FAR_PLANE);
}

void draw_2d_begin() {
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);

  draw.view = mat4d(1.0f);
  draw.projection = mat4Orthographic(0, (real32) getWindowWidth(), (real32) getWindowHeight(), 0, -1.0f, 1.0f);
}

void draw_rectangle(Rect rect, vec4 color) {
  // TODO(harrison): have own repository of shaders, do not use the user ones.

  draw_setShader(shader(SHADER_rectangle));

  mat4 model = mat4d(1.0f);
  model = mat4Translate(model, vec3(rect.x, rect.y, 0));
  model = mat4Scale(model, vec3(rect.w, rect.h, 1));

  shader_setMatrix(&draw.activeShader, "model", model);
  shader_setMatrix(&draw.activeShader, "view", draw.view);
  shader_setMatrix(&draw.activeShader, "projection", draw.projection);

  shader_setVec4(&draw.activeShader, "color", vec4(color.x/255.0f, color.y/255.0f, color.z/255.0f, color.w));

  glBindVertexArray(draw.quadVAO);

  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
}

void draw_sprite(Rect rect, Texture t) {
  // TODO(harrison): have own repository of shaders, do not use the user ones.

  draw_setShader(shader(SHADER_sprite));

  mat4 model = mat4d(1.0f);
  model = mat4Translate(model, vec3(rect.x, rect.y, 0));
  model = mat4Scale(model, vec3(rect.w, rect.h, 1));

  shader_setMatrix(&draw.activeShader, "model", model);
  shader_setMatrix(&draw.activeShader, "view", draw.view);
  shader_setMatrix(&draw.activeShader, "projection", draw.projection);

  glBindTexture(GL_TEXTURE_2D, t.id);

  glBindVertexArray(draw.quadVAO);

  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
}

void draw_3d_mesh(Mesh mesh, mat4 model, Texture t) {
  shader_setMatrix(&draw.activeShader, "model", model);

  shader_setMatrix(&draw.activeShader, "view", draw.view);
  shader_setMatrix(&draw.activeShader, "projection", draw.projection);

  glBindTexture(GL_TEXTURE_2D, t.id);
  glBindVertexArray(mesh.vao);
  glDrawArrays(GL_TRIANGLES, 0, mesh.faceCount);
  glBindVertexArray(0);
}

void draw_3d_model(Model m, mat4 model, Texture t) {
  shader_setMatrix(&draw.activeShader, "model", model);

  shader_setMatrix(&draw.activeShader, "view", draw.view);
  shader_setMatrix(&draw.activeShader, "projection", draw.projection);

  glBindTexture(GL_TEXTURE_2D, t.id);
  glBindVertexArray(m.vao);
  glDrawArrays(GL_TRIANGLES, 0, m.vertCount);
  glBindVertexArray(0);
}
