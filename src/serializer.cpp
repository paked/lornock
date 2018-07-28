#define SERIALIZER_MODE_WRITE (false)
#define SERIALIZER_MODE_READ (true)

#define SERIALIZER_BUFFER_CHUNK_SIZE (kilobytes(512))

// TODO(harrison): Consider un-define-mushing this chunk of code. While I like being able to add new types super easily, I think there is definitely room to abstract logic from the read/write functions into other places. I want to make the defines themselves as small as possible.

struct Serializer {
  bool reading;

  MemoryArena buffer;

#define serializer(Type) \
  void (* Type ## Read) (Serializer*, Type *);\
  void (* Type ## Write) (Serializer*, Type);

#include <serializer_types.cpp>

#undef serializer

  // NOTE(harrison): Used for reading only.
  MemoryIndex tip;
};

#define serializer(Type) \
void serializer_binary_ ## Type ## Write(Serializer* s, Type u) { \
  MemoryBlock* b = memoryArena_getLastBlock(&s->buffer); \
\
  if (b == 0) { \
    memoryArena_pushSize(&s->buffer, SERIALIZER_BUFFER_CHUNK_SIZE); \
\
    b = memoryArena_getLastBlock(&s->buffer); \
\
    assert(b != 0); \
  } \
\
  MemoryIndex blockSize = b->size - sizeof(MemoryBlock); \
\
  if (b->used + sizeof(Type) > blockSize) {\
    memoryArena_pushSize(&s->buffer, SERIALIZER_BUFFER_CHUNK_SIZE);\
\
    b = memoryArena_getLastBlock(&s->buffer); \
\
    assert(b != 0); \
  } \
\
  uint8* addr = b->start + b->used; \
\
  void* loc = (void*) addr; \
\
  Type* pt = (Type*) loc; \
\
  *pt = u; \
\
  b->used += sizeof(Type); \
} \
\
void serializer_binary_ ## Type ## Read(Serializer* s, Type * u) { \
  MemoryBlock* b = memoryArena_getLastBlock(&s->buffer); \
\
  assert(b != 0); \
\
  uint8* addr = b->start + s->tip; \
\
  void* loc = (void*) addr; \
\
  Type* ptr = (Type*) loc;\
\
  *u = *ptr;\
\
  s->tip += sizeof(Type);\
} \
\
void serializer_ ## Type (Serializer* s, Type* v) { \
  if (s->reading) { \
    s-> Type ## Read(s, v); \
\
    return;\
  } \
\
  s-> Type ## Write(s, *v);\
}\

#include <serializer_types.cpp>

#undef serializer

void serializer_init(Serializer* s, bool reading, MemoryArena b) {
  s->reading = reading;

  s->buffer = b;

#define serializer(Type) \
  s-> Type ## Read = serializer_binary_ ## Type ## Read; \
  s-> Type ## Write = serializer_binary_ ## Type ## Write;

#include <serializer_types.cpp>
#undef serializer

  s->tip = 0;
}
