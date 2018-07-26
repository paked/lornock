# Lornock

Todo:

- Make time jumps not have to access the disk.
  - Write ActionChunks into a temporary arena
  - Then copy back into ActionArena once complete
  - THEN when the user ACTUALLY wants to access the disk, serialize and dump everything
- Create assert alternative `ensure`, which will make a SDL pop up window occur.
- Make billboarding work on a per face basis
- Save rotation and current face in camera state (maybe write a proper meta file)
- Refactor camera into camera struct
- Have things on the world
  - Have things on the world which change over time
- Fix number keys not triggering input events
- Hot reloading for assets
- Create "assets" memory pool which can dynamically grow, and will release files when they're not needed
  - Change loadFromFile interface to load file into provided data
- Remove all references to transient arena, instead access through TempArena
- Serialize camera position in save-file
- FIX THE FUCKING QUAT FROM PITCH YAW ROLL FUNCTION!

In progress:
- [ ] Add items to the world which can be picked up and placed in inventory
  - [ ] Find some way to save these in the world file?
- [ ] Create basic UI library
  - [x] Render rectangle
  - [x] Render icons
  - [x] Create toolbar
  - [ ] Render text
  - [ ] Create UI for time jump
- [ ] Windows support
  - [x] Split linux platform code into separate file
  - [x] Create "interface" which needs to be implemented by new platforms
  - [x] Get windows compiling
  - [ ] Write win32 platform code
- [ ] Fix the whole billboarding/camera rotation shenangian
  - _HOLY FUCK I'VE SPENT LIKE A WEEK ON THIS SHIT._
  - [x] Refactor code into self contained chunks
  - [x] Create player as a "point" which can traverse the chunk
  - [ ] Display the player quad the correct orientation (perpendicular to the face it's on, face towards the camera)
  - [ ] Refactor code to use Quaternions 100% of the time

Done:
- [x] Lighting
  - Add super basic lighting
- [x] Add OBJ file loader
- [x] Create a temporary memory pool / arena which can be used to allocate memory which will be cleared after a frame.
- [x] Fix the weird wrong face bug
- [x] Refactor TimeBox back into GameState
  - [x] Create a MemoryArena for storing this memory, and an ActionPool struct to put in MemoryBlocks
  - [x] Refactor TimeBox to use MemoryArena for it's actions instead of re-parsing at runtime
  - [x] Ensure the TimeBox is GOOD CODE.
    - Make TimeBox stateless, parse in `TimeBoxIndex` to the functions to control where it searches from
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
