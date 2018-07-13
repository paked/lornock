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
- [ ] Windows support
  - [x] Split linux platform code into separate file
  - [x] Create "interface" which needs to be implemented by new platforms
  - [ ] Write win32 platform code
- [ ] Fix the whole billboarding/camera rotation shenangian
  - _HOLY FUCK I'VE SPENT LIKE A WEEK ON THIS SHIT._
  - [x] Refactor code into self contained chunks
  - [x] Create player as a "point" which can traverse the chunk
  - [ ] Display the player quad the correct orientation (perpendicular to the face it's on, face towards the camera)
  - [ ] Refactor code to use Quaternions 100% of the time

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

## Example code:

```cpp
// On every rotation
if (g->rotState == ROT_FORWARD) {
  g->playerPitch = g->playerPitch * quatFromPitchYawRoll(-90.0f, 0, 0);
  g->playerPitch = quatNormalize(g->playerPitch);
} else if (g->rotState == ROT_BACKWARD) {
  g->playerPitch = g->playerPitch * quatFromPitchYawRoll(90.0f, 0, 0);
  g->playerPitch = quatNormalize(g->playerPitch);
} else if (g->rotState == ROT_LEFT) {
  g->playerRoll = g->playerRoll * quatFromPitchYawRoll(0, 0, 90.0f);
  g->playerRoll = quatNormalize(g->playerRoll);
} else if (g->rotState == ROT_RIGHT) {
  g->playerRoll = g->playerRoll * quatFromPitchYawRoll(0, 0, -90.0f);
  g->playerRoll = quatNormalize(g->playerRoll);
}

...

// On update
quat playerOffset = quatFromPitchYawRoll(90, 0, 0); // We start on the front face, so player needs to be at 90 degrees
playerOffset = playerOffset * (g->playerYaw * g->playerPitch * g->playerRoll);

vec3 pivot = vec3(0, -0.5f, 0.0);

mat4 model = mat4d(1.0f);
model = mat4Translate(model, pivot);
model = mat4Translate(model, g->playerPosition + vec3(0.0f, 0.5f, 0.0));
model = model * quatToMat4(playerOffset);
model = mat4Scale(model, vec3(g->playerSize.x, g->playerSize.y, 1));
model = mat4Translate(model, pivot * -1);

// Draw w/ shader, etc.
```
