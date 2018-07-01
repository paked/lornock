#pragma once

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

typedef void* (* OpenGLLoadProc)(const char *name);
typedef void (* LoadFromFile)(const char *path, void** data, uint32* len);

struct Platform {
  int fps;

  bool quit;

  LoadFromFile loadFromFile;
  OpenGLLoadProc glLoadProc;
};

#define loadFromFile(p, d, l) platform->loadFromFile(p, d, l);

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
