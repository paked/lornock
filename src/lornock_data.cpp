#define WORLD_WIDTH 3
#define WORLD_HEIGHT 3
#define WORLD_DEPTH 3

#include <state.h>

struct LornockData {
  Assets assets;

  State state;

  MemoryArena tempArena;
  MemoryArena actionsArena;
}* lornockData = 0;

#define assetsRequestShader(i) lornockData->assets.shaderRequests[i]++
#define assetsReleaseShader(i) lornockData->assets.shaderRequests[i]--
#define assetsRequestTexture(i) lornockData->assets.textureRequests[i]++
#define assetsReleaseTexture(i) lornockData->assets.textureRequests[i]--
#define assetsRequestModel(i) lornockData->assets.modelRequests[i]++
#define assetsReleaseModel(i) lornockData->assets.modelRequests[i]--

#define shader(i) lornockData->assets.shaders[i]
#define texture(i) lornockData->assets.textures[i]
#define model(i) lornockData->assets.models[i]

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

  for (uint32 i = 0; i < MAX_MODEL; i++) {
    if (lornockData->assets.modelRequests[i] > 0) {
      if (model(i).vao == 0) {
        logln("Created model");

        model(i) = model_init(modelFilename(i));
      }
    } else {
      if (model(i).vao != 0)  {
        logln("Destroyed model");

        model_clean(&model(i));
      }
    }
  }
}
