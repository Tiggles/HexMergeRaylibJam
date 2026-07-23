#ifndef HARVEST_GUARD
#define HARVEST_GUARD

#include "raylib.h"
#include <stdio.h>
#include <math.h>

#include "types.c"
#include "globals.c"
#include "consts.c"
#include "func_decls.c"
#include "animation.c"

FlowerType chooseFlower(Hive *h) {
  Vector2 hivePosition = h->position;

  int total = 7;
  int zinnias = 0;
  int dahlias = 0;
  int lavenders = 0;
  int sunflowers = 0;

  for (int i = 0; i < gs->numFlowers; i++) {
    Flower *f = gs->flowers[i];
    if (isTileNeighbor(f->position, hivePosition)) {
      switch (f->type) {
      case FLOWER_NONE:
        break;
      case FLOWER_ZINNIAS:
        zinnias += 1;
        break;
      case FLOWER_DAHLIAS:
        dahlias += 1;
        break;
      case FLOWER_LAVENDERS:
        lavenders += 1;
        break;
      case FLOWER_SUNFLOWERS:
        sunflowers += 1;
        break;
      default: {
      }
      }
    }
  }

  int val = GetRandomValue(0, total);

  if (val < zinnias) {
    return FLOWER_ZINNIAS;
  }
  val -= zinnias;
  if (val < dahlias) {
    return FLOWER_DAHLIAS;
  }
  val -= dahlias;
  if (val < lavenders) {
    return FLOWER_LAVENDERS;
  }
  val -= lavenders;
  if (val < sunflowers) {
    return FLOWER_SUNFLOWERS;
  }

  return FLOWER_NONE;
}

void assignHexTile(Hive *h) {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      int c = GetRandomValue(0, COLUMN_COUNT - 1);
      int rowCount = c % 2 == 0 ? ROW_COUNT_EVEN : ROW_COUNT_UNEVEN;
      int r = GetRandomValue(0, rowCount - 1);
      if (h->hexes[c][r]->flowerType != FLOWER_EMPTY)
        continue;
      h->hexes[c][r]->flowerType = chooseFlower(h);
      h->hexes[c][r]->timeUntilReadyMS = DEFAULT_TIME_UNTIL_READY;
      return;
    }
  }
}

void drawHarvestScene(void) {
  if (gs->activeHiveIndex == -1) {
    printf("No activeHiveIndex set. Bailing from function.\n");
    return;
  }
  DrawTexture(harvestBg, 0, 0, WHITE);
  drawButton(&sellHoneyButton);
  drawButton(&exitHarvestButton);

  Hive *h = gs->hives[gs->activeHiveIndex];
  for (int c = 0; c < COLUMN_COUNT; c++) {
    int rowCount = c % 2 == 0 ? ROW_COUNT_EVEN : ROW_COUNT_UNEVEN;
    for (int r = 0; r < rowCount; r++) {
      HarvestHex *hh = h->hexes[c][r];
      if (hh->flowerType != FLOWER_EMPTY) {
        Vector2 pos = harvestHexDrawingCoordinates((Vector2){.x = r, .y = c});
        Color color = WHITE;
        if (hh->timeUntilReadyMS > 0) {
          color.a =
              255 - (hh->timeUntilReadyMS / DEFAULT_TIME_UNTIL_READY) * 255;
        }
        Texture2D *t = &hexRed;
        switch (hh->flowerType) {
        case FLOWER_NONE: {
          t = &hexYellow;
          break;
        }
        case FLOWER_ZINNIAS: {
          t = &hexRed;
          break;
        }
        case FLOWER_DAHLIAS: {
          t = &hexPink;
          break;
        }
        case FLOWER_LAVENDERS: {
          t = &hexPurple;
          break;
        }
        case FLOWER_SUNFLOWERS: {
          t = &hexOrange;
          break;
        }
        default: {
        }
        }
        DrawTexture(*t, pos.x, pos.y, color);
      }
    }
  }

  if (CheckCollisionPointRec(GetMousePosition(), HexGridRect)) {
    drawHarvestHex();
  }

  for (int i = 0; i < HARVEST_CHAIN_COUNT; i++) {
    Vector2 v = gs->harvestChain[i];
    if (v.x == -1 && v.y == -1)
      break;
    Vector2 pos = harvestHexDrawingCoordinates(v);
    DrawTexture(hexOutline, pos.x, pos.y, WHITE);
  }

  DrawTextEx(font20, "Click 3 adjacent filled tiles to merge and harvest honey",
             (Vector2){215, 530}, 20, 0, WHITE);
  DrawTextEx(font20, "Click alternate colors for higher multiplier",
             (Vector2){245, 550}, 20, 0, WHITE);
  DrawTextEx(
      font20,
      "Click \"Sell honey\" when the jar is full, to continue harvesting",
      (Vector2){180, 570}, 20, 0, WHITE);
  DrawTextEx(font20, "Right click to cancel tile selection",
             (Vector2){270, 590}, 20, 0, WHITE);

  drawJar();
}

void updateHarvestScene() {
  updateButton(&sellHoneyButton);
  sellHoneyButton.isDisabled = gs->jar.iteration != JAR_ITERATIONS - 1;
  updateButton(&exitHarvestButton);

  if (!sellHoneyButton.isDisabled && sellHoneyButton.isClicked) {
    gs->jar.iteration = 0;
    gs->money += gs->jar.value;
    gs->jar.value = 0;
    PlaySound(sellHoney);
  }

  if (exitHarvestButton.isClicked) {
    clearHarvestChain();
    gs->currentScene = GARDEN;
  }

  if (gs->activeHiveIndex == -1) {
    printf("No activeHiveIndex set. Bailing from function.\n");
    return;
  }

  Hive *h = gs->hives[gs->activeHiveIndex];

  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    if (gs->jar.iteration == JAR_ITERATIONS - 1)
      return;

    int isNeighbor = false;

    Vector2 point = mouseToHexPointCoordinates();
    if (point.x == -1 && point.y == -1)
      return;
    HarvestHex *hex = h->hexes[(int)point.y][(int)point.x];
    if (hex->flowerType == FLOWER_EMPTY || hex->timeUntilReadyMS > 0)
      return;
    // Check for existing harvest-chain selection to disallow duplicates
    int duplicate = false;
    int firstFreeIdx = -1;

    for (int i = 0; i < HARVEST_CHAIN_COUNT; i++) {
      Vector2 hex = gs->harvestChain[i];
      if (hex.x == -1 && hex.y == -1) {
        firstFreeIdx = i;
        break;
      }

      isNeighbor |= isTileNeighbor(hex, point);

      if (hex.x == point.x && hex.y == point.y) {
        duplicate = true;
        break;
      }
    }

    if (!duplicate && firstFreeIdx != -1) {
      if (firstFreeIdx < HARVEST_CHAIN_COUNT) {
        if (firstFreeIdx != 0) {
          // Priorier entry
          if (!isNeighbor) {
            return;
          }
        }
        gs->harvestChain[firstFreeIdx] = point;
      }
    }

    if (firstFreeIdx == HARVEST_CHAIN_COUNT - 1) {
      harvestActiveChain();
      clearHarvestChain();
    }
  } else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) ||
             IsKeyDown(KEY_ESCAPE)) {
    clearHarvestChain();
  }
}

int flowerTypeToMoney(FlowerType t) {
  switch (t) {
  case FLOWER_ZINNIAS:
    return 20;
  case FLOWER_DAHLIAS:
    return 30;
  case FLOWER_LAVENDERS:
    return 40;
  case FLOWER_SUNFLOWERS:
    return 50;
  case FLOWER_NONE:
    return 10;
  case FLOWER_EMPTY:
  default:
    return 0;
  }
}

void harvestActiveChain(void) {
  int money = 0;
  float multiplier = 0.8;
  FlowerType lastFlowerType = FLOWER_EMPTY;
  for (int i = 0; i < HARVEST_CHAIN_COUNT; i++) {
    Vector2 *p = &gs->harvestChain[i];
    HarvestHex *h = gs->hives[gs->activeHiveIndex]->hexes[(int)p->y][(int)p->x];
    FlowerType currentFlowerType = h->flowerType;
    if (currentFlowerType != lastFlowerType) {
      multiplier += 0.2;
    }
    lastFlowerType = currentFlowerType;
    money += flowerTypeToMoney(currentFlowerType) * multiplier;

    p->x = -1;
    p->y = -1;

    h->flowerType = FLOWER_EMPTY;
    h->timeUntilReadyMS = -1;
  }
  gs->jar.iteration += 1;
  gs->jar.value += money;
}

void clearHarvestChain(void) {
  for (int i = 0; i < HARVEST_CHAIN_COUNT; i++) {
    Vector2 *hex = &gs->harvestChain[i];
    hex->x = -1;
    hex->y = -1;
  }
}

void drawJar() {
  float textureWidth = honeyGlassSprites.texture.width;
  float spriteWidth = textureWidth / honeyGlassSprites.numFrames;
  float spriteHeight = (float)honeyGlassSprites.texture.height;
  Rectangle frameRec = {
      spriteWidth * gs->jar.iteration,
      0,
      spriteWidth,
      spriteHeight,
  };

  Rectangle destRec = {0, 520, spriteWidth, spriteHeight};

  Vector2 origin = {0, 0};
  DrawTexturePro(honeyGlassSprites.texture, frameRec, destRec, origin, 0,
                 WHITE);

  DrawTextEx(font30, TextFormat("value: %i", gs->jar.value),
             (Vector2){180, 650}, 30, 0, WHITE);
}


Vector2 mouseToHexPointCoordinates(void) {
  Vector2 mouse = GetMousePosition();
  int heightOfHex = HexGridRect.height / COLUMN_COUNT;
  int column = (mouse.y - HexGridRect.y) / heightOfHex;
  int isEven = column % 2 == 0;
  if (!isEven) {
    if (mouse.x < HexGridRect.x + 24 ||
        mouse.x > HexGridRect.x + HexGridRect.width - 24) {
      return (Vector2){.x = -1, .y = -1};
    }
    mouse.x -= UNEVEN_ROW_X_OFFSET;
  }

  int widthOfHex = HexGridRect.width / (ROW_COUNT_EVEN);
  int row = (mouse.x - HexGridRect.x) / widthOfHex;
  return (Vector2){
      .x = row >= 14 ? 14 : row,
      .y = column > COLUMN_COUNT - 1 ? COLUMN_COUNT - 1 : column,
  };
}

Hive *initHive(unsigned int x, unsigned int y) {
  Hive *h = malloc(sizeof(Hive));
  h->hexes = malloc(sizeof(size_t) * COLUMN_COUNT);
  for (int c = 0; c < COLUMN_COUNT; c++) {
    h->hexes[c] = malloc(sizeof(size_t) * ROW_COUNT_EVEN);
    for (int r = 0; r < ROW_COUNT_EVEN; r++) {
      h->hexes[c][r] = initHarvestHex();
    }
  }

  int fourthOfColumnCount = COLUMN_COUNT / 4;
  int fourthOfRowCount = ROW_COUNT_UNEVEN / 4;
  for (int c = 0; c < fourthOfColumnCount; c++) {
      for (int r = 0; r < fourthOfColumnCount; r++) {
          FlowerType f = chooseFlower(h);
          h->hexes[c + fourthOfColumnCount * 2][r + fourthOfRowCount * 2]->flowerType = f;
      }
  }

  h->position = (Vector2){x, y};
  h->nextHexStartFill = NEXT_FILL_TIME_IN_SECONDS;
  h->animation = initAnimationFromTexture(hiveSprite, 3, 200);

  return h;
}

HarvestHex *initHarvestHex() {
  HarvestHex *h = malloc(sizeof(HarvestHex));
  h->flowerType = FLOWER_EMPTY;
  h->timeUntilReadyMS = -1;
  return h;
}

int isTileNeighbor(Vector2 t1, Vector2 t2) {

  if (t1.y == t2.y) {
    return t1.x - 1 == t2.x || t1.x + 1 == t2.x;
  }

  int isEven = (int)(t1.y) % 2 == 0;
  if (isEven) {
    if (fabsf(t1.y - t2.y) > 1)
      return false;
    return t1.x - 1 == t2.x || t1.x == t2.x;
  } else {
    if (fabsf(t1.y - t2.y) > 1)
      return false;
    return t1.x == t2.x || t1.x + 1 == t2.x;
  }

  return false;
}


#define SMALL_DELTA_BETWEEN_HEXES 3
void drawHarvestHex(void) {
  Vector2 point = mouseToHexPointCoordinates(); // x is row, y is column
  if (point.x == -1 || point.y == -1)
    return;
  Vector2 pos = harvestHexDrawingCoordinates(point);
  Color color = WHITE;
  if (gs->jar.iteration == JAR_ITERATIONS - 1) {
    color.r = 255;
    color.g = 0;
    color.b = 0;
  }
  DrawTexture(hexOutlineLight, pos.x, pos.y, color);
}

Vector2 harvestHexDrawingCoordinates(Vector2 pos) {
  int evenColumn = ((int)pos.y) % 2 == 0;
  int xOffset = (evenColumn ? 4 : UNEVEN_ROW_X_OFFSET) +
                (SMALL_DELTA_BETWEEN_HEXES * pos.x);
  int x = hexOutline.width * pos.x + HexGridRect.x + (xOffset);
  int y = (hexOutline.height - 9) * pos.y + HexGridRect.y +
          (SMALL_DELTA_BETWEEN_HEXES * pos.y);
  return (Vector2){.x = x, .y = y};
}

#endif
