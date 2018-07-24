#include <windows.h>

void* win32_allocateMemory(uint64 size) {
  return VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

void win32_loadFromFile(const char* path, void** data, uint32* len) {
  logln("LOADING FROM FILE");

  // TODO(harrison): fill me in!
}

void win32_writeToFile(const char* path, void* data, int64 len) {
  logln("WRITING FILE");

  // TODO(harrison): fill me in!
}

bool win32_getFileTime(const char* fname, FILETIME* ft) {
  WIN32_FIND_DATA find_data;
  HANDLE find_handle = FindFirstFileA(filename, &find_data);
  if(find_handle == INVALID_HANDLE_VALUE) {
    logln("ERROR: could not get filetime");

    return false;
  }

  FindClose(find_handle);
  *ft = find_data.ftLastWriteTime;

  return true;
}

const char* win32_libPath = "Debug\\lornock.dll"

bool win32_libValid = false;
FILETIME win32_libFileTime = 0;
HMODULE win32_libHandle = 0;

bool win32_libNeedsReloading() {
  return false;
}

bool win32_libReload() {
  return false;
}

bool win32_libIsValid() {
  return win32_libValid;
}

bool win32_libExists() {
  FILETIME ft;

  return win32_getFileTime(win32_libPath, &ft);
}
