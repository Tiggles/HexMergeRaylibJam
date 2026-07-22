#ifndef ANIMATION_GUARD
#define ANIMATION_GUARD

#include "raylib.h"
#include "types.c"

// Load an animation (spritesheet) into memory with a set of animation
// parameters.
Animation initAnimation(char *fileName, int numFrames, int intervalMs) {
  return (Animation){
      .texture = LoadTexture(fileName),
      .frame = 0,
      .intervalMs = intervalMs,
      .numFrames = numFrames,
      .lastDraw = 0,
  };
}

Animation initAnimationFromTexture(Texture2D texture, int numFrames, int intervalMs) {
  return (Animation){
      .texture = texture,
      .frame = 0,
      .intervalMs = intervalMs,
      .numFrames = numFrames,
      .lastDraw = 0,
  };
}

// Draw animation frame. Frames should step automatically.
void drawAnimationFrame(Animation *animation, Vector2 position) {
  animation->lastDraw += GetFrameTime();

  float textureWidth = animation->texture.width;
  float spriteWidth = textureWidth / animation->numFrames;
  float spriteHeight = animation->texture.height;
  Rectangle frameRec = {
      spriteWidth * animation->frame,
      0,
      spriteWidth,
      spriteHeight,
  };

  Rectangle destRec = {position.x, position.y, spriteWidth, spriteHeight};

  Vector2 origin = {0, 0};
  // DrawTextureRec(animation->texture, frameRec, position, WHITE);
  DrawTexturePro(animation->texture, frameRec, destRec, origin, 0, WHITE);

  if (animation->lastDraw * 1000 > animation->intervalMs) {
    animation->lastDraw = 0;
    animation->frame += 1;

    if (animation->frame >= animation->numFrames) {
      animation->frame = 0;
    }
  }
}

void unloadAnimation(Animation *animation) {
  UnloadTexture(animation->texture);
}

#endif
