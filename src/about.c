#ifndef ABOUT_GUARD
#define ABOUT_GUARD

#include "raylib.h"
#include "globals.c"
#include "button.c"

void drawAbout(void) {
  // Draw background
  DrawTexture(aboutBg, 0, 0, WHITE);

  DrawTextEx(
      font20,
      "Best Bee-uddies is a small game about producing and harvesting honey.",
      (Vector2){50, 104}, 20, 0, DARKBROWN);
  DrawTextEx(
      font20,
      "The game was made in one week in July 2026 for the Raylib GameJam 6.x.",
      (Vector2){50, 134}, 20, 0, DARKBROWN);
  DrawTextEx(font20, "The theme was HEX + MERGE.", (Vector2){50, 164}, 20, 0,
             DARKBROWN);
  DrawTextEx(font20, "Brian Ravn (xiroV): Design, Programming, Artwork",
             (Vector2){50, 214}, 20, 0, DARKBROWN);
  DrawTextEx(font20, "Jonas Hinchely (Tiggles): Programming",
             (Vector2){50, 244}, 20, 0, DARKBROWN);
  DrawTextEx(
      font20,
      "Made 100% without the use of generative AI/Large Language Models.",
      (Vector2){50, 274}, 20, 0, DARKBROWN);

  DrawTextEx(font20, "Music:", (Vector2){50, 324}, 20, 0, DARKBROWN);
  DrawTextEx(font20,
             "Calm Acoustic Guitar for Serene Moments by Gustavo_Alivera",
             (Vector2){50, 354}, 20, 0, DARKBROWN);
  DrawTextEx(font20,
             "-- https://freesound.org/s/761373/ -- License: Attribution 4.0",
             (Vector2){50, 386}, 20, 0, DARKBROWN);
  DrawTextEx(font20, "Sounds:", (Vector2){50, 436}, 20, 0, DARKBROWN);
  DrawTextEx(font20,
             "Cha_Ching.mp3 by Lucish_ -- https://freesound.org/s/554841/ -- "
             "License: Attribution 3.0",
             (Vector2){50, 466}, 20, 0, DARKBROWN);
  DrawTextEx(font20, "rabbit - thump - on - soil - edited.wav by bunnyluvvid",
             (Vector2){50, 496}, 20, 0, DARKBROWN);
  DrawTextEx(
      font20,
      "-- https://freesound.org/s/431204/ -- License: Creative Commons 0",
      (Vector2){50, 526}, 20, 0, DARKBROWN);

  // Draw buttons
  drawButton(&backToMenuButton);
}

void updateAbout(void) {
  updateButton(&backToMenuButton);

  if (backToMenuButton.isClicked) {
    gs->currentScene = MENU;
  }
}

#endif
