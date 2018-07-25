// Handle Include external libraries

#include <sys/mman.h> // mmap
#include <sys/stat.h> // stat
#include <dlfcn.h> // dlopen,dlsym
#include <unistd.h> // read
#include <fcntl.h>
#include <errno.h>

void* linux_allocateMemory(uint64 size) {
  return mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
}

// File IO
void linux_loadFromFile(const char* path, void** data, uint32* len) {
  *data = 0;
  *len = 0;

  struct stat attr;

  if (stat(path, &attr) != 0) {
    logfln("ERROR: failed to find file: %s", path);
    return;
  }

  int64 readSizeExpected = attr.st_size;

  int fd = open(path, O_RDONLY);
  if (fd == -1) {
    logfln("ERROR: unable to open file. fd: %d", fd);
  }

  void* readData = mmap(0, readSizeExpected, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  int64 readSizeGot = read(fd, readData, readSizeExpected);

  if (readSizeGot != readSizeExpected) {
    logfln("ERROR: could not read file (wanted: %zd read: %zd) (errno: %d)!", readSizeExpected, readSizeGot, errno);
  }

  *data = readData;
  *len = (uint32) readSizeExpected;

  close(fd);
}

void linux_writeToFile(const char* path, void* data, int64 len) {
  /*logfln("writing to file %s. contents:\n%s", path, data);*/

  int fd = open(path, O_RDWR | O_CREAT);
  if (fd == -1) {
    logfln("ERROR: unable to open file. fd: %d", fd);
  }

  int64 writeSizeGot = write(fd, data, len);

  if (writeSizeGot != len) {
    logfln("ERROR: could not write file (wanted: %zd read: %zd) (errno: %d)!", len, writeSizeGot, errno);
  }

  logln("Wrote file!");

  close(fd);
}

/*
struct GameCode {
  char* path;
} gameCode = { 0 };*/

// Hot reloading
const char* linux_libPath = "./liblornock.so";
bool linux_libValid = false;
time_t linux_libFileTime;
void* linux_libHandle = 0;

ino_t linux_getFileID(const char *fname) {
  struct stat attr;

  if (stat(fname, &attr) != 0) {
    logfln("ERROR: Can't get file ID of '%s'", fname);

    attr.st_ino = 0;
  }

  return attr.st_ino;
}

time_t linux_getFileTime(const char *fname) {
  struct stat attr;

  if (stat(fname, &attr) != 0) {
    logfln("ERROR: Can't get file ID of '%s'", fname);

    attr.st_mtime = 0;
  }

  return attr.st_mtime;
}

bool linux_libNeedsReloading() {
  return linux_getFileTime(linux_libPath) > linux_libFileTime;
}

void linux_libUnload() {
  if (linux_libHandle == 0) {
    return;
  }

  dlclose(linux_libHandle);

  linux_libValid = false;
  linux_libHandle = 0;
  linux_libFileTime = 0;
}

bool linux_libReload() {
  if (linux_libHandle != 0) {
    SDL_Delay(200);
    linux_libUnload();
  }

  linux_libHandle = dlopen(linux_libPath, RTLD_NOW | RTLD_LOCAL);

  if (linux_libHandle == 0) {
    logln("ERROR: Cannot load game liblornock");

    return false;
  }

  gameLibUpdateFunction = (GameLibUpdateFunction) dlsym(linux_libHandle, "lornock_update");
  if (!gameLibUpdateFunction) {
    logln("ERROR: Could not find update function");

    return false;
  }

  gameLibInitFunction = (GameLibInitFunction) dlsym(linux_libHandle, "lornock_init");
  if (!gameLibUpdateFunction) {
    logln("ERROR: Could not find init function");

    return false;
  }

  gameLibCleanFunction = (GameLibCleanFunction) dlsym(linux_libHandle, "lornock_clean");
  if (!gameLibCleanFunction) {
    logln("ERROR: Could not find clean function");

    return false;
  }

  linux_libValid = true;
  linux_libFileTime = linux_getFileTime(linux_libPath);

  return true;
}

bool linux_libIsValid() {
  return linux_libValid;
}

bool linux_libExists() {
  return linux_getFileID(linux_libPath) != 0;
}
