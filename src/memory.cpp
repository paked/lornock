struct MemoryBlock {
  uint8* base;
  uint8* start;
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

#define memoryArena_pushStruct(arena, Type) ( (Type *)memoryArena_pushSize(arena, sizeof(Type)) )

void* memoryArena_pushSize(MemoryArena* ma, MemoryIndex size) {
  uint8* end;
  MemoryBlock* last = 0;

  if (ma->first == 0) {
    end = ma->base;
  } else {
    MemoryBlock* i = ma->first;
    while (i != 0) {
      last = i;

      i = i->next;
    } 

    assert(last != 0);

    end = last->base + last->size;
  }

  MemoryBlock* block = (MemoryBlock*) end;
  block->base = end;
  block->start = end + sizeof(MemoryBlock);
  block->size = sizeof(MemoryBlock) + size;
  block->next = 0;

  if (ma->first == 0) {
    ma->first = block;
  } else {
    last->next = block;
  }

  return (void*) block->start;
}
