#pragma once

#include <cstdint>
#include <cstdio>
#include <string.h>

#define log(...) fprintf(stderr, __VA_ARGS__)
#define logln(fmt) fprintf(stderr, fmt "\n")
#define logfln(fmt, ...) fprintf(stderr, fmt "\n", __VA_ARGS__)

#define bytes(n)     (n)
#define kilobytes(n) (bytes(n)*1024)
#define megabytes(n) (kilobytes(n)*1024)
#define gigabytes(n) (megabytes(n)*1024)

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

typedef size_t MemoryIndex;

// TODO(harrison): Add in rest of keys!
enum {
  KEY_unknown,

  KEY_0, KEY_1, KEY_2, KEY_3, KEY_4,
  KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,

  KEY_shift, KEY_grave, KEY_ctrl, KEY_tab,

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
#define LORNOCK_PERMANENT_MEMORY_STORAGE_SIZE megabytes((uint64) 8)
#define LORNOCK_TRANSIENT_MEMORY_STORAGE_SIZE megabytes((uint64) 8)

struct MemoryArena;

typedef void* (* OpenGLLoadProcFunc)(const char *name);
typedef void (* LoadFromFileFunc)(const char *path, void** data, uint32* len);
typedef void (* LoadFromFileAsArenaFunc)(const char *path, MemoryArena* ma);
typedef void (* WriteToFileFunc)(const char *path, void* data, int64 len);
typedef void (* WriteArenaToFileFunc)(const char *path, MemoryArena* ma);
typedef void (* ShowErrorBoxFunc)(const char* title, const char* fmt, ...);

struct Platform {
  int fps;

  bool quit;

  bool keyStateNow[MAX_KEY];
  bool keyStateLast[MAX_KEY];

  LoadFromFileFunc loadFromFile;
  LoadFromFileAsArenaFunc loadFromFileAsArena;
  WriteToFileFunc writeToFile;
  WriteArenaToFileFunc writeArenaToFile;

  ShowErrorBoxFunc showErrorBox;

  OpenGLLoadProcFunc glLoadProc;

  uint32 windowWidth;
  uint32 windowHeight;
  bool fullscreen;

  // current time in ms
  uint32 time;
  real32 deltaTime;
};
