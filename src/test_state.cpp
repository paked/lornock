struct TestState {
  real32 t;

  uint32 currentFace;

  // Player
  vec3 up;
  vec3 forward;
  vec3 right;
  vec3 pos;
  mat4 rotMat;

  GLuint quadVAO;
  GLuint cubeVAO;
};

void testState_init(State* state) {
  TestState* g = (TestState*) state->memory;

  g->currentFace = TOP;

  g->t = 0.0f;

  g->up = vec3(0.0f, 1.0f, 0.0f);
  g->forward = vec3(0.0f, 0.0f, -1.0f);
  g->right = vec3(1.0f, 0.0f, 0.0f);

  g->pos = vec3(0.0f, 1.5f, 0.0f);

  g->rotMat = HMM_Rotate(0.0f, g->right);

  real32 quadData[] = {
    // Verts                UVs
    -0.5f, -0.5f, 0.0f,    0.0f, 0.0f,
    -0.5f,  0.5f, 0.0f,    0.0f, 1.0f,
     0.5f, -0.5f, 0.0f,    1.0f, 0.0f,
     0.5f,  0.5f, 0.0f,    1.0f, 1.0f
  };

  GLuint quadVAO, quadVBO;

  glGenVertexArrays(1, &quadVAO);
  glBindVertexArray(quadVAO);

  glGenBuffers(1, &quadVBO);

  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadData), quadData, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  g->quadVAO = quadVAO;

  real32 cubeData[] = cubeMesh;
  GLuint cubeVAO, cubeVBO;

  glGenVertexArrays(1, &cubeVAO);
  glBindVertexArray(cubeVAO);

  glGenBuffers(1, &cubeVBO);

  glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeData), cubeData, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  g->cubeVAO = cubeVAO;

  assetsRequestShader(SHADER_default);
  assetsRequestTexture(TEXTURE_test);
  assetsRequestTexture(TEXTURE_player);
  assetsRequestTexture(TEXTURE_rock);
}

void testState_update(State* state) {
  TestState* g = (TestState*) state->memory;

  if (keyDown(KEY_tab)) {
    g->t += getDt() * 100;
  }

  if (keyUp(KEY_shift)) {
    real32 speed = 2.0f;
    real32 dt = getDt();

    if (keyDown(KEY_a)) {
      g->pos -= g->right * dt * speed;
    }

    if (keyDown(KEY_d)) {
      g->pos += g->right * dt * speed;
    }

    if (keyDown(KEY_w)) {
      g->pos += g->forward * dt * speed;
    }

    if (keyDown(KEY_s)) {
      g->pos -= g->forward * dt * speed;
    }

    g->pos.x = clamp(g->pos.x, -1.5f, 1.5f);
    g->pos.y = clamp(g->pos.y, -1.5f, 1.5f);
    g->pos.z = clamp(g->pos.z, -1.5f, 1.5f);
  } else {
    uint32 rotation = ROT_IDLE;

    if (keyJustDown(KEY_w)) {
      rotation = ROT_FORWARD;
      mat4 rot = HMM_Rotate(-90.0f, g->right);

      g->rotMat = g->rotMat * rot;

      vec4 forward = rot * vec4FromVec3(g->forward);
      g->forward = vec3FromVec4(forward);

      vec4 up = rot * vec4FromVec3(g->up);
      g->up = vec3FromVec4(up);
    } else if (keyJustDown(KEY_s)) {
      rotation = ROT_BACKWARD;

      mat4 rot = HMM_Rotate(90.0f, g->right);

      g->rotMat = g->rotMat * rot;

      vec4 forward = rot * vec4FromVec3(g->forward);
      g->forward = vec3FromVec4(forward);

      vec4 up = rot * vec4FromVec3(g->up);
      g->up = vec3FromVec4(up);
    } else if (keyJustDown(KEY_a)) {
      rotation = ROT_LEFT;

      mat4 rot = HMM_Rotate(-90.0f, g->forward);

      g->rotMat = g->rotMat * rot;

      vec4 right = rot * vec4FromVec3(g->right);
      g->right = vec3FromVec4(right);

      vec4 up = rot * vec4FromVec3(g->up);
      g->up = vec3FromVec4(up);
    } else if (keyJustDown(KEY_d)) {
      rotation = ROT_RIGHT;

      mat4 rot = HMM_Rotate(90.0f, g->forward);

      g->rotMat = g->rotMat * rot;

      vec4 right = rot * vec4FromVec3(g->right);
      g->right = vec3FromVec4(right);

      vec4 up = rot * vec4FromVec3(g->up);
      g->up = vec3FromVec4(up);   
    }

    if (rotation != ROT_IDLE) {
      printRot(rotation);
      log("old face: ");
      printFace(g->currentFace);

      g->currentFace = faceRotationMap[g->currentFace][rotation];
      log("new face: ");
      printFace(g->currentFace);

      logfln("up: %f %f %f", g->up.x, g->up.y, g->up.z);
      logfln("right: %f %f %f", g->right.x, g->right.y, g->right.z);
      logfln("forward: %f %f %f", g->forward.x, g->forward.y, g->forward.z);
    }
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  mat4 projection = mat4Perspective(70.0f, (real32) getWindowWidth() / (real32) getWindowHeight(), 0.1f, 1000.0f);

  real32 t = g->t;
  vec3 cameraPos = vec3(sin(deg2Rad(t))*10, 3.0f, cos(deg2Rad(t))*10);
  mat4 view = mat4d(1.0f);
  view = mat4LookAt(view, cameraPos, vec3(0, 0, 0));

  {
    Shader s = shader(SHADER_default);
    Texture t = texture(TEXTURE_test);

    glUseProgram(s.id);

    mat4 model = mat4d(1.0f);
    model = mat4Translate(model, vec3(-1.5f, -1.5f, -1.5f));
    model = mat4Scale(model, vec3(3, 3, 3));

    shaderSetMatrix(&s, "model", model);
    shaderSetMatrix(&s, "view", view);
    shaderSetMatrix(&s, "projection", projection);

    glBindTexture(GL_TEXTURE_2D, t.id);
    glBindVertexArray(g->cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }

  {
    Shader s = shader(SHADER_default);
    Texture t = texture(TEXTURE_player);

    glUseProgram(s.id);

    vec3 pivot = vec3(0.0f, 0.5f, 0.0f);
    mat4 model = mat4d(1.0f);
    model = mat4Translate(model, g->pos);
    model = model * g->rotMat;
    model = mat4Translate(model, pivot);

    shaderSetMatrix(&s, "model", model);
    shaderSetMatrix(&s, "view", view);
    shaderSetMatrix(&s, "projection", projection);

    glBindTexture(GL_TEXTURE_2D, t.id);
    glBindVertexArray(g->quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    mat4 parent = model;

    s = shader(SHADER_default);
    t = texture(TEXTURE_rock);

    glUseProgram(s.id);

    model = mat4d(1.0f);
    model = mat4Translate(model, vec3(-0.5f, -0.5f, -0.5f));
    model = mat4Translate(model, vec3(0, 0, 3.0f));
    model = parent * model;

    shaderSetMatrix(&s, "model", model);
    shaderSetMatrix(&s, "view", view);
    shaderSetMatrix(&s, "projection", projection);

    glBindTexture(GL_TEXTURE_2D, t.id);
    glBindVertexArray(g->cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }
}
