# Lornock

a time travelling farming game which takes place on an asteroid.

## Build Instructions

### Emscripten (web)

1. Install Emscripten somewhere (follow an online guide for that)
2. Run `source ./emsdk_env.sh`
3. Return to project dir, `mkdir build && cd build`
4. `emcmake cmake ..`
5. `make`
6. Start a HTTP server with your favourite web server util (eg `python3 -m http.server`)
7. Open up `localhost:XXXX/game.html` in your browser. Replace XXXX with whatever your port is.

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
