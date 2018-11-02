# Lornock

a time travelling farming game which takes place on an asteroid.

## Potentially Interesting Files:

- OBJ model and font loader: https://github.com/paked/lornock/blob/master/src/assets.cpp
- Simple but flexible Immediate Mode GUI implementation: https://github.com/paked/lornock/blob/master/src/ui.cpp
- Super basic font packer built on stb_truetype: https://github.com/paked/lornock/blob/master/fbake/fbake.cpp
- Quake style `MemoryArena`s: https://github.com/paked/lornock/blob/master/src/memory.hpp (see in action: https://github.com/paked/lornock/blob/master/src/entities/timeline.cpp)

## Build Instructions

### Linux

1. Install SDL2, SDL_image, and SDL_ttf packages from your package manager
2. In this directory `mkdir build && cd build`
3. `cmake ..`
4. `make`
5. `./game`

### Windows

1. Download SDL2 the runtime devel "VC" files for SDL2, SDL_image, and SDL_ttf
2. Place the extracted folders in a common directory (I'm currently using C:\dev).
3. Open command prompt
4. Go to the source directory, create a new folder which you want to build out of.
5. Type `..\build.bat`. This will create a new visual studio 2017 project which you can open and continue as normal. If you are using different versions of SDL to me, OR have a different directory where you put the runtime files you will need to edit the paths in `build.bat`
6. Copy relevant library files into `pwd` for when you run the executable

### OSX

Unknown. Should work fine, may need to make adjustments to the Cmakelists file.
