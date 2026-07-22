#ifndef GARDEN_GUARD
#define GARDEN_GUARD

#include "raylib.h"
#include "globals.c"
#include "func_decls.c"

void updateGardenScene(void) {
  gs->playerNearShop = false;

  // Update camera
  static Vector2 bbox = {0.2f, 0.2f};

  int width = 720;
  int height = 720;

  Vector2 bboxWorldMin = GetScreenToWorld2D(
      (Vector2){(1 - bbox.x) * 0.5f * width, (1 - bbox.y) * 0.5f * height},
      gardenCamera);
  Vector2 bboxWorldMax = GetScreenToWorld2D(
      (Vector2){(1 + bbox.x) * 0.5f * width, (1 + bbox.y) * 0.5f * height},
      gardenCamera);
  gardenCamera.offset =
      (Vector2){(1 - bbox.x) * 0.5f * width, (1 - bbox.y) * 0.5f * height};

  if (gs->playerPosition.x < bboxWorldMin.x)
    gardenCamera.target.x = gs->playerPosition.x;
  if (gs->playerPosition.y < bboxWorldMin.y)
    gardenCamera.target.y = gs->playerPosition.y;
  if (gs->playerPosition.x > bboxWorldMax.x)
    gardenCamera.target.x =
        bboxWorldMin.x + (gs->playerPosition.x - bboxWorldMax.x);
  if (gs->playerPosition.y > bboxWorldMax.y)
    gardenCamera.target.y =
        bboxWorldMin.y + (gs->playerPosition.y - bboxWorldMax.y);

  float distance = vector2Distance(gs->playerPosition, SHOP_POSITION);
  if (distance < 100) {
    gs->playerNearShop = true;
  }

  Vector2 newPlayerPosition = gs->playerPosition;

  // Keybindings
  if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
    if (gs->playerPosition.y + MOVEMENT_SPEED * GetFrameTime() < 660) {
      newPlayerPosition.y += MOVEMENT_SPEED * GetFrameTime();
    }
    gs->playerDirection = DOWN;
    gs->playerMoving = true;
  }
  if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
    if (gs->playerPosition.y - MOVEMENT_SPEED * GetFrameTime() > 30) {
      newPlayerPosition.y -= MOVEMENT_SPEED * GetFrameTime();
    }
    gs->playerDirection = UP;
    gs->playerMoving = true;
  }
  if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
    if (gs->playerPosition.x - MOVEMENT_SPEED * GetFrameTime() > 15) {
      newPlayerPosition.x -= MOVEMENT_SPEED * GetFrameTime();
    }
    gs->playerDirection = LEFT;
    gs->playerMoving = true;
  }
  if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
    if (gs->playerPosition.x + MOVEMENT_SPEED * GetFrameTime() < 710) {
      newPlayerPosition.x += MOVEMENT_SPEED * GetFrameTime();
    }
    gs->playerDirection = RIGHT;
    gs->playerMoving = true;
  }

  // Check if new suggested position is colliding with hive
  if (!isHiveCollision(newPlayerPosition)) {
    gs->playerPosition = newPlayerPosition;
  }

  if (IsKeyPressed(KEY_Z)) {
    if (gs->playerNearShop) {
      gs->currentScene = SHOP;
      return;
    }

    // If close to hive, enter harvest mode
    Vector2 nearestHivePixelPos =
        gardenHexPositionToPixelPosition(gs->hives[gs->nearestHive]->position);
    float distanceToNearestHive =
        vector2Distance(gs->playerPosition, nearestHivePixelPos);
    if (distanceToNearestHive < 100) {
      gs->activeHiveIndex = gs->nearestHive;
      gs->currentScene = HARVEST;
    }
  }

  // Find nearest hive
  float distanceToNearest = 9999;
  for (unsigned int i = 0; i < gs->numHives; i++) {
    Vector2 hivePixelPos = gardenHexPositionToPixelPosition(gs->hives[i]->position);
    float distance = vector2Distance(gs->playerPosition, hivePixelPos);
    if (distance < distanceToNearest) {
      distanceToNearest = distance;
      gs->nearestHive = i;
    }
  }
}

void drawGardenHex(Vector2 center) {
  DrawPolyLines(center, 6, 30, 210, DARKGRAY);
}

void drawGardenHexFilled(Vector2 center, Color color) {
  DrawPoly(center, 6, 30, 210, color);
}

void drawGardenScene(void) {
  KeeperSprite keeperSprite = FRONT;

  BeginMode2D(gardenCamera);

  // Draw background
  DrawTexture(gardenBg, 0, -120, WHITE);

  // Draw objects
  drawHives();
  drawFlowers();

  // Draw player
  switch (gs->playerDirection) {
  case UP:
    if (gs->playerMoving) {
      keeperSprite = WALK_UP;
    } else {
      keeperSprite = BACK;
    }
    break;
  case DOWN:
    if (gs->playerMoving) {
      keeperSprite = WALK_DOWN;
    } else {
      keeperSprite = FRONT;
    }
    break;
  case LEFT:
    if (gs->playerMoving) {
      keeperSprite = WALK_LEFT;
    } else {
      keeperSprite = SIDE;
    }
    break;
  case RIGHT:
    if (gs->playerMoving) {
      keeperSprite = WALK_RIGHT;
    } else {
      keeperSprite = SIDE;
    }
    break;
  }

  Vector2 playerSpritePosition = {gs->playerPosition.x - 24,
                                  gs->playerPosition.y - 28};
  drawAnimationFrame(&keeperSprites[keeperSprite], playerSpritePosition);

  // Draw key if close to shop
  if (gs->playerNearShop) {
    drawAnimationFrame(&keyZ, SHOP_POSITION);
  } else {
    // If nearest hive is close, show key-z button.
    Vector2 nearestHivePixelPos =
        gardenHexPositionToPixelPosition(gs->hives[gs->nearestHive]->position);
    float distanceToNearestHive =
        vector2Distance(gs->playerPosition, nearestHivePixelPos);
    if (distanceToNearestHive < 100) {
      Vector2 keyZPos = nearestHivePixelPos;
      keyZPos.x -= 8;
      keyZPos.y -= 42;
      drawAnimationFrame(&keyZ, keyZPos);
    }
  }

  EndMode2D();
}

void drawHives(void) {
  for (unsigned int i = 0; i < gs->numHives; i++) {
    Vector2 hivePixelPosition =
        gardenHexPositionToPixelPosition(gs->hives[i]->position);
    hivePixelPosition.x -= 32;
    hivePixelPosition.y -= 55;
    drawAnimationFrame(&gs->hives[i]->animation, hivePixelPosition);
  }
}

void drawFlowers(void) {
  for (unsigned int i = 0; i < gs->numFlowers; i++) {
    Vector2 flowerPixelPosition =
        gardenHexPositionToPixelPosition(gs->flowers[i]->position);

    flowerPixelPosition.x -= 20;
    flowerPixelPosition.y -= 30;

    switch (gs->flowers[i]->type) {
    case FLOWER_ZINNIAS: {
      DrawTextureV(zinniasSprite, flowerPixelPosition, WHITE);
      break;
    }
    case FLOWER_DAHLIAS: {
      DrawTextureV(dahliasSprite, flowerPixelPosition, WHITE);
      break;
    }
    case FLOWER_LAVENDERS: {
      DrawTextureV(lavenderSprite, flowerPixelPosition, WHITE);
      break;
    }
    case FLOWER_SUNFLOWERS: {
      DrawTextureV(sunflowerSprite, flowerPixelPosition, WHITE);
      break;
    }
    case FLOWER_NONE:
    case FLOWER_EMPTY: {
      // Relevant elsewhere
      break;
    }
    }
  }
}

// Return nearest hex coordinates from pixel point
// TODO: Definitely there is a better way to do this.
Vector2 gardenHexFromPoint(Vector2 point) {
  Vector2 closest;
  int closestDistance = 9999;

  for (unsigned int row = 0; row < 13; row++) {
    int numColumns = 13;
    if (row % 2 == 1)
      numColumns = 12;
    for (unsigned int column = 0; column < numColumns; column++) {
      Vector2 hexCoord = {column, row};
      Vector2 pixelCoord = gardenHexPositionToPixelPosition(hexCoord);

      int distance = vector2Distance(pixelCoord, point);
      if (distance < GARDEN_HEX_SIZE) {
        return hexCoord;
      }

      if (distance < closestDistance) {
        closestDistance = distance;
        closest = hexCoord;
      }
    }
  }

  return closest;
}

bool isGardenHexOccupied(Vector2 position) {
  // Check hives
  for (unsigned int i = 0; i < gs->numHives; i++) {
    if (position.x == gs->hives[i]->position.x &&
        position.y == gs->hives[i]->position.y) {
      return true;
    }
  }

  // Check flowers
  for (unsigned int i = 0; i < gs->numFlowers; i++) {
    if (position.x == gs->flowers[i]->position.x &&
        position.y == gs->flowers[i]->position.y) {
      return true;
    }
  }

  return false;
}

Flower* initFlower(FlowerType type, unsigned int x, unsigned int y) {
    Flower *f = (Flower*) malloc(sizeof(Flower));
    f->type = type;
    f->position = (Vector2){x, y};
    return f;
}

#endif
