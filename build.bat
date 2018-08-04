@echo off

set SDL2PATH=C:\dev\SDL2-2.0.8
set SDL2LIBS=SDL2.lib SDL2main.lib

set common_compiler_flags=-nologo -Gm- -GR- -EHa- -Od /Zi /I ..\src
set common_linker_flags=/OPT:REF /INCREMENTAL:NO

IF NOT EXIST .\build mkdir .\build

del *.pdb > NUL 2> NUL

pushd .\build

cl -MT %common_compiler_flags% ..\src\sdl_main.cpp /I %SDL2PATH%\include /link %common_linker_flags% /LIBPATH:%SDL2PATH%\lib\x64 SDL2.lib SDL2main.lib /OUT:sdl_lornock.exe /SUBSYSTEM:CONSOLE

cl -LD %common_compiler_flags% ..\src\lornock.cpp ..\vendor\glad\src\glad.c /I ..\vendor\glad\include /I ..\vendor\HandmadeMath /I ..\vendor\stb /link %common_linker_flags%

popd
