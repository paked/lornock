cmake_minimum_required(VERSION 3.5.1)
project(lornock_game)

#
# Vendored code
#

file(GLOB GLAD_INCLUDE_DIRS vendor/glad/include)
file(GLOB GLAD_SOURCES vendor/glad/src/glad.c)

file(GLOB STB_IMAGE_INCLUDE_DIRS vendor/stb)

file(GLOB HANDMADE_MATH_INCLUDE_DIRS vendor/HandmadeMath)

#
# Lornock game code (liblornock.so/lornock.dll)
#

file(GLOB LIB_SOURCES
    src/lornock.cpp)

add_library(lornock SHARED
    ${GLAD_SOURCES}
    ${LIB_SOURCES})

target_include_directories(lornock PRIVATE
    src

    ${STB_IMAGE_INCLUDE_DIRS}
    ${HANDMADE_MATH_INCLUDE_DIRS})

#
# Lornock runner (game/game.exe)
#

# Put source files in this glob
file(GLOB GAME_SOURCES
    src/sdl_main.cpp

    ${GLAD_SOURCES})

add_executable(game ${GAME_SOURCES})

include_directories(
    src

    ${GLAD_INCLUDE_DIRS})

#
# Handle assets
#

set(DATA_FILES
    data/shaders/default.vert
    data/shaders/default.frag
    data/shaders/billboard.vert
    data/shaders/billboard.frag
    data/shaders/color.vert
    data/shaders/color.frag

    data/mesh/rock.obj

    data/img/player.png
    data/img/rock.png
    data/img/rock_albedo.png
    data/img/test.png)

foreach(DATA_FILE ${DATA_FILES})
    set(DATA_IN ${CMAKE_CURRENT_SOURCE_DIR}/${DATA_FILE})
    set(DATA_OUT ${CMAKE_CURRENT_BINARY_DIR}/${DATA_FILE})

    # If the file at DATA_IN does not exist or is changed, copy it into the build directory
    configure_file(${DATA_IN} ${DATA_OUT} COPYONLY)
endforeach(DATA_FILE)

add_custom_command(TARGET lornock PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/data/saves" "${CMAKE_BINARY_DIR}/data/saves"
    COMMENT "copying over saves"
    VERBATIM)

# Runs if building for a native platform (currently only Linux)
if ((WIN32 OR (UNIX AND NOT APPLE)) AND NOT EMSCRIPTEN)
    # TODO(harrison): figure out exactly what this is for...
    if(NOT WIN32)
        set(GLAD_LIBRARIES dl)

        # We use a bit of a basterdised version of C++11, but it's C++11 no less
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 -g -Wall -Werror")
    endif()

	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
	find_package(SDL2 REQUIRED)

	include_directories(game ${SDL2_INCLUDE_DIR})

	target_link_libraries(game ${SDL2_LIBRARY}
                         ${GLAD_LIBRARIES})
#[[ 
# FIXME(harrison): I've disabled Emscripten builds until I get native builds working.
elseif(EMSCRIPTEN)
    # Runs if building for Emscripten (web)
    # Modified excerpt from https://discourse.libsdl.org/t/sdl2-desktop-bridge-emscripten-questions/22632/2
    # Make emscripten output a html page instead of just the javascript (for easier testing).
    set(CMAKE_EXECUTABLE_SUFFIX ".html")

    # Using SDL2 and SDL2_image
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2 --std=c++11")

    # Get SDL2_image to use PNGs
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s SDL2_IMAGE_FORMATS='[\"png\"]'")

    # Use O2 optimisation level (makes a .mem file)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O2")

    # Use assets folder
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file assets")

    # Another option you can try if having trouble with loading images:
    # https://groups.google.com/forum/#!topic/emscripten-discuss/0qxtF7zyG0s
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s STB_IMAGE=1")

    # Might need to play around with this if emscripten gives errors saying your program is too big.
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s TOTAL_MEMORY=67108864")
]]
else()
    message(FATAL_ERROR "Platform not supported")
endif()
