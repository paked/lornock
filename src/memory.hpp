// TODO(harrison): Add temporary memory faculties

struct MemoryBlock {
  uint8* base;
  uint8* start;

  MemoryIndex used;
  MemoryIndex size;
  MemoryBlock* next;
};

struct MemoryArena {
  uint8* base;
  MemoryIndex size;

  MemoryBlock* first;
};

void memoryArena_init(MemoryArena* ma, MemoryIndex size, uint8* base) {
  ma->size = size;
  ma->base = base;
  ma->first = 0;

  logfln("creating memory arena of size %zu", size);
}

void memoryArena_clear(MemoryArena* ma) {
  memset(ma->base, 0, ma->size);

  ma->first = 0;
}

MemoryBlock* memoryArena_getLastBlock(MemoryArena* ma) {
  MemoryBlock* last = 0;

  MemoryBlock* i = ma->first;
  while (i != 0) {
    last = i;

    i = i->next;
  }

  return last;
}

#define memoryArena_pushStruct(arena, Type) ( (Type *)memoryArena_pushSize(arena, sizeof(Type)) )
#define memoryArena_pushArray(arena, count, Type) ( (Type *) memoryArena_pushSize(arena, (count) * sizeof(Type)) )

void* memoryArena_pushSize(MemoryArena* ma, MemoryIndex size) {
  uint8* end;
  MemoryBlock* last = 0;
  MemoryIndex totalSize = 0;

  if (ma->first == 0) {
    end = ma->base;
  } else {
    MemoryBlock* i = ma->first;
    while (i != 0) {
      last = i;
      totalSize += i->size;

      i = i->next;
    } 

    ensure(last != 0);

    end = last->base + last->size;
  }

  MemoryIndex blockSize = sizeof(MemoryBlock) + size;

  ensure(totalSize + blockSize < ma->size);

  MemoryBlock* block = (MemoryBlock*) end;
  block->base = end;
  block->start = end + sizeof(MemoryBlock);
  block->size = blockSize;
  block->next = 0;
  block->used = 0;

  if (ma->first == 0) {
    ma->first = block;
  } else {
    last->next = block;
  }

  return (void*) block->start;
}
