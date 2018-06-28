cmake_minimum_required(VERSION 3.5.1)
project(lornock)

# Glob vendored files
file(GLOB VENDOR_INCLUDE_DIRECTORIES
    vendor/glad/include)

file(GLOB VENDOR_SOURCES
    vendor/glad/src/glad.c)

# Glob project files
include_directories(
    src

    ${VENDOR_INCLUDE_DIRECTORIES})

# Put source files in this glob
file(GLOB SOURCES
    src/main.cpp

    ${VENDOR_SOURCES})

# Create executable
add_executable(lornock ${SOURCES})

# Copy over assets
add_custom_command(TARGET lornock PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
    "${CMAKE_SOURCE_DIR}/assets" "${CMAKE_BINARY_DIR}/assets"
    COMMENT "copying over assets"
	VERBATIM)

# We use a bit of a basterdised version of C++11, but it's C++11 no less
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")

# Runs if building for a native platform (currently only Linux)
if ((WIN32 OR (UNIX AND NOT APPLE)) AND NOT EMSCRIPTEN)
    # TODO(harrison): figure out exactly what this is for...
    if(NOT WIN32)
        set(GLAD_LIBRARIES dl)
    endif()

	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

	find_package(SDL2 REQUIRED)

	include_directories(${SDL2_INCLUDE_DIR})

	target_link_libraries(lornock ${SDL2_LIBRARY}
						 ${SDL2_IMAGE_LIBRARIES}
						 ${SDL2_TTF_LIBRARIES}
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
