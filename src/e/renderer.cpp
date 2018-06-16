#include <e/renderer.hpp>

#include <stdio.h>

#include <e/core.hpp>

void Renderer::render(SDL_Renderer* r) {
  while(!queue.empty()) {
    RenderJob next = queue.top();

    unsigned char oldAlpha;
    SDL_GetTextureAlphaMod(next.tex, &oldAlpha);

    if (next.alpha != oldAlpha) {
      SDL_SetTextureAlphaMod(next.tex, next.alpha);
    }

    SDL_RenderCopyEx(r, next.tex, &next.src, &next.dst, next.angle, NULL, next.flip);

    if (next.alpha != oldAlpha) {
      SDL_SetTextureAlphaMod(next.tex, oldAlpha);
    }

    queue.pop();
  }
}
