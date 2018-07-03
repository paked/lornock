# Lornock

Tasks:

- Refactor camera into camera struct
- Generalise billboard renderer
- Begin work porting time travel code over
- Write win32 platform code

Dev tooling:
- Create `dev.sh` (and `dev.bat`) environment with `edit`, `build`, and `run` macros
- Create script which will list all TODOs and FIXMEs

Eventually:

- Create a platform side "renderer", and have the game just send commands to it
  - I need to experiment with what lornocks renderer "is" until I can actually do this though. No point making some obtusely generalised thing which I don't reaaaallly need

## Manifesto

- Game is going to be voxel-driven 3D with 2D billboarded characters
  - Environment is easy to pixel-voxel
  - Characters are easy to animate
- Will be made in C style C++
  - avoid STL
  - Maybe use MemoryArenas
  - `uintX` typedefs
  - dynamic arrays (maybe use delix's?)
- Will create an anonymous union'd command buffer renderer
- Focus on splitting "platform layer" from "game" in order to hopefully promote clean code
- Use HandmadeMath and GLAD
- Fast iteration with low compile time
