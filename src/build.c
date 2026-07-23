#ifndef BUILD_GUARD
#define BUILD_GUARD

#include "raylib.h"

#include "button.c"
#include "garden.c"
#include "globals.c"

void drawBuildScene(void) {
  SetMouseCursor(MOUSE_CURSOR_CROSSHAIR);
  Vector2 cursor = GetMousePosition();

  // Draw background
  DrawTexture(gardenBg, 0, -110, WHITE);

  // Draw button
  drawButton(&cancelPurchaseButton);

  // Draw hexgrid
  for (unsigned int row = 0; row < 13; row++) {
    int numColumns = 13;
    if (row % 2 == 1)
      numColumns = 12;

    for (unsigned int column = 0; column < numColumns; column++) {
      Vector2 hexPos = gardenHexPositionToPixelPosition((Vector2){column, row});

      drawGardenHex(hexPos);
    }
  }

  // Draw objects
  drawHives();
  drawFlowers();

  Vector2 chosenHexCoord = gardenHexFromPoint(cursor);

  // Draw build
  if (cancelPurchaseButton.isHovered) {
    return;
  }
  bool isHexOccupied = isGardenHexOccupied(chosenHexCoord);

  if (isHexOccupied) {
    Vector2 chosenHexPixelCoord =
        gardenHexPositionToPixelPosition(chosenHexCoord);
    drawGardenHexFilled(chosenHexPixelCoord, RED);
    return;
  }

  Vector2 chosenHexPixelCoord =
      gardenHexPositionToPixelPosition(chosenHexCoord);
  drawGardenHexFilled(chosenHexPixelCoord, SKYBLUE);

  switch (gs->currentlyBuilding) {
  case BUILD_HIVE:
    DrawTextureRec(
        hiveSprite,
        (Rectangle){0, 0, hiveSprite.width / 3.0f, hiveSprite.height},
        (Vector2){chosenHexPixelCoord.x - 32, chosenHexPixelCoord.y - 55},
        WHITE);
    break;
  case BUILD_ZINNIAS:
    DrawTextureV(
        zinniasSprite,
        (Vector2){chosenHexPixelCoord.x - 20, chosenHexPixelCoord.y - 30},
        WHITE);
    break;
  case BUILD_DAHLIAS:
    DrawTextureV(
        dahliasSprite,
        (Vector2){chosenHexPixelCoord.x - 20, chosenHexPixelCoord.y - 30},
        WHITE);
    break;
  case BUILD_LAVENDERS:
    DrawTextureV(
        lavenderSprite,
        (Vector2){chosenHexPixelCoord.x - 20, chosenHexPixelCoord.y - 30},
        WHITE);
    break;
  case BUILD_SUNFLOWERS:
    DrawTextureV(
        sunflowerSprite,
        (Vector2){chosenHexPixelCoord.x - 20, chosenHexPixelCoord.y - 30},
        WHITE);
    break;
  case BUILD_NULL:
    // Ignore
    break;
  }
}

void updateBuildScene(void) {
  Vector2 cursor = GetMousePosition();
  updateButton(&cancelPurchaseButton);

  if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
    if (cancelPurchaseButton.isClicked) {
      gs->currentScene = SHOP;
      return;
    }

    Vector2 chosenHexCoord = gardenHexFromPoint(cursor);
    bool isHexOccupied = isGardenHexOccupied(chosenHexCoord);

    if (isHexOccupied) {
      return;
    }

    int didBuild = false;
    switch (gs->currentlyBuilding) {
    case BUILD_HIVE: {
      gs->hives[gs->numHives++] = initHive(chosenHexCoord.x, chosenHexCoord.y);
      gs->money -= HIVE_PRICE;
      gs->currentScene = GARDEN;
      didBuild = true;

      break;
    }
    case BUILD_ZINNIAS: {
      gs->flowers[gs->numFlowers++] =
          initFlower(FLOWER_ZINNIAS, chosenHexCoord.x, chosenHexCoord.y);
      gs->money -= ZINNIAS_PRICE;
      gs->currentScene = GARDEN;
      didBuild = true;

      break;
    }
    case BUILD_DAHLIAS: {
      gs->flowers[gs->numFlowers++] =
          initFlower(FLOWER_DAHLIAS, chosenHexCoord.x, chosenHexCoord.y);
      gs->money -= DAHLIAS_PRICE;
      gs->currentScene = GARDEN;
      didBuild = true;

      break;
    }
    case BUILD_LAVENDERS: {
      gs->flowers[gs->numFlowers++] =
          initFlower(FLOWER_LAVENDERS, chosenHexCoord.x, chosenHexCoord.y);
      gs->money -= LAVENDERS_PRICE;
      gs->currentScene = GARDEN;
      didBuild = true;

      break;
    }
    case BUILD_SUNFLOWERS: {
      gs->flowers[gs->numFlowers++] =
          initFlower(FLOWER_SUNFLOWERS, chosenHexCoord.x, chosenHexCoord.y);
      gs->money -= SUNFLOWERS_PRICE;
      gs->currentScene = GARDEN;
      didBuild = true;

      break;
    }
    default: {
      // If item is not covered, do nothing
    }
    }

    if (didBuild) {
      PlaySound(thump);
    }
  }
}
#endif
