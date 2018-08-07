#include <state.h>

struct LornockData {
  Assets assets;

  State state;

  MemoryArena tempArena;
  MemoryArena actionsArena;
}* lornockData = 0;

#define assets_requestShader(i) lornockData->assets.shaderRequests[i]++
#define assets_releaseShader(i) lornockData->assets.shaderRequests[i]--
#define assets_requestTexture(i) lornockData->assets.textureRequests[i]++
#define assets_releaseTexture(i) lornockData->assets.textureRequests[i]--
#define assets_requestModel(i) lornockData->assets.modelRequests[i]++
#define assets_releaseModel(i) lornockData->assets.modelRequests[i]--
#define assets_requestFont(i) lornockData->assets.fontRequests[i]++
#define assets_releaseFont(i) lornockData->assets.fontRequests[i]--

#define shader(i) lornockData->assets.shaders[i]
#define texture(i) lornockData->assets.textures[i]
#define model(i) lornockData->assets.models[i]
#define font(i) lornockData->assets.fonts[i]

void lornockData_assetsUpdate() {
  for (uint32 i = 0; i < MAX_SHADER; i++) {
    if (lornockData->assets.shaderRequests[i] > 0) {
      if (shader(i).id == 0) {
        logln("Created shader");

        shader(i) = shader_load(shaderFilename(i));
      }
    } else {
      if (shader(i).id != 0)  {
        logln("Destroyed shader");

        shader_clean(&shader(i));
      }
    }
  }

  for (uint32 i = 0; i < MAX_TEXTURE; i++) {
    if (lornockData->assets.textureRequests[i] > 0) {
      if (texture(i).id == 0) {
        logln("Created texture");

        texture(i) = texture_load(textureFilename(i));
      }
    } else {
      if (texture(i).id != 0)  {
        logln("Destroyed texture");

        texture_clean(&texture(i));
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

  for (uint32 i = 0; i < MAX_FONT; i++) {
    if (lornockData->assets.fontRequests[i] > 0) {
      if (font(i).texture.id == 0) {
        logln("Created font");

        font(i) = font_load(fontFilename(i));
      }
    } else {
      if (font(i).texture.id != 0)  {
        logln("Destroyed font");

        font_clean(&font(i));
      }
    }
  }
}
