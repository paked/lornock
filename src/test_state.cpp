enum {
  DIRECTION_UP,
  DIRECTION_RIGHT,
  DIRECTION_FORWARD,
  MAX_DIRECTION
};

vec3 faceCardinalDirections[MAX_FACE][MAX_DIRECTION] = {
  // UP                       Right                     Forward
  { vec3(0.0f, 0.0f, 1.0f),  vec3(1.0f, 0.0f, 0.0f),   vec3(0.0f, 1.0f, 0.0f) },  // Back face
  { vec3(0.0f, 0.0f, -1.0f),   vec3(1.0f, 0.0f, 0.0f),   vec3(0.0f, -1.0f, 0.0f) },   // Front face

  { vec3(-1.0f, 0.0f, 0.0f),  vec3(0.0f, 1.0f, 0.0f),  vec3(0.0f, 0.0f, -1.0f) },  // Left face
  { vec3(1.0f, 0.0f, 0.0f),   vec3(0.0f, -1.0f, 0.0f),  vec3(0.0f, 0.0f, -1.0f) },  // Right face

  { vec3(0.0f, 1.0f, 0.0f),   vec3(1.0f, 0.0f, 0.0f),   vec3(0.0f, 0.0f, -1.0f) },  // Top face
  { vec3(0.0f, -1.0f, 0.0f),  vec3(1.0f, 0.0f, 0.0f),  vec3(0.0f, 0.0f, 1.0f) }  // Bottom face
};

struct TestState {
  real32 t;

  uint32 currentFace;

  // Player
  vec3 up;
  vec3 forward;
  vec3 right;
  vec3 pos;
  mat4 rotMat;

  // Current face
  vec3 faceRight;
  vec3 faceForward;

  // Camera
  quat cameraRotation;

  // Rotation animation
  bool rotating;
  uint32 startTime;
  real32 pitchEnd;
  real32 yawEnd;
  real32 rollEnd;
  quat start;

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

  g->cameraRotation = quatFromPitchYawRoll(90.0f, 0, 0);

  g->rotating = false;
  g->startTime = 0;
  g->pitchEnd = 0;
  g->yawEnd = 0;
  g->rollEnd = 0;

  {
    mat4 rot = HMM_Rotate(90.0f, g->forward);

    vec4 forward = vec4FromVec3(g->forward);

    vec4 right = rot * vec4FromVec3(g->right);
    vec4 up = rot * vec4FromVec3(g->up);

    logfln("RIGHT. vec3(%.1f,%.1f,%.1f), vec3(%.1f,%.1f,%.1f), vec3(%.1f,%.1f,%.1f)", up.x, up.y, up.z, right.x, right.y, right.z, forward.x, forward.y, forward.z);
  }

  {
    mat4 view = mat4d(1);
    view = mat4Translate(view, vec3(0.0f, 0.0f, -8.0f));
    view = view * quatToMat4(g->cameraRotation);

    g->faceRight = vec3(view[0][0], view[1][0], view[2][0]);
    g->faceForward = vec3(view[0][1], view[1][1], view[2][1]);
  }

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

void testState_rotate(TestState* g, uint32 direction) {
  real32 pitchFactor = 0.0f;
  real32 yawFactor = 0.0f;

  mat4 rot;

  switch(direction) {
    case ROT_FORWARD:
      {
        pitchFactor = 1.0f;

        rot = HMM_Rotate(-90.0f, g->right);

        vec4 forward = rot * vec4FromVec3(g->forward);
        g->forward = vec3FromVec4(forward);

        vec4 up = rot * vec4FromVec3(g->up);
        g->up = vec3FromVec4(up);
      } break;
    case ROT_BACKWARD:
      {
        pitchFactor = -1.0f;

        rot = HMM_Rotate(90.0f, g->right);

        vec4 forward = rot * vec4FromVec3(g->forward);
        g->forward = vec3FromVec4(forward);

        vec4 up = rot * vec4FromVec3(g->up);
        g->up = vec3FromVec4(up);
      } break;
    case ROT_LEFT:
      {
        yawFactor = 1.0f;

        rot = HMM_Rotate(-90.0f, g->forward);

        vec4 right = rot * vec4FromVec3(g->right);
        g->right = vec3FromVec4(right);

        vec4 up = rot * vec4FromVec3(g->up);
        g->up = vec3FromVec4(up);
      } break;
    case ROT_RIGHT:
      {
        yawFactor = -1.0f;

        rot = HMM_Rotate(90.0f, g->forward);

        vec4 right = rot * vec4FromVec3(g->right);
        g->right = vec3FromVec4(right);

        vec4 up = rot * vec4FromVec3(g->up);
        g->up = vec3FromVec4(up);
      } break;
    default:
      {
        logln("ERROR: unknown rotation direction");

        return;
      } break;
  }

  logln("old:");
  printFace(g->currentFace);
  printRot(direction);

  g->rotMat = g->rotMat * rot;
  g->currentFace = faceRotationMap[g->currentFace][direction];

  g->rotating = true;
  g->start = g->cameraRotation;
  g->startTime = getTime();
  g->pitchEnd = 90.0f * pitchFactor;
  g->yawEnd = 90.0f * yawFactor;
  g->rollEnd = 0.0f;

  logln("new:");
  printFace(g->currentFace);
  printRot(direction);
}

void testState_update(State* state) {
  TestState* g = (TestState*) state->memory;

  if (keyDown(KEY_tab)) {
    g->t += getDt() * 100;
  }

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

  if (keyJustDown(KEY_shift)) {
    uint32 rightDir = DIRECTION_RIGHT;
    uint32 forwardDir = DIRECTION_FORWARD;

    vec3 realRight = faceCardinalDirections[g->currentFace][rightDir];
    vec3 realForward = faceCardinalDirections[g->currentFace][forwardDir];
    vec3 realUp = faceCardinalDirections[g->currentFace][DIRECTION_UP];

    realRight = g->faceRight;
    realForward = g->faceForward * -1;

    real32 rightLen = vec3Sum(g->pos * realRight);
    real32 forwardLen = vec3Sum(g->pos * realForward);

    /*
    if (g->currentFace == LEFT || g->currentFace == RIGHT) {
      real32 temp = rightLen;
      rightLen = forwardLen;
      forwardLen = temp;
      logln("WHAT?");
    }*/

    logfln("rightlen: %f, forwardlen: %f", rightLen, forwardLen);

    if (fabs(rightLen) > 1.49f) {
      testState_rotate(g, (rightLen > 0) ? ROT_RIGHT : ROT_LEFT);
    } else if (fabs(forwardLen) > 1.49f) {
      testState_rotate(g, (forwardLen < 0) ? ROT_FORWARD : ROT_BACKWARD);
    }
  }

  g->pos.x = clamp(g->pos.x, -1.5f, 1.5f);
  g->pos.y = clamp(g->pos.y, -1.5f, 1.5f);
  g->pos.z = clamp(g->pos.z, -1.5f, 1.5f);

  if (g->rotating) {
    real32 pc = 1.0f - ((float) ((g->startTime + ROTATION_DURATION) - getTime())) / ROTATION_DURATION;

    if (getTime() > g->startTime + ROTATION_DURATION) {
      pc = 1.0f;
      g->rotating = false;
    }

    pc = pc * pc;

    g->cameraRotation = quatFromPitchYawRoll(g->pitchEnd * pc, g->yawEnd * pc, g->rollEnd * pc) * g->start;
    g->cameraRotation = quatNormalize(g->cameraRotation);
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  mat4 projection = mat4Perspective(70.0f, (real32) getWindowWidth() / (real32) getWindowHeight(), 0.1f, 1000.0f);

  /*
  real32 t = g->t;
  vec3 cameraPos = vec3(sin(deg2Rad(t))*10, 3.0f, cos(deg2Rad(t))*10);
  mat4 view = mat4d(1.0f);
  view = mat4LookAt(view, cameraPos, vec3(0, 0, 0));*/

  {
    mat4 view = mat4d(1);
    view = mat4Translate(view, vec3(0.0f, 0.0f, -8.0f));
    view = view * quatToMat4(g->cameraRotation);

    g->faceRight = vec3(view[0][0], view[1][0], view[2][0]);
    g->faceForward = vec3(view[0][1], view[1][1], view[2][1]);
  }

  quat offset = quatFromPitchYawRoll(-30.0f, 0, 0);

  mat4 view = mat4d(1.0f);
  view = mat4Translate(view, vec3(0.0f, 0.0f, -8.0f));
  view = view * quatToMat4(offset);
  view = view * quatToMat4(g->cameraRotation);

  {
    Shader s = shader(SHADER_default);
    Texture t = texture(TEXTURE_player);

    glUseProgram(s.id);

    mat4 model = mat4d(1.0f);
    model = mat4Translate(model, g->pos);
    model = model * g->rotMat;
    model = mat4Scale(model, vec3(0.1f, 0.1f, 0.1f));

    shaderSetMatrix(&s, "model", model);
    shaderSetMatrix(&s, "view", view);
    shaderSetMatrix(&s, "projection", projection);

    glBindTexture(GL_TEXTURE_2D, t.id);
    glBindVertexArray(g->cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }

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
}
