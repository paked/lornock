#define WORLD_WIDTH 3
#define WORLD_HEIGHT 3
#define WORLD_DEPTH 3

struct LornockData {
  Assets assets;

  real32 progress;

  uint8 world[WORLD_HEIGHT][WORLD_DEPTH][WORLD_WIDTH];
  uint64 vertCount;

  GLuint VAO, VBO, EBO;
}* lornockData = 0;

#define assetsRequestShader(i) lornockData->assets.shaderRequests[i]++
#define assetsReleaseShader(i) lornockData->assets.shaderRequests[i]--
#define assetsRequestTexture(i) lornockData->assets.textureRequests[i]++
#define assetsReleaseTexture(i) lornockData->assets.textureRequests[i]--

#define shader(i) lornockData->assets.shaders[i]
#define texture(i) lornockData->assets.textures[i]

void updateAssets() {
  for (uint32 i = 0; i < MAX_SHADER; i++) {
    if (lornockData->assets.shaderRequests[i] > 0) {
      if (shader(i).id == 0) {
        logln("Created shader");

        shader(i) = shaderLoad(shaderFilename(i));
      }
    } else {
      if (shader(i).id != 0)  {
        logln("Destroyed shader");

        shaderClean(&shader(i));
      }
    }
  }

  for (uint32 i = 0; i < MAX_TEXTURE; i++) {
    if (lornockData->assets.textureRequests[i] > 0) {
      if (texture(i).id == 0) {
        logln("Created texture");

        texture(i) = textureLoad(textureFilename(i));
      }
    } else {
      if (texture(i).id != 0)  {
        logln("Destroyed texture");

        textureClean(&texture(i));
      }
    }
  }
}
