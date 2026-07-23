#ifndef MENU_GUARD
#define MENU_GUARD

#include "raylib.h"

#include "utils.c"
#include "button.c"
#include "consts.c"
#include "globals.c"

void drawMenu(void) {
  // Draw background
  DrawTexture(menuBg, 0, 0, WHITE);

  // Draw version
  DrawTextEx(font20, TextFormat("v%s", VERSION), (Vector2){680, 690}, 20, 0, WHITE);

  // Draw buttons
  drawButton(&startButton);
  drawButton(&aboutButton);
}

void updateMenu(void) {
  Vector2 cursor = GetMousePosition();
  startButton.isHovered = false;
  aboutButton.isHovered = false;

  if (vector2Distance(cursor, (Vector2){startButton.position.x + 50,
                                        startButton.position.y + 50}) < 50) {
    startButton.isHovered = true;

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
      gs->currentScene = GARDEN;
    }
  }

  if (vector2Distance(cursor, (Vector2){aboutButton.position.x + 50,
                                        aboutButton.position.y + 50}) < 50) {
    aboutButton.isHovered = true;

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
      gs->currentScene = ABOUT;
    }
  }
}
#endif
