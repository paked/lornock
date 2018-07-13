#define getTime() (platform->time)
#define getDt() (platform->deltaTime)
#define getWindowWidth() (platform->windowWidth)
#define getWindowHeight() (platform->windowHeight)

#define loadFromFile(p, d, l) platform->loadFromFile(p, d, l)
#define writeToFile(p, d, l) platform->writeToFile(p, d, l)

#define keyJustDown(k) (platform->keyStateNow[k] && !platform->keyStateLast[k])
#define keyJustUp(k) (!platform->keyStateNow[k] && platform->keyStateLast[k])
#define keyDown(k) (platform->keyStateNow[k])
#define keyUp(k) (!platform->keyStateNow[k])

#define CUBE_MESH_DATA_FACE_LENGTH 30

#define CUBE_MESH_DATA { \
     0.0f,  0.0f,  0.0f,  0.0f, 0.0f, /* back */ \
     1.0f,  0.0f,  0.0f,  1.0f, 0.0f, \
     1.0f,  1.0f,  0.0f,  1.0f, 1.0f, \
     1.0f,  1.0f,  0.0f,  1.0f, 1.0f, \
     0.0f,  1.0f,  0.0f,  0.0f, 1.0f, \
     0.0f,  0.0f,  0.0f,  0.0f, 0.0f, \
\
     0.0f,  0.0f,  1.0f,  0.0f, 0.0f, /* front */ \
     1.0f,  0.0f,  1.0f,  1.0f, 0.0f, \
     1.0f,  1.0f,  1.0f,  1.0f, 1.0f, \
     1.0f,  1.0f,  1.0f,  1.0f, 1.0f, \
     0.0f,  1.0f,  1.0f,  0.0f, 1.0f, \
     0.0f,  0.0f,  1.0f,  0.0f, 0.0f, \
\
     0.0f,  1.0f,  1.0f,  1.0f, 0.0f, /* left */ \
     0.0f,  1.0f,  0.0f,  1.0f, 1.0f, \
     0.0f,  0.0f,  0.0f,  0.0f, 1.0f, \
     0.0f,  0.0f,  0.0f,  0.0f, 1.0f, \
     0.0f,  0.0f,  1.0f,  0.0f, 0.0f, \
     0.0f,  1.0f,  1.0f,  1.0f, 0.0f, \
\
     1.0f,  1.0f,  1.0f,  1.0f, 0.0f, /* right */ \
     1.0f,  1.0f,  0.0f,  1.0f, 1.0f, \
     1.0f,  0.0f,  0.0f,  0.0f, 1.0f, \
     1.0f,  0.0f,  0.0f,  0.0f, 1.0f, \
     1.0f,  0.0f,  1.0f,  0.0f, 0.0f, \
     1.0f,  1.0f,  1.0f,  1.0f, 0.0f, \
\
     0.0f,  0.0f,  0.0f,  0.0f, 1.0f, /* bottom */ \
     1.0f,  0.0f,  0.0f,  1.0f, 1.0f, \
     1.0f,  0.0f,  1.0f,  1.0f, 0.0f, \
     1.0f,  0.0f,  1.0f,  1.0f, 0.0f, \
     0.0f,  0.0f,  1.0f,  0.0f, 0.0f, \
     0.0f,  0.0f,  0.0f,  0.0f, 1.0f, \
\
     0.0f,  1.0f,  0.0f,  0.0f, 1.0f, /* top */ \
     1.0f,  1.0f,  0.0f,  1.0f, 1.0f, \
     1.0f,  1.0f,  1.0f,  1.0f, 0.0f, \
     1.0f,  1.0f,  1.0f,  1.0f, 0.0f, \
     0.0f,  1.0f,  1.0f,  0.0f, 0.0f, \
     0.0f,  1.0f,  0.0f,  0.0f, 1.0f \
}
