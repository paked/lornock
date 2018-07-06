# Lornock

Tasks:

- Fix number keys not triggering input events
- [IN PROGRESS] Begin work porting time travel code over
  - [x] Create Action datastructures
  - [x] Parse timeline file
  - [x] Plan how LARGE timelines will be handled (at least initially)
  - [x] Play timeline file
  - [ ] Write timeline file
- [IN PROGRESS] Write win32 platform code
  - [x] Split linux platform code into separate file
  - [x] Create "interface" which needs to be implemented by new platforms
  - [ ] Write win32 platform code
- Refactor camera into camera struct
- Generalise billboard renderer
- Lighting
- Have things on the world
  - Have things on the world which change over time


Dev tooling:
- SWITCH TO `-Wall -Werror`
- Create `dev.sh` (and `dev.bat`) environment with `edit`, `build`, and `run` macros
- Create script which will list all TODOs and FIXMEs
Eventually:

- Pretty pixel art effect skybox generation
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

## For Shits And Gigs

Without stdlib: 1.3s, 0.471mb
Including vector: 1.5s, 0.479mb
