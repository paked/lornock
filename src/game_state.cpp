#include <game_state.hpp>

#include <entities/actions.cpp>
#include <entities/timeline.cpp>
#include <entities/past_player.cpp>
#include <entities/particles.cpp>

#define CLOUD_COUNT (50)
struct GameState {
  TimeIndex timeIndex;
  uint64 timelineNextTickTime;
  Timeline timeline;

  vec3 playerUp;
  vec3 playerRight;
  vec3 playerForward;
  vec3 playerPos;
  vec3 playerLastMove;
  mat4 playerRotation;

  quat cameraRotation;

  vec3 faceRight;
  vec3 faceForward;

  bool rotating;
  uint32 rotatingStartTime;
  real32 rotatingPitchEnd;
  real32 rotatingYawEnd;
  real32 rotatingRollEnd;
  quat rotatingStart;

  Environment environment;

  PastPlayer pastPlayers[MAX_PAST_PLAYERS];

  World world;
  Mesh worldMesh;

  Mesh cubeMesh;

  GLuint shadowFBO;
  GLuint shadowMap;

  // @Particles
  ParticleEmitter clouds[CLOUD_COUNT];
};

uint32 gameState_getCurrentFace(GameState *g) {
  for (int i = 0; i < MAX_FACE; i++) {
    if (vec3AlmostEqual(faceCardinalDirections[i][DIRECTION_UP], g->playerUp)) {
      return i;
    }
  }

  ensure(false);

  return -1;
}

void gameState_spawnNecessaryPastPlayers(GameState *g, Timeline* tb, TimeIndex* worldIndex);

void gameState_setFaceDirections(GameState* g) {
  mat4 view = mat4d(1);
  view = mat4Translate(view, CAMERA_POSITION);
  view = view * quatToMat4(g->cameraRotation);

  g->faceRight = vec3(view[0][0], view[1][0], view[2][0]);
  g->faceForward = vec3(view[0][1], view[1][1], view[2][1]);
}

int gameState_getFirstPastPlayer(GameState* g) {
  int id = -1;

  for (int i = 0; i < MAX_PAST_PLAYERS; i++) {
    if (!g->pastPlayers[i].exists) {
      id = i;

      break;
    }
  }

  return id;
}

void gameState_disableAllPastPlayers(GameState* g) {
  for (int i = 0; i < MAX_PAST_PLAYERS; i++) {
    g->pastPlayers[i].exists = false;
  }
}

void gameState_init(State* state) {
  LornockMemory* m = lornockMemory;
  GameState* g = (GameState*) state->memory;

  // @Clouds init
  for (int i = 0; i < CLOUD_COUNT; i++) {
    real32 x = rand01() * 10.0f;
    real32 y = rand01() * 10.0f;
    real32 z = rand01() * 10.0f;

    x *= (rand01() < 0.5 ? -1 : 1);
    y *= (rand01() < 0.5 ? -1 : 1);
    z *= (rand01() < 0.5 ? -1 : 1);

    if (vec3LengthSquared(vec3(x, y, z)) < (WORLD_SIZE*2.5f) * (WORLD_SIZE*2.5f)) {
      i -= 1;

      continue;
    }

    real32 w = randFromTo(0.5f, 1.5f) * PARTICLE_CLOUD_W;
    real32 h = randFromTo(0.8f, 1.5f) * PARTICLE_CLOUD_H;
    real32 d = randFromTo(0.5f, 1.5f) * PARTICLE_CLOUD_D;

    g->clouds[i] = particleEmitter_init(x, y, z);

    {
      g->clouds[i].particleShape = TEXTURE_particle;
      g->clouds[i].colorMap = (rand01() > 0.8f) ? TEXTURE_particle_color_map_red : TEXTURE_particle_color_map_blue;
    }

    particleEmitter_generateEllipsoid(&g->clouds[i], w, h, d);
  }

  timeline_init(&g->timeline, &lornockData->actionsArena);
  timeline_load(&g->timeline, "simple");
  g->timelineNextTickTime = 0;

  g->cameraRotation = g->timeline.info.camera;

  g->playerUp = g->timeline.info.up;
  g->playerRight = g->timeline.info.right;
  g->playerForward = g->timeline.info.forward;

  g->playerPos = vec3(0.0f, 1.5f, 0.0f);
  g->playerLastMove = vec3_zero;
  g->playerRotation = mat4d(1.0f);

  gameState_setFaceDirections(g); // g->faceRight, g->faceForward

  g->rotating = false;
  g->rotatingStartTime = 0;
  g->rotatingPitchEnd = 0;
  g->rotatingYawEnd = 0;
  g->rotatingRollEnd = 0;
  g->rotatingStart = quatFromPitchYawRoll(0.0f, 0.0f, 0.0f);

  gameState_disableAllPastPlayers(g);

  for (int y = 0; y < WORLD_CELL_COUNT; y++) {
    for (int z = 0; z < WORLD_CELL_COUNT; z++) {
      for (int x = 0; x < WORLD_CELL_COUNT; x++) {
        g->world[y][z][x] = 1;
      }
    }
  }

  // init world
  for (int y = 0; y < WORLD_CELL_COUNT; y++) {
    for (int z = 0; z < WORLD_CELL_COUNT; z++) {
      for (int x = 0; x < WORLD_CELL_COUNT; x++) {
        g->world[y][z][x] = 1;
      }
    }
  }

  uint64 count = 0;
  real32* verts = (real32*) m->transientStorage;

  for (int y = 0; y < WORLD_CELL_COUNT; y++) {
    for (int z = 0; z < WORLD_CELL_COUNT; z++) {
      for (int x = 0; x < WORLD_CELL_COUNT; x++) {
        if (g->world[y][z][x] != 1) {
          continue;
        }

        vec3 pos = vec3(x, y, z) * WORLD_CELL_SIZE;

        for (int d = 0; d < MAX_FACE; d++) {
          if (!voxelEmptyToThe(d, g->world, x, y, z)) {
            continue;
          }

          addFaceToMesh(d, verts, &count, pos);
        }
      }
    }
  }

  g->worldMesh = mesh_init(verts, count);

  real32 cubeData[] = CUBE_MESH_DATA;
  g->cubeMesh = mesh_init(cubeData, sizeof(cubeData)/sizeof(real32));

  glGenFramebuffers(1, &g->shadowFBO);

  // const unsigned int SHADOW_WIDTH = getWindowWidth(), SHADOW_HEIGHT = getWindowHeight();
  const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

  GLuint shadowMap;
  glGenTextures(1, &shadowMap);

  glBindTexture(GL_TEXTURE_2D, shadowMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

  glBindFramebuffer(GL_FRAMEBUFFER, g->shadowFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  g->shadowMap = shadowMap;

  assets_requestShader(SHADER_default);
  assets_requestShader(SHADER_depth);
  assets_requestShader(SHADER_particle);

  assets_requestShader(SHADER_text);
  assets_requestShader(SHADER_sprite);
  assets_requestShader(SHADER_rectangle);

  assets_requestTexture(TEXTURE_particle);
  assets_requestTexture(TEXTURE_particle_color_map_blue);
  assets_requestTexture(TEXTURE_particle_color_map_red);

  assets_requestTexture(TEXTURE_test);
  assets_requestTexture(TEXTURE_player);
  assets_requestTexture(TEXTURE_rock);

  assets_requestTexture(TEXTURE_rock_icon);
  assets_requestTexture(TEXTURE_rock_albedo);

  assets_requestTexture(TEXTURE_coal_icon);
  assets_requestTexture(TEXTURE_coal_albedo);

  assets_requestModel(MODEL_rock);
  assets_requestModel(MODEL_coal);

  assets_requestFont(FONT_quicksand_regular);
  assets_requestFont(FONT_renogare);

  draw.clear = vec4(7.0f, 6.0f, 15.0f, 1.0f);

  {
    Action a = { 0 };

    ensure(timeline_findLastAction(&g->timeline, &a));

    g->timeIndex.time = g->timeIndex.timeDoneTo = a.common.time;
    g->timeIndex.sequence = a.common.sequence;
    g->timeIndex.jumpID = a.common.jumpID;

    uint64 current = a.common.sequence;

    bool found = true;
    while (timeline_actionInSequence(&g->timeline, current, &a)) {
      if (a.common.type == MOVE || a.common.type == SPAWN) {
        found = true;

        break;
      }

      current -= 1;
    }

    ensure(found);

    if (a.common.type == MOVE) {
      g->playerPos = a.move.pos;
    } else if (a.common.type == SPAWN) {
      g->playerPos = a.spawn.pos;
    }

    logfln("starting at: %f %f %f", a.move.pos.x, a.move.pos.y, a.move.pos.z);
  }

  {
    gameState_spawnNecessaryPastPlayers(g, &g->timeline, &g->timeIndex);

    timeline_getEnvironmentStateAt(&g->timeline, &g->environment, g->timeIndex.time);
  }
}

void gameState_rotate(GameState* g, uint32 direction) {
  real32 pitchFactor = 0.0f;
  real32 yawFactor = 0.0f;

  mat4 rot;

  switch(direction) {
    case ROT_FORWARD:
      {
        pitchFactor = 1.0f;

        rot = HMM_Rotate(-90.0f, g->playerRight);

        vec4 forward = rot * vec4FromVec3(g->playerForward);
        g->playerForward = vec3FromVec4(forward);

        vec4 up = rot * vec4FromVec3(g->playerUp);
        g->playerUp = vec3FromVec4(up);
      } break;
    case ROT_BACKWARD:
      {
        pitchFactor = -1.0f;

        rot = HMM_Rotate(90.0f, g->playerRight);

        vec4 forward = rot * vec4FromVec3(g->playerForward);
        g->playerForward = vec3FromVec4(forward);

        vec4 up = rot * vec4FromVec3(g->playerUp);
        g->playerUp = vec3FromVec4(up);
      } break;
    case ROT_LEFT:
      {
        yawFactor = 1.0f;

        rot = HMM_Rotate(-90.0f, g->playerForward);

        vec4 right = rot * vec4FromVec3(g->playerRight);
        g->playerRight = vec3FromVec4(right);

        vec4 up = rot * vec4FromVec3(g->playerUp);
        g->playerUp = vec3FromVec4(up);
      } break;
    case ROT_RIGHT:
      {
        yawFactor = -1.0f;

        rot = HMM_Rotate(90.0f, g->playerForward);

        vec4 right = rot * vec4FromVec3(g->playerRight);
        g->playerRight = vec3FromVec4(right);

        vec4 up = rot * vec4FromVec3(g->playerUp);
        g->playerUp = vec3FromVec4(up);
      } break;
    default:
      {
        logln("ERROR: unknown rotation direction");

        return;
      } break;
  }

  g->playerRotation = g->playerRotation * rot;

  g->rotating = true;
  g->rotatingStart = g->cameraRotation;
  g->rotatingStartTime = getTime();
  g->rotatingPitchEnd = 90.0f * pitchFactor;
  g->rotatingYawEnd = 90.0f * yawFactor;
  g->rotatingRollEnd = 0.0f;
}

void gameState_spawnNecessaryPastPlayers(GameState *g, Timeline* tb, TimeIndex* worldIndex) {
  gameState_disableAllPastPlayers(g);

  TimeIndex index = *worldIndex;
  index.timeDoneTo = -1;

  Action spawnAction;
  while (timeline_nextAction(tb, &index, &spawnAction)) {
    if (spawnAction.type != SPAWN || spawnAction.common.jumpID == index.jumpID) {
      continue;
    }

    bool unresolved = true;

    Action lastMove = spawnAction;
    Action jumpAction;
    uint64 seq = spawnAction.common.sequence + 1;
    while (timeline_actionInSequence(tb, seq, &jumpAction)) {
      seq += 1;

      if (jumpAction.type == MOVE) {
        lastMove = jumpAction;
      }

      if (jumpAction.common.time > index.time) {
        break;
      }

      if (jumpAction.type == JUMP) {
        unresolved = false;

        break;
      }
    }

    if (unresolved) {
      logln("SPAWNING PAST PLAYER!");
      int id = gameState_getFirstPastPlayer(g);
      if (id < 0) {
        logln("WARNING: too many active past players.");

        continue;
      }

      pastPlayer_init(&g->pastPlayers[id], tb, worldIndex, lastMove);
    }
  }
}

void gameState_timeJump(GameState *g, int64 destination) {
  Timeline* tb = &g->timeline;
  TimeIndex* index = &g->timeIndex;

  timeline_add(tb, index, action_makeJump(destination));
  index->jumpID += 1;

  timeline_commit(tb);

  index->time = index->timeDoneTo = destination;

  timeline_add(tb, index, action_makeSpawn(g->playerPos));

  gameState_spawnNecessaryPastPlayers(g, tb, index);
  timeline_getEnvironmentStateAt(tb, &g->environment, index->time);
}

bool gameState_pickup(GameState* g, uint32 item) {
  // Firstly try and fit it into an existing stack
  for (int i = 0; i < INVENTORY_SIZE; i++) {
    InventorySlot* slot = &g->timeline.info.inventory.hotbar[i];

    if (slot->type == item && slot->count < INVENTORY_MAX_STACK_SIZE) {
      slot->count += 1;

      return true;
    }
  }

  // Secondly make new stack!
  for (int i = 0; i < INVENTORY_SIZE; i++) {
    InventorySlot* slot = &g->timeline.info.inventory.hotbar[i];

    if (slot->type == BLOCK_NONE) {
      slot->type = item;
      slot->count = 1;

      return true;
    }
  }

  // No space left in inventory

  return false;
}

bool gameState_use(GameState* g, uint32 slot) {
  ensure(slot < INVENTORY_SIZE && slot >= 0);

  InventorySlot* is = &g->timeline.info.inventory.hotbar[slot];

  if (is->type == BLOCK_NONE) {
    return false;
  }

  if (is->count > 0) {
    is->count -= 1;

    if (is->count == 0) {
      is->type = BLOCK_NONE;
    }
  }

  return true;
}

void gameState_touchEnvironment(GameState* g, int face, int x, int y) {
  uint8 v = g->environment[face][y][x];

  uint32 block = v;

  if (v == BLOCK_NONE) {
    uint32 i = g->timeline.info.inventory.hotbar[g->timeline.info.inventory.currentSlot].type;

    if (!gameState_use(g, g->timeline.info.inventory.currentSlot)) {
      logln("nothing in selected inventory slot");

      return;
    }

    block = i;
  } else {
    gameState_pickup(g, v);

    block = BLOCK_NONE;
  }

  timeline_add(&g->timeline, &g->timeIndex, action_makeTouch(block, face, x, y));

  environment_handle(&g->environment, block, face, x, y);
}

enum RenderMode {
  RENDER_MODE_NORMAL,
  RENDER_MODE_LIGHT,
};

void gameState_render(GameState *g, RenderMode m) {
  vec3 lightColor = vec3_one;

  vec3 lightPos = vec3(8.0f, 8.0f, -5.0f);

  mat4 lightView = mat4LookAt(mat4d(1.0f), lightPos, vec3(0.0f, 0.0f, 0.0f));
  mat4 lightProjection = mat4Orthographic(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 100.0f);

  if (m == RENDER_MODE_NORMAL) {
    draw_begin(); // Clear buffers, set viewport size, etc.

    draw_3d_begin(70.0f);

    // TODO(harrison): refactor into OrbitalCamera struct.
    {
      mat4 view = mat4d(1.0f);
      view = mat4Translate(view, CAMERA_POSITION);
      view = view * quatToMat4(quatFromPitchYawRoll(CAMERA_ROTATION_OFFSET, 0, 0));
      view = view * quatToMat4(g->cameraRotation);

      draw.view = view;
    }

    draw_setShader(shader(SHADER_default));

    shader_setVec3(&draw.activeShader, "lightColor", lightColor);
    shader_setVec3(&draw.activeShader, "lightPos", lightPos);

    mat4 lightSpaceMatrix = lightProjection * lightView;

    shader_setInt(&draw.activeShader, "shadowMap", 1);
    shader_setInt(&draw.activeShader, "ourTexture", 0);

    shader_setMatrix(&draw.activeShader, "lightSpaceMatrix", lightSpaceMatrix);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g->shadowMap);

    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  } else if (m == RENDER_MODE_LIGHT) {
    glViewport(0, 0, 1024, 1024);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    draw.view = lightView;

    draw.projection = lightProjection;

    draw_setShader(shader(SHADER_depth));

    if (keyDown(KEY_p)) {
      return;
    }
  } else {
    ensure(false);
  }

  // @Render

  // @Asteroid render
  {
    vec3 asteroidPosition = vec3(0, 0, 0);
    vec3 asteroidOffset = -WORLD_SIZE/2 * vec3_one;

    mat4 model = mat4d(1);
    model = mat4Translate(model, asteroidOffset);
    model = mat4Translate(model, asteroidPosition);

    draw_3d_mesh(g->worldMesh, model, texture(TEXTURE_rock));
  }

  // @Player render
  {
    real32 scale = 0.5f;

    mat4 model = mat4d(1.0f);
    model = mat4Translate(model, g->playerPos);
    model = mat4Translate(model, -1*vec3_one/2*scale);
    model = mat4Scale(model, vec3_one*scale);

    draw_3d_mesh(g->cubeMesh, model, texture(TEXTURE_test));
  }

  // @Past Players render
  {
    for (int i = 0; i < MAX_PAST_PLAYERS; i++) {
      PastPlayer pp = g->pastPlayers[i];

      if (!pp.exists) {
        continue;
      }

      real32 scale = 0.4f;

      mat4 model = mat4d(1.0f);
      model = mat4Translate(model, pp.pos);
      model = mat4Translate(model, -1*vec3_one/2*scale);
      model = mat4Scale(model, vec3_one*scale);

      draw_3d_mesh(g->cubeMesh, model, texture(TEXTURE_test));
    }
  }

  // @Items render
  {
    vec3 worldOffset = vec3(-WORLD_SIZE/2, WORLD_SIZE/2, -WORLD_SIZE/2);
    vec3 modelOffset = vec3(0.5f, 0.0f, 0.5f);

    for (int f = 0; f < MAX_FACE; f++) {
      mat4 face = quatToMat4(faceRotations[f]);

      for (int y = 0; y < WORLD_CELL_COUNT; y++) {
        for (int x = 0; x < WORLD_CELL_COUNT; x++) {
          uint32 type = g->environment[f][y][x];
          if (type == BLOCK_NONE) {
            continue;
          }

          uint32 modelID = MAX_MODEL;
          uint32 textureID = MAX_TEXTURE;

          switch (type) {
            case BLOCK_ROCK:
              {
                modelID = MODEL_rock;
                textureID = TEXTURE_rock_albedo;
              } break;
            case BLOCK_COAL:
              {
                modelID = MODEL_coal;
                textureID = TEXTURE_coal_albedo;
              } break;
          }

          ensure(modelID != MAX_MODEL && textureID != MAX_TEXTURE);

          mat4 model = mat4d(1.0f);
          model = mat4Translate(model, worldOffset + modelOffset * WORLD_CELL_SIZE);
          model = mat4Translate(model, vec3(x, 0, y) * WORLD_CELL_SIZE);
          model = face * model;
          model = mat4Scale(model, vec3_one * WORLD_CELL_SIZE);

          draw_3d_model(model(modelID), model, texture(textureID));
        }
      }
    }
  }

  if (m == RENDER_MODE_NORMAL) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // @Clouds render
    for (int i = 0; i < CLOUD_COUNT; i++) {
      particleEmitter_render(g->clouds[i]);
    }

    // @UI render
    draw_2d_begin();

    char side[32] = "";

    switch (gameState_getCurrentFace(g)) {
      case BACK:
        {
          strcpy(side, "back side");
        } break;
      case FRONT:
        {
          strcpy(side, "front side");
        } break;
      case LEFT:
        {
          strcpy(side, "left side");
        } break;
      case RIGHT:
        {
          strcpy(side, "right side");
        } break;
      case TOP:
        {
          strcpy(side, "top side");
        } break;
      case BOTTOM:
        {
          strcpy(side, "bottom side");
        } break;
      default:
        {
          strcpy(side, "unknown side");
        } break;
    }

    draw_text(side, vec2(getWindowWidth()/2, 50.0f), 1.0f, font(FONT_renogare), vec3_white, TEXT_ALIGN_CENTER);

    ui_begin(font(FONT_quicksand_regular));

    {
      ui_toolbarBegin(uiid_gen());

      for (int i = 0; i < INVENTORY_SIZE; i++) {
        uint32 tex = MAX_TEXTURE;

        InventorySlot item = g->timeline.info.inventory.hotbar[i];

        switch (item.type) {
          case BLOCK_ROCK:
            {
              tex = TEXTURE_rock_icon;
            } break;
          case BLOCK_COAL:
            {
              tex = TEXTURE_coal_icon;
            } break;
          default:
            {
              tex = MAX_TEXTURE;
            } break;
        }

        ui_toolbarOption(
            uiid_genEx(i),
            i == g->timeline.info.inventory.currentSlot,
            item.count,
            tex);
      }

      ui_toolbarEnd();
    }

    ui_end();

    ui_draw();
  }
}

void gameState_update(State *state) {
  GameState* g = (GameState*) state->memory;
  Timeline* tb = &g->timeline;

  {
    vec3 cameraPos = CAMERA_POSITION;

    vec4 v = vec4(cameraPos.x, cameraPos.y, cameraPos.z, 1);

    mat4 rot = mat4d(1.0f) * quatToMat4(quatFromPitchYawRoll(CAMERA_ROTATION_OFFSET, 0, 0));
    rot = rot * quatToMat4(g->cameraRotation);

    v = rot * v;

    cameraPos = vec3FromVec4(v);

    for (int i = 0; i < CLOUD_COUNT; i++) {
      particleEmitter_update(&g->clouds[i], cameraPos);
    }
  }

  if (getTime() > g->timelineNextTickTime) {
    g->timelineNextTickTime = getTime() + TIMELINE_TICK_MS_INTERVAL;

    Action a;
    while (timeline_nextAction(tb, &g->timeIndex, &a)) {
      switch (a.type) {
        case SPAWN:
          {
            logln("SPAWNING!");
            int id = gameState_getFirstPastPlayer(g);
            if (id < 0) {
              logln("WARNING: too many active past players!");

              continue;
            }

            pastPlayer_init(&g->pastPlayers[id], tb, &g->timeIndex, a);
          } break;
        case TOUCH:
          {
            environment_handle(&g->environment, a.touch);
          } break;
        default:
          {
            // we don't need to do anything!
          } break;
      }
    }

    g->timeIndex.time += 1;
  }

  for (int i = 0; i < MAX_PAST_PLAYERS; i++) {
    if (!g->pastPlayers[i].exists) {
      continue;
    }

    pastPlayer_update(&g->pastPlayers[i], tb, &g->timeIndex);
  }

  gameState_setFaceDirections(g);

  for (int i = 0; i < 9; i++) {
    uint32 k = KEY_1 + i;

    if (keyJustDown(k)) {
      g->timeline.info.inventory.currentSlot = i;
    }
  }

  if (keyJustDown(KEY_tab)) {
    gameState_timeJump(g, 2);
  }

  if (keyJustDown(KEY_n)) {
    printFace(gameState_getCurrentFace(g));
  }

  if (keyJustDown(KEY_l)) {
    logln("saving!");
    timeline_save(tb, g->timeIndex);
  }

  if (g->rotating) {
    bool done = false;

    real32 pc = 1.0f - ((float) ((g->rotatingStartTime + ROTATION_DURATION) - getTime())) / ROTATION_DURATION;

    if (getTime() > g->rotatingStartTime + ROTATION_DURATION) {
      pc = 1.0f;
      g->rotating = false;

      done = true;
    }

    pc = pc * pc;

    g->cameraRotation = quatFromPitchYawRoll(
        g->rotatingPitchEnd * pc,
        g->rotatingYawEnd * pc,
        g->rotatingRollEnd * pc) * g->rotatingStart;

    g->cameraRotation = quatNormalize(g->cameraRotation);

    if (done) {
      g->timeline.info.camera = g->cameraRotation;

      g->timeline.info.up = g->playerUp;
      g->timeline.info.right = g->playerRight;
      g->timeline.info.forward = g->playerForward;
    }
  } else {
    real32 speed = 3.0f;
    real32 dt = getDt();

    vec3 right = g->playerRight;
    vec3 forward = g->playerForward;

    vec3 move = vec3_zero;

    if (keyDown(KEY_a)) {
      move -= right;
    }

    if (keyDown(KEY_d)) {
      move += right;
    }

    if (keyDown(KEY_w)) {
      move += forward;
    }

    if (keyDown(KEY_s)) {
      move -= forward;
    }

    g->playerPos += move * dt * speed;

    if (!vec3AlmostEqual(move, g->playerLastMove)) {
      timeline_add(tb, &g->timeIndex, action_makeMove(g->playerPos));

      g->playerLastMove = move;
    }

    if (keyJustDown(KEY_shift)) {
      vec3 realRight = g->faceRight;
      vec3 realForward = g->faceForward;

      real32 rightLen = vec3Sum(g->playerPos * realRight);
      real32 forwardLen = vec3Sum(g->playerPos * realForward);

      uint32 direction = ROT_IDLE;

      if (fabs(rightLen) > WORLD_SIZE/2) {
        direction = (rightLen > 0) ? ROT_RIGHT : ROT_LEFT;
      } else if (fabs(forwardLen) > WORLD_SIZE/2) {
        direction = (forwardLen > 0) ? ROT_FORWARD : ROT_BACKWARD;
      }

      if (direction != ROT_IDLE) {
        gameState_rotate(g, direction);
      }
    }

    g->playerPos.x = clamp(g->playerPos.x, -WORLD_SIZE/2, WORLD_SIZE/2);
    g->playerPos.y = clamp(g->playerPos.y, -WORLD_SIZE/2, WORLD_SIZE/2);
    g->playerPos.z = clamp(g->playerPos.z, -WORLD_SIZE/2, WORLD_SIZE/2);

    if (keyJustDown(KEY_space)) {
      int face = gameState_getCurrentFace(g);

      vec3 right = faceCardinalDirections[face][DIRECTION_RIGHT];
      vec3 forward = faceCardinalDirections[face][DIRECTION_FORWARD];

      real32 rx = vec3Sum(g->playerPos * right) + WORLD_SIZE/2;
      real32 ry = vec3Sum(g->playerPos * forward) * -1 + WORLD_SIZE/2;

      int x = (int) (rx / WORLD_CELL_SIZE);
      int y = (int) (ry / WORLD_CELL_SIZE);

      gameState_touchEnvironment(g, face, x, y);
    }
  }

  glBindFramebuffer(GL_FRAMEBUFFER, g->shadowFBO);
  gameState_render(g, RENDER_MODE_LIGHT);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  gameState_render(g, RENDER_MODE_NORMAL);
}
