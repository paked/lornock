#include <windows.h>

void* win32_allocateMemory(uint64 size) {
  return VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

bool win32_freeMemory(void* addr, MemoryIndex size) {
  return VirtualFree(addr, size, MEM_DECOMMIT);
}

void win32_writeToFile(const char *path, void *data, int64 data_len) {
    HANDLE file = {};
    char error_msg[64] = {};
    
    {
        DWORD desired_access = GENERIC_WRITE;
        DWORD share_mode = 0;
        SECURITY_ATTRIBUTES security_attributes = {
            (DWORD)sizeof(SECURITY_ATTRIBUTES),
            0,
            0,
        };
        DWORD creation_disposition = CREATE_ALWAYS;
        DWORD flags_and_attributes = 0;
        HANDLE template_file = 0;
        
        if((file = CreateFile(path, desired_access, share_mode, &security_attributes, creation_disposition, flags_and_attributes, template_file)) != INVALID_HANDLE_VALUE) {
            void *data_to_write = data;
            DWORD data_to_write_size = (DWORD)data_len;
            DWORD bytes_written = 0;
            OVERLAPPED overlapped = {};
            
            WriteFile(file, data_to_write, data_to_write_size, &bytes_written, &overlapped);
            
            CloseHandle(file);
        }
        else {
            snprintf(error_msg, 64, "Could not save to \"%s\"", path);
            logfln("File I/O Error %s", error_msg);
        }
    }
}

void win32_loadFromFile(const char *data_path, void **data, uint32* data_len) {
      *data = 0;
    *data_len = 0;
    
    HANDLE file = {};
    char error_msg[64] = {};
    
    {
        DWORD desired_access = GENERIC_READ;
        DWORD share_mode = 0;
        SECURITY_ATTRIBUTES security_attributes = {
            (DWORD)sizeof(SECURITY_ATTRIBUTES),
            0,
            0,
        };
        DWORD creation_disposition = OPEN_EXISTING;
        DWORD flags_and_attributes = 0;
        HANDLE template_file = 0;
        
        if((file = CreateFile(data_path, desired_access, share_mode, &security_attributes, creation_disposition, flags_and_attributes, template_file)) != INVALID_HANDLE_VALUE) { 
            
            DWORD read_bytes = GetFileSize(file, 0);
            void *read_data = VirtualAlloc(0, read_bytes,
                                           MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            DWORD bytes_read = 0;
            OVERLAPPED overlapped = {};
            
            ReadFile(file, read_data, read_bytes, &bytes_read, &overlapped);
            
            *data = read_data;
            *data_len = (uint32)bytes_read;
            
            CloseHandle(file);
        }
        else {
                snprintf(error_msg, 64, "Could not read from \"%s\"", data_path);
                logfln("File I/O Error %s", error_msg);
        }
    }
}

bool win32_getFileTime(const char* fname, FILETIME* ft) {
  WIN32_FIND_DATA find_data;
  HANDLE find_handle = FindFirstFileA(fname, &find_data);
  if(find_handle == INVALID_HANDLE_VALUE) {
    logln("ERROR: could not get filetime");

    return false;
  }

  FindClose(find_handle);
  *ft = find_data.ftLastWriteTime;

  return true;
}

const char* win32_libPath = "lornock.dll";

bool win32_libValid = false;
FILETIME win32_libFileTime = {0};
HMODULE win32_libHandle = 0;

bool win32_libNeedsReloading() {
  return false;
}

bool win32_libReload() {
  win32_libHandle = LoadLibraryA(win32_libPath);

  if (win32_libHandle == 0) {
    logln("ERROR: Unable to load Lornock DLL");

    return false;
  }

  gameLibInitFunction = (GameLibInitFunction) GetProcAddress(win32_libHandle, "lornock_init");
  gameLibUpdateFunction = (GameLibUpdateFunction) GetProcAddress(win32_libHandle, "lornock_update");
  gameLibCleanFunction = (GameLibCleanFunction) GetProcAddress(win32_libHandle, "lornock_clean");

  win32_libValid = (gameLibInitFunction != 0 && gameLibUpdateFunction != 0 && gameLibCleanFunction != 0);

  logfln("%p", gameLibInitFunction);

  return win32_libValid;
}

bool win32_libIsValid() {
  return win32_libValid;
}

bool win32_libExists() {
  FILETIME ft;

  return win32_getFileTime(win32_libPath, &ft);
}
