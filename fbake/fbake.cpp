/*
 * fbake is a simple tool which rasterizes TTF fonts into a bitmap texture
 * atlas and '.fnt' file containing information to lookup printable characters.
 *
 * Build with: `g++ -std=c++11 -I${STB_PATH} fbake.cpp -lm`
 *
 * Use like: `fbake 64 "Quicksand-Regular.ttf" ../data/font/quicksand_regular`
 */

#include <cstdint>
#include <cstdio>

#include <math.h>
#include <malloc.h>
#include <assert.h>

#define logln(fmt) fprintf(stderr, fmt "\n")
#define logfln(fmt, ...) fprintf(stderr, fmt "\n", __VA_ARGS__)

typedef int8_t    int8;
typedef int16_t   int16;
typedef int32_t   int32;
typedef int64_t   int64;
typedef uint8_t   uint8;
typedef uint16_t  uint16;
typedef uint32_t  uint32;
typedef uint64_t  uint64;
typedef float     real32;
typedef double    real64;
typedef int32     bool32;

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

int main(int argc, char* argv[]) {
  if (argc < 3) {
    logln("Invalid parameters. Usage: `fbake ${FONT_SIZE} ${TTF_PATH} ${OUTPUT_BASE_NAME}`");

    return 0;
  }

  size_t ttfFileSize = 0;
  unsigned char* ttfFileBuffer = 0;
  unsigned char* bitmapBuffer = 0;

  char* fname = argv[2];

  int pixelHeight = 0;
  assert(sscanf(argv[1], "%d", &pixelHeight) == 1);

  char* outputBaseName = argv[3];

  const char* outputPNGExt = ".png";
  const char* outputFNTExt = ".fnt";

  char* outputPNG = (char*) malloc(strlen(outputBaseName) + strlen(outputPNGExt));
  strcat(outputPNG, outputBaseName);
  strcat(outputPNG, outputPNGExt);

  char* outputFNT = (char*) malloc(strlen(outputBaseName) + strlen(outputFNTExt));
  strcat(outputFNT, outputBaseName);
  strcat(outputFNT, outputFNTExt);

  logfln("Writing to %s and %s", outputFNT, outputPNG);

  // 1. Read TTF file into buffer
  {
    FILE* ttfFile = fopen(fname, "rb");

    fseek(ttfFile, 0, SEEK_END);

    ttfFileSize = ftell(ttfFile);

    fseek(ttfFile, 0, SEEK_SET);

    ttfFileBuffer = (unsigned char*) malloc(ttfFileSize);

    size_t read = fread(ttfFileBuffer, ttfFileSize, 1, ttfFile);
    assert(read == 1);
    
    fclose(ttfFile);
  }

  // 2. Create bitmap
  int64 bitmapWidth = 512;
  int64 bitmapHeight = 512;
  int64 lineHeight = (int64) (pixelHeight * 1.4);

  int firstChar = 0;
  int charCount = 0;

  {
    char begin = ' ';
    char end = '~';

    firstChar = (int) begin;
    charCount = (int) end - begin;
  }

  stbtt_bakedchar* chars = (stbtt_bakedchar*) malloc(sizeof(stbtt_bakedchar) * charCount);

  bitmapBuffer = (unsigned char*) malloc(bitmapWidth * bitmapHeight);

  int res = stbtt_BakeFontBitmap(
      ttfFileBuffer, 0,
      pixelHeight,
      bitmapBuffer, bitmapWidth, bitmapHeight,
      firstChar, charCount,
      chars);

  if (res > 0) {
    logfln("Success! Generated bitmap. Bitmap fits %d characters", res);
  } else {
    logfln("Failure! Was only able to fit %d characters, wanted %d.", res * -1, charCount);

    return -1;
  }

  FILE* out = fopen(outputFNT, "wb");
  if (out == 0) {
    logln("ERROR: could not open output.fnt file");

    return -1;
  }

  // 3. Output char information
  fprintf(out, "lh %ld\n", lineHeight);

  for (int i = 0; i < charCount; i++) {
    stbtt_bakedchar info = *(chars + i);

    fprintf(out, "id %d x0 %d y0 %d x1 %d y1 %d xoff %f yoff %f xadv %f\n",
        firstChar + i,
        info.x0, info.y0, info.x1, info.y1,
        info.xoff, info.yoff, info.xadvance);
  }

  fclose(out);

  stbi_write_png(outputPNG, bitmapWidth, bitmapHeight, 1, bitmapBuffer, bitmapWidth);

  free(ttfFileBuffer);
  free(bitmapBuffer);
  free(chars);

  return 0;
}
