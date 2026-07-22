#ifndef BUTTON_GUARD
#define BUTTON_GUARD

#include "raylib.h"

#include "types.c"

Button initButton(Texture2D texture, Vector2 position) {
  Button b;

  b.isClicked = false;
  b.isDisabled = false;
  b.isHovered = false;
  b.texture = texture;
  b.position = position;

  return b;
}

void updateButton(Button *button) {
  Vector2 cursor = GetMousePosition();
  button->isHovered = false;
  button->isClicked = false;
  Rectangle buttonRec = {button->position.x, button->position.y,
                         button->texture.width / 2.0f, button->texture.height};

  if (CheckCollisionPointRec(cursor, buttonRec)) {
    button->isHovered = true;

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
      button->isClicked = true;
    }
  }
}

void drawButton(Button *button) {
  int buttonWidth = button->texture.width / 2;
  Rectangle buttonRec = {0, 0, buttonWidth, button->texture.height};

  if (button->isHovered && !button->isDisabled) {
    SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    buttonRec.x = buttonWidth;
  }

  Color color = WHITE;

  if (button->isDisabled) {
    color = (Color){.r = GRAY.r, .g = GRAY.g, .b = GRAY.b, .a = 150};
  }
  DrawTextureRec(button->texture, buttonRec, button->position, color);
}
#endif
