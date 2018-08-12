#define PARTICLE_CLOUD_W (3.0f)
#define PARTICLE_CLOUD_D (2.0f)
#define PARTICLE_CLOUD_H (0.5f)

#define PARTICLE_MIN_SIZE (0.1f)
#define PARTICLE_MAX_SIZE (0.5f)

#define PARTICLE_COUNT (70)
struct Particle {
  real32 x, y, z;

  real32 scale;

  real32 alpha;

  // For depth sorting
  real32 distanceFromCameraSquared;
};

struct ParticleEmitter {
  real32 x, y, z;

  Particle particles[PARTICLE_COUNT];

  uint32 particleShape;
  uint32 colorMap;
};

ParticleEmitter particleEmitter_init(real32 x, real32 y, real32 z) {
  ParticleEmitter pe;

  pe.x = x;
  pe.y = y;
  pe.z = z;

  return pe;
}

void particleEmitter_generateEllipsoidVolume(ParticleEmitter* pe, real32 w, real32 h, real32 d) {
  for (int i = 0; i < PARTICLE_COUNT; i++) {
    Particle p;

    p.x = (rand01() * 2 - 1) * w;
    p.y = (rand01() * 2 - 1) * h;
    p.z = (rand01() * 2 - 1) * d;

    // Particles must be between 0.5 and 1.0f
    p.scale = rand01() * 0.5f + 0.5f;

    p.alpha =
      (p.x * p.x)/(w)*(w) +
      (p.y * p.y)/(h)*(h) +
      (p.z * p.z)/(d)*(d);

    if (p.alpha >= 1) {
      // This particle doesn't fit within the ellipsoid, get another one.
      i -= 1;

      continue;
    }

    pe->particles[i] = p;
  }
}

void particleEmitter_generateSphereSurface(ParticleEmitter* pe, real32 r) {
  for (int i = 0; i < PARTICLE_COUNT; i++) {
    Particle p;

    real32 x = rand01() * r * 2;
    real32 y = rand01() * r * 2;
    real32 z = rand01() * r * 2;

    x *= (rand01() < 0.5 ? -1 : 1);
    y *= (rand01() < 0.5 ? -1 : 1);
    z *= (rand01() < 0.5 ? -1 : 1);

    if (vec3LengthSquared(vec3(x, y, z)) < r*r) {
      i -= 1;

      continue;
    }

    p.x = x;
    p.y = y;
    p.z = z;

    p.scale = randFromTo(0.5f, 1.0f);

    p.alpha = 1.0f;

    pe->particles[i] = p;
  }
}

void particleEmitter_update(ParticleEmitter* pe, vec3 cameraPos) {
  for (int i = 0; i < PARTICLE_COUNT; i++) {
    Particle* p = &pe->particles[i];

    vec3 pos = (vec3(p->x, p->y, p->z)) + vec3(pe->x, pe->y, pe->z);

    p->distanceFromCameraSquared = vec3LengthSquared(pos - cameraPos);
  }

  // NOTE(harrison): We use a simple bubble sort instead of something more
  // complicated here for two reasons:
  //
  // 1. Most of the time the array will be PRETTY MUCH sorted.  Bubblesort is
  // good at quickly switching two items in place without much
  // overhead
  // 2. It doesn't require us to use any external memory buffers.  Makes things
  // a lot simpler.
  //
  // If I come back and benchmark this and it turns out to be a bottleneck,
  // well then... We'll change it to something else.
  //
  // TODO(harrison): if we don't use depth mask do we even need this sorting?
  
  /*
  Particle temp;
  int i = 0;
  int j = 0;

  while (i < PARTICLE_COUNT) {
    j = 0;
    while (j < i) {
      if (pe->particles[j].distanceFromCameraSquared > pe->particles[i].distanceFromCameraSquared) {
        temp = pe->particles[j];
        pe->particles[j] = pe->particles[i];
        pe->particles[i] = temp;
      }

      j++;
    }

    i++;
  }*/
}

void particleEmitter_render(ParticleEmitter pe) {
  draw_setShader(shader(SHADER_particle));

  shader_setMatrix(&draw.activeShader, "view", draw.view);
  shader_setMatrix(&draw.activeShader, "projection", draw.projection);

  shader_setInt(&draw.activeShader, "ourTexture", 0);
  shader_setInt(&draw.activeShader, "colorMap", 1);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture(pe.particleShape).id);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, texture(pe.colorMap).id);

  glBindVertexArray(draw.quadVAO);

  mat4 model = mat4d(1.0f);
  vec3 pos;

  glDepthMask(GL_FALSE);
  for (int i = PARTICLE_COUNT - 1; i >= 0; i--) {
    Particle p = pe.particles[i];

    pos = vec3(p.x, p.y, p.z) + vec3(pe.x, pe.y, pe.z);

    model = mat4Translate(mat4d(1.0f), pos);
    model = mat4Translate(model, vec3(0.5f, 0.5f, 0.0f) * -1);

    shader_setMatrix(&draw.activeShader, "model", model);
    shader_setVec2(&draw.activeShader, "scale", vec2(p.scale, p.scale));

    glDrawArrays(GL_TRIANGLES, 0, 6);
  }

  glDepthMask(GL_TRUE);

  glBindVertexArray(0);
}
