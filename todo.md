# Lornock

Todo:

- Make billboarding work on a per face basis
- Save rotation and current face in camera state (maybe write a proper meta file)
- Refactor camera into camera struct
- Generalise billboard renderer
- Lighting
- Have things on the world
  - Have things on the world which change over time
- Fix number keys not triggering input events
- Refactor TimeBox into GOOD CODE.
  - Come up with a consistent API for accessing the timebox
    - Remove the concept of "upto" state from the TimeBox, create a separate `TimeBoxIndex` struct which contains that info.
    - Functions like `timeBox_findNextAction(TimeBox* tb, TimeBoxIndex* tbi, uint64 sequence)`
    - Create versatile search functions
  - Parse each line into an action when it is first read, keep in chunks of certain sizes and kept in some storage
    - Create a divide between `FrameStorage` (memory which gets killed after the end of a frame), `TransientStorage` (memory which can be killed at any point and regenerated if need be, usually lasts many frames though)

In progress:
- [IN PROGRESS] Windows support
  - [x] Split linux platform code into separate file
  - [x] Create "interface" which needs to be implemented by new platforms
  - [ ] Write win32 platform code

Done:
- [x] Begin work porting time travel code over
  - [x] Create Action datastructures
  - [x] Parse timeline file
  - [x] Plan how LARGE timelines will be handled (at least initially)
  - [x] Play basic timeline file
  - [x] Write basic timeline file
  - [x] Have timelines be both playing back and written (ie. bring back to feature parity with original codebase)

Dev tooling:
- SWITCH TO `-Wall -Werror`
- Create `dev.sh` (and `dev.bat`) environment with `edit`, `build`, and `run` macros
- Create script which will list all TODOs and FIXMEs

Eventually:

- Pretty pixel art effect skybox generation
- Create a platform side "renderer", and have the game just send commands to it
  - I need to experiment with what lornocks renderer "is" until I can actually do this though. No point making some obtusely generalised thing which I don't reaaaallly need
- Run on OSX

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

## Brain Dump For Tomorrow

1. Probably should look into writing some sort of allocator so it is easier to access the TransientStorage
2. I AM GOING TO REWRITE THE READING OF ACTIONS SO THAT IT READS STRAIGHT FROM THE FILE MEMORY BUFFER AND PARSES EACH ACTION AT RUNTIME. IT IS NOT SUPER EFFICIENT BUT IT IS THE EASIEST WAY TO START.
3. Each time the player jumps backwards or forwards in time THEY SHOULD WRITE THEIR PENDING ACTIONS.
