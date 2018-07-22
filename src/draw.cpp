#define DRAW_3D_NEAR_PLANE (0.1f)
#define DRAW_3D_FAR_PLANE (100.0f)

struct {
  vec4 clear;

  mat4 view;
  mat4 projection;

  Shader activeShader;
} draw;

void draw_init() {
  draw.view = mat4d(1.0f);
}

void draw_begin() {
  // TODO(harrison): how much of a performance hit does this cost?
  glViewport(0, 0, getWindowWidth(), getWindowHeight());

  glClearColor(draw.clear.x/255.0f, draw.clear.y/255.0f, draw.clear.z/255.f, draw.clear.w);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  draw.view = mat4d(1.0f);
}

void draw_3d_begin(real32 fov) {
  draw.projection = mat4Perspective(fov,
      (real32) getWindowWidth() / (real32) getWindowHeight(),
      DRAW_3D_NEAR_PLANE,
      DRAW_3D_FAR_PLANE);
}

void draw_setShader(Shader s) {
  glUseProgram(s.id);

  draw.activeShader = s;
}

void draw_3d_mesh(Mesh mesh, mat4 model, Texture t) {
  shaderSetMatrix(&draw.activeShader, "model", model);

  shaderSetMatrix(&draw.activeShader, "view", draw.view);
  shaderSetMatrix(&draw.activeShader, "projection", draw.projection);

  glBindTexture(GL_TEXTURE_2D, t.id);
  glBindVertexArray(mesh.vao);
  glDrawArrays(GL_TRIANGLES, 0, mesh.faceCount);
  glBindVertexArray(0);
}

void draw_3d_model(Model m, mat4 model, Texture t) {
  shaderSetMatrix(&draw.activeShader, "model", model);

  shaderSetMatrix(&draw.activeShader, "view", draw.view);
  shaderSetMatrix(&draw.activeShader, "projection", draw.projection);

  glBindTexture(GL_TEXTURE_2D, t.id);
  glBindVertexArray(m.vao);
  glDrawArrays(GL_TRIANGLES, 0, m.vertCount);
  glBindVertexArray(0);
}
