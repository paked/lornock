#pragma once

// TODO: refactor the non-platform related stuff into a separate file

#include <cstdio>

// TODO: come up with a nicer name for dbg_assert
#define dbg_assert(b) if (!(b)) { logfln("ASSERT FAILED: \"%s\" at \"%d\" \"%s\"", #b, __LINE__, __FILE__); }

#define log(...) fprintf(stderr, __VA_ARGS__)
#define logln(fmt) fprintf(stderr, fmt "\n")
#define logfln(fmt, ...) fprintf(stderr, fmt "\n", __VA_ARGS__)

#define bytes(n)     (n)
#define kilobytes(n) (bytes(n)*1024)
#define megabytes(n) (kilobytes(n)*1024)
#define gigabytes(n) (megabytes(n)*1024)

#define PI  3.1415926535897
#define PIf 3.1415926f

#define rad2Deg(a) ((a) * (180/PIf))
#define deg2Rad(a) ((a) * (PIf/180))

typedef int8_t    int8;
typedef int16_t   int16;
typedef int32_t   int32;
typedef int64_t   int64;
typedef uint8_t   uint8;
typedef uint16_t  uint16;
typedef uint32_t  uint32;
typedef uint64_t  uint64;
typedef float     real32;
typedef double    real64;
typedef int32     bool32;

// TODO(harrison): Add in rest of keys!
enum {
  KEY_unknown,

  KEY_0, KEY_1, KEY_2, KEY_3, KEY_4,
  KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,

  KEY_shift, KEY_grave, KEY_ctrl,

  KEY_a,
  KEY_b,
  KEY_c,
  KEY_d,
  KEY_e,
  KEY_f,
  KEY_g,
  KEY_h,
  KEY_i,
  KEY_j,
  KEY_k,
  KEY_l,
  KEY_m,
  KEY_n,
  KEY_o,
  KEY_p,
  KEY_q,
  KEY_r,
  KEY_s,
  KEY_t,
  KEY_u,
  KEY_v,
  KEY_w,
  KEY_x,
  KEY_y,
  KEY_z,

  MAX_KEY
};

struct LornockMemory {
  bool initialized;

  uint64 permanentStorageSize;
  void* permanentStorage;

  uint64 transientStorageSize;
  void* transientStorage;
};

// We cast to uint64 to prevent issues where 32 bit constants wrap around.
#define LORNOCK_PERMANENT_MEMORY_STORAGE_SIZE megabytes((uint64) 2)
#define LORNOCK_TRANSIENT_MEMORY_STORAGE_SIZE megabytes((uint64) 8)

typedef void* (* OpenGLLoadProcFunc)(const char *name);
typedef void (* LoadFromFileFunc)(const char *path, void** data, uint32* len);
typedef void (* WriteToFileFunc)(const char *path, void* data, int64 len);

struct Platform {
  int fps;

  bool quit;

  bool keyStateNow[MAX_KEY];
  bool keyStateLast[MAX_KEY];

  WriteToFileFunc writeToFile;
  LoadFromFileFunc loadFromFile;
  OpenGLLoadProcFunc glLoadProc;

  uint32 windowWidth;
  uint32 windowHeight;
  bool fullscreen;

  // current time in ms
  uint32 time;
  real32 deltaTime;
};

#define getTime() (platform->time)
#define getDt() (platform->deltaTime)
#define getWindowWidth() (platform->windowWidth)
#define getWindowHeight() (platform->windowHeight)

#define loadFromFile(p, d, l) platform->loadFromFile(p, d, l)
#define writeToFile(p, d, l) platform->writeToFile(p, d, l)

#define keyJustDown(k) (platform->keyStateNow[k] && !platform->keyStateLast[k])
#define keyJustUp(k) (!platform->keyStateNow[k] && platform->keyStateLast[k])
#define keyDown(k) (platform->keyStateNow[k])
#define keyUp(k) (!platform->keyStateNow[k])

#define cubeMesh { \
     0.0f,  0.0f,  0.0f,  0.0f, 0.0f, /* back */ \
     1.0f,  0.0f,  0.0f,  1.0f, 0.0f, \
     1.0f,  1.0f,  0.0f,  1.0f, 1.0f, \
     1.0f,  1.0f,  0.0f,  1.0f, 1.0f, \
     0.0f,  1.0f,  0.0f,  0.0f, 1.0f, \
     0.0f,  0.0f,  0.0f,  0.0f, 0.0f, \
\
     0.0f,  0.0f,  1.0f,  0.0f, 0.0f, /* front */ \
     1.0f,  0.0f,  1.0f,  1.0f, 0.0f, \
     1.0f,  1.0f,  1.0f,  1.0f, 1.0f, \
     1.0f,  1.0f,  1.0f,  1.0f, 1.0f, \
     0.0f,  1.0f,  1.0f,  0.0f, 1.0f, \
     0.0f,  0.0f,  1.0f,  0.0f, 0.0f, \
\
     0.0f,  1.0f,  1.0f,  1.0f, 0.0f, /* left */ \
     0.0f,  1.0f,  0.0f,  1.0f, 1.0f, \
     0.0f,  0.0f,  0.0f,  0.0f, 1.0f, \
     0.0f,  0.0f,  0.0f,  0.0f, 1.0f, \
     0.0f,  0.0f,  1.0f,  0.0f, 0.0f, \
     0.0f,  1.0f,  1.0f,  1.0f, 0.0f, \
\
     1.0f,  1.0f,  1.0f,  1.0f, 0.0f, /* right */ \
     1.0f,  1.0f,  0.0f,  1.0f, 1.0f, \
     1.0f,  0.0f,  0.0f,  0.0f, 1.0f, \
     1.0f,  0.0f,  0.0f,  0.0f, 1.0f, \
     1.0f,  0.0f,  1.0f,  0.0f, 0.0f, \
     1.0f,  1.0f,  1.0f,  1.0f, 0.0f, \
\
     0.0f,  0.0f,  0.0f,  0.0f, 1.0f, /* bottom */ \
     1.0f,  0.0f,  0.0f,  1.0f, 1.0f, \
     1.0f,  0.0f,  1.0f,  1.0f, 0.0f, \
     1.0f,  0.0f,  1.0f,  1.0f, 0.0f, \
     0.0f,  0.0f,  1.0f,  0.0f, 0.0f, \
     0.0f,  0.0f,  0.0f,  0.0f, 1.0f, \
\
     0.0f,  1.0f,  0.0f,  0.0f, 1.0f, /* top */ \
     1.0f,  1.0f,  0.0f,  1.0f, 1.0f, \
     1.0f,  1.0f,  1.0f,  1.0f, 0.0f, \
     1.0f,  1.0f,  1.0f,  1.0f, 0.0f, \
     0.0f,  1.0f,  1.0f,  0.0f, 0.0f, \
     0.0f,  1.0f,  0.0f,  0.0f, 1.0f \
}

real32 clamp(real32 v, real32 min, real32 max) {
  if (v < min) {
    return min;
  } else if (v > max) {
    return max;
  }

  return v;
}
