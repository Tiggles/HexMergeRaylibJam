#ifndef SHOP_GUARD
#define SHOP_GUARD

#include "raylib.h"
#include "button.c"
#include "globals.c"

void updateShopScene(void) {
  SetMouseCursor(MOUSE_CURSOR_DEFAULT);

  // Update buttons
  for (unsigned int i = 0; i < 5; i++) {
    updateButton(&purchaseButtons[i]);
  }
  updateButton(&exitShopButton);

  if (exitShopButton.isClicked) {
    gs->currentScene = GARDEN;
    return;
  }

  purchaseButtons[0].isDisabled = gs->money < HIVE_PRICE;
  purchaseButtons[1].isDisabled = gs->money < ZINNIAS_PRICE;
  purchaseButtons[2].isDisabled = gs->money < DAHLIAS_PRICE;
  purchaseButtons[3].isDisabled = gs->money < LAVENDERS_PRICE;
  purchaseButtons[4].isDisabled = gs->money < SUNFLOWERS_PRICE;

  if (purchaseButtons[0].isClicked && gs->money >= HIVE_PRICE) {
    gs->currentScene = BUILD;
    gs->currentlyBuilding = BUILD_HIVE;
  } else if (purchaseButtons[1].isClicked && gs->money >= ZINNIAS_PRICE) {
    gs->currentScene = BUILD;
    gs->currentlyBuilding = BUILD_ZINNIAS;
  } else if (purchaseButtons[2].isClicked && gs->money >= DAHLIAS_PRICE) {
    gs->currentScene = BUILD;
    gs->currentlyBuilding = BUILD_DAHLIAS;
  } else if (purchaseButtons[3].isClicked && gs->money >= LAVENDERS_PRICE) {
    gs->currentScene = BUILD;
    gs->currentlyBuilding = BUILD_LAVENDERS;
  } else if (purchaseButtons[4].isClicked && gs->money >= SUNFLOWERS_PRICE) {
    gs->currentScene = BUILD;
    gs->currentlyBuilding = BUILD_SUNFLOWERS;
  }
}

void drawShopScene(void) {
  // Draw background
  DrawTexture(shopBg, 0, 0, WHITE);

  // Draw descriptions
  DrawTextEx(font20, "A beehive full of bees. Produces delicious honey.",
             (Vector2){165, 117}, 20, 0, DARKBROWN);
  DrawTextEx(font20, "Easy to grow, and comes in a large variety of colors.",
             (Vector2){165, 237}, 20, 0, DARKBROWN);
  DrawTextEx(font20,
             "Plant near a hive to make the bees produce zinnias (red) honey.",
             (Vector2){165, 257}, 20, 0, DARKBROWN);
  DrawTextEx(font20,
             "Their open pedals makes it easy for bees to reach the nectar.",
             (Vector2){165, 357}, 20, 0, DARKBROWN);
  DrawTextEx(font20,
             "Plant near a hive to make the bees produce dahlia (pink) honey.",
             (Vector2){165, 377}, 20, 0, DARKBROWN);
  DrawTextEx(font20, "Bees love the fragrance, which also repels pests.",
             (Vector2){165, 477}, 20, 0, DARKBROWN);
  DrawTextEx(
      font20,
      "Plant near a hive to make the bees produce lavender (purple) honey.",
      (Vector2){165, 497}, 20, 0, DARKBROWN);
  DrawTextEx(font20, "Rich in nectar and pollen. A favorite among bees.",
             (Vector2){165, 597}, 20, 0, DARKBROWN);
  DrawTextEx(
      font20,
      "Plant near a hive to make the bees produce sunflower (orange) honey.",
      (Vector2){165, 617}, 20, 0, DARKBROWN);

  // Draw prices
  DrawTexture(coin, 500, 170, WHITE);
  if (gs->money >= HIVE_PRICE) {
    DrawTextEx(font30, TextFormat("%i", HIVE_PRICE), (Vector2){520, 163}, 30, 0,
               DARKBROWN);
  } else {
    DrawTextEx(font30, TextFormat("%i", HIVE_PRICE), (Vector2){520, 163}, 30, 0,
               RED);
  }

  DrawTexture(coin, 500, 289, WHITE);
  if (gs->money >= ZINNIAS_PRICE) {
    DrawTextEx(font30, TextFormat("%i", ZINNIAS_PRICE), (Vector2){520, 282}, 30,
               0, DARKBROWN);
  } else {
    DrawTextEx(font30, TextFormat("%i", ZINNIAS_PRICE), (Vector2){520, 282}, 30,
               0, RED);
  }

  DrawTexture(coin, 500, 408, WHITE);
  if (gs->money >= DAHLIAS_PRICE) {
    DrawTextEx(font30, TextFormat("%i", DAHLIAS_PRICE), (Vector2){520, 401}, 30,
               0, DARKBROWN);
  } else {
    DrawTextEx(font30, TextFormat("%i", DAHLIAS_PRICE), (Vector2){520, 401}, 30,
               0, RED);
  }

  DrawTexture(coin, 500, 527, WHITE);
  if (gs->money >= LAVENDERS_PRICE) {
    DrawTextEx(font30, TextFormat("%i", LAVENDERS_PRICE), (Vector2){520, 520},
               30, 0, DARKBROWN);
  } else {
    DrawTextEx(font30, TextFormat("%i", LAVENDERS_PRICE), (Vector2){520, 520},
               30, 0, RED);
  }

  DrawTexture(coin, 500, 646, WHITE);
  if (gs->money >= SUNFLOWERS_PRICE) {
    DrawTextEx(font30, TextFormat("%i", SUNFLOWERS_PRICE), (Vector2){520, 639},
               30, 0, DARKBROWN);
  } else {
    DrawTextEx(font30, TextFormat("%i", SUNFLOWERS_PRICE), (Vector2){520, 639},
               30, 0, RED);
  }

  // Draw buttons
  for (unsigned int i = 0; i < 5; i++) {
    drawButton(&purchaseButtons[i]);
  }
  drawButton(&exitShopButton);
}

#endif
