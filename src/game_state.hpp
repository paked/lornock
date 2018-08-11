#define PLAYER_DEFAULT_SPAWN vec3(0.0f, 1.5f, 0.0f)

#define WORLD_CELL_SIZE (0.5f)
#define WORLD_CELL_COUNT (6)
#define WORLD_SIZE ((WORLD_CELL_SIZE*WORLD_CELL_COUNT))

#define CAMERA_ROTATION_OFFSET (-30)
#define CAMERA_POSITION (vec3(0, 0.0f, -7.25f))
#define MAX_PAST_PLAYERS 10
#define ROTATION_DURATION 400
#define PLAYER_INVENTORY_SIZE 9


typedef uint32 World[WORLD_CELL_COUNT][WORLD_CELL_COUNT][WORLD_CELL_COUNT];

// Voxel direction/faces
enum {
  BACK,
  FRONT,
  LEFT,
  RIGHT,
  BOTTOM,
  TOP,
  MAX_FACE
};

void printFace(uint32 f) {
  switch(f) {
    case BACK:
      { logln("back face"); } break;
    case FRONT:
      { logln("front face"); } break;
    case LEFT:
      { logln("left face"); } break;
    case RIGHT:
      { logln("right face"); } break;
    case BOTTOM:
      { logln("bottom face"); } break;
    case TOP:
      { logln("top face"); } break;
    default:
      { logln("unknown face"); } break;
  }
}

enum {
  ROT_FORWARD,
  ROT_BACKWARD,
  ROT_LEFT,
  ROT_RIGHT,
  ROT_IDLE
};

void printRot(uint32 r) {
  switch (r) {
    case ROT_FORWARD:
      { logln("forward rotating"); } break;
    case ROT_BACKWARD:
      { logln("backward rotating"); } break;
    case ROT_LEFT:
      { logln("left rotating"); } break;
    case ROT_RIGHT:
      { logln("right rotating"); } break;
    case ROT_IDLE:
      { logln("idle rotating"); } break;
  }
}

enum {
  DIRECTION_UP,
  DIRECTION_RIGHT,
  DIRECTION_FORWARD,
  MAX_DIRECTION
};

vec3 faceCardinalDirections[MAX_FACE][MAX_DIRECTION] = {
  // UP                       Right                     Forward
  { vec3(0.0f, 0.0f, -1.0f),   vec3(1.0f, 0.0f, 0.0f),   vec3(0.0f, -1.0f, 0.0f) },   // Back face
  { vec3(0.0f, 0.0f, 1.0f),  vec3(1.0f, 0.0f, 0.0f),   vec3(0.0f, 1.0f, 0.0f) },  // Front face

  { vec3(-1.0f, 0.0f, 0.0f),  vec3(0.0f, 1.0f, 0.0f),  vec3(0.0f, 0.0f, -1.0f) },  // Left face
  { vec3(1.0f, 0.0f, 0.0f),   vec3(0.0f, -1.0f, 0.0f),  vec3(0.0f, 0.0f, -1.0f) },  // Right face

  { vec3(0.0f, -1.0f, 0.0f),  vec3(1.0f, 0.0f, 0.0f),  vec3(0.0f, 0.0f, 1.0f) },  // Bottom face
  { vec3(0.0f, 1.0f, 0.0f),   vec3(1.0f, 0.0f, 0.0f),   vec3(0.0f, 0.0f, -1.0f) }  // Top face
};

quat faceRotations[MAX_FACE] = {
  quatFromAngleAxis(270.0f, vec3_right), // Back
  quatFromAngleAxis(90.0f, vec3_right), // Front

  quatFromAngleAxis(270.0f, vec3_forward), // Left
  quatFromAngleAxis(90.0f, vec3_forward), // Right

  quatFromAngleAxis(180.0f, vec3_right), // Bottom
  quatFromAngleAxis(0.0f, vec3_right), // Top
};

bool voxelEmptyToThe(int d, World w, int x, int y, int z) {
  int offsetX = 0;
  int offsetY = 0;
  int offsetZ = 0;

  switch(d) {
    case BACK:
      {
        offsetZ = -1;
      } break;
    case FRONT:
      {
        offsetZ = 1;
      } break;
    case LEFT:
      {
        offsetX = -1;
      } break;
    case RIGHT:
      {
        offsetX = 1;
      } break;
    case BOTTOM:
      {
        offsetY = -1;
      } break;
    case TOP:
      {
        offsetY = 1;
      } break;
    default:
      {
        logln("WARNING: unknown voxel face type");

        return false;
      } break;
  }

  int posX = x + offsetX;
  int posY = y + offsetY;
  int posZ = z + offsetZ;

  if (posX < 0 || posX > WORLD_CELL_COUNT - 1 ||
      posY < 0 || posY > WORLD_CELL_COUNT - 1||
      posZ < 0 || posZ > WORLD_CELL_COUNT - 1) {
    return true;
  }

  return w[posY][posZ][posX] != 1;
}

void addFaceToMesh(uint32 d, real32* verts, uint64* len, vec3 offset) {
  real32 cube[] = CUBE_MESH_DATA;

  uint64 start = d * CUBE_MESH_DATA_FACE_LENGTH;
  uint64 end = start + CUBE_MESH_DATA_FACE_LENGTH;

  for (uint64 i = start; i < end; i++) {
    real32 v = cube[i];

    int index = i % CUBE_MESH_ELEMENT_LEN;
    if (index < 3) {
      v *= WORLD_CELL_SIZE;

      v += offset[index];
    }

    verts[*len] = v;

    *len += 1;
  }
}

enum BlockType : uint32 {
  BLOCK_NONE,
  BLOCK_ROCK,
  BLOCK_COAL
};

struct InventorySlot {
  uint32 type; // BlockType

  int count;
};

bool inventorySlot_empty(InventorySlot is) {
  return is.type == BLOCK_NONE;
}

#define INVENTORY_SIZE 9
#define INVENTORY_MAX_STACK_SIZE 64
struct Inventory {
  int currentSlot;

  InventorySlot hotbar[INVENTORY_SIZE];
};
