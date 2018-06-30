struct LornockData {
  Assets assets;

  GLuint VAO, VBO, EBO;
}* lornockData = 0;

#define assetsRequestShader(i) lornockData->assets.shaderRequests[i]++
#define assetsReleaseShader(i) lornockData->assets.shaderRequests[i]--

#define shader(i) lornockData->assets.shaders[i]

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
}
