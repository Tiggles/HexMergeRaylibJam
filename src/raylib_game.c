/********************************************************************************************
 *
 *   raylib gamejam template
 *
 *   Code licensed under an unmodified zlib/libpng license, which is an
 * OSI-certified, BSD-like license that allows static linking with closed source
 * software
 *
 *   Copyright (c) 2022-2026 Ramon Santamaria (@raysan5)
 *
 ********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h> // Emscripten library
#endif

#include <math.h>   // Required for: sqrt(), powf()
#include <stdlib.h> // Required for:

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Simple log system to avoid printf() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO)
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
#include "consts.c"
#include "types.c"

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------
#include "globals.c"
//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
#include "func_decls.c"

#include "about.c"
#include "animation.c"
#include "build.c"
#include "button.c"
#include "garden.c"
#include "harvest.c"
#include "menu.c"
#include "shop.c"
//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
#if !defined(_DEBUG)
  SetTraceLogLevel(LOG_NONE); // Disable raylib trace log messages
#endif

  // Initialization
  //--------------------------------------------------------------------------------------
  InitWindow(screenWidth, screenHeight, "Best Bee-uddies");
  InitAudioDevice();

  // Check(Brian): Fjerner muligheden for at lukke spillet med Escape. Vil gerne
  // bruge escape til at afbryde en kæde ved harvesting.
  SetExitKey(KEY_NULL);

  gs = malloc(sizeof(struct GameState));
  gs->currentScene = MENU;
  gs->playerPosition = (Vector2){490, 120};
  gs->playerDirection = DOWN;
  gs->playerMoving = false;
  gs->playerNearShop = false;
  gs->money = STARTING_MONEY;
  gs->currentlyBuilding = BUILD_NULL;
  gs->activeHiveIndex = -1;
  gs->nearestHive = 0;

  gs->jar = (Jar){
      .iteration = 0,
      .value = 0,
  };

  for (int i = 0; i < HARVEST_CHAIN_COUNT; i++) {
    gs->harvestChain[i] = (Vector2){
        .x = -1,
        .y = -1,
    };
  }

  gs->flowers = malloc(sizeof(Flower *) * 200);
  gs->numFlowers = 0;

  // Initialize garden camera
  gardenCamera.target = gs->playerPosition;
  gardenCamera.offset = (Vector2){screenWidth / 2.0f, screenHeight / 2.0f};
  gardenCamera.rotation = 0.0f;
  gardenCamera.zoom = 2.0f;

  hiveSprite = LoadTexture("resources/hive.png");
  harvestBg = LoadTexture("resources/harvest_bg.png");
  menuBg = LoadTexture("resources/menu_bg.png");
  gardenBg = LoadTexture("resources/garden_bg.png");
  shopBg = LoadTexture("resources/shop_bg.png");
  aboutBg = LoadTexture("resources/about_bg.png");
  keyZ = initAnimation("resources/key_z.png", 10, 200);
  coin = LoadTexture("resources/coin.png");
  zinniasSprite = LoadTexture("resources/zinnias.png");
  dahliasSprite = LoadTexture("resources/dahlias.png");
  lavenderSprite = LoadTexture("resources/lavender.png");
  sunflowerSprite = LoadTexture("resources/sunflowers.png");
  hexOutline = LoadTexture("resources/hex_outline.png");
  hexOutlineLight = LoadTexture("resources/hex_outline_light.png");
  hexOrange = LoadTexture("resources/hex_orange.png");
  hexPurple = LoadTexture("resources/hex_purple.png");
  hexPink = LoadTexture("resources/hex_pink.png");
  hexRed = LoadTexture("resources/hex_red.png");
  hexYellow = LoadTexture("resources/hex_yellow.png");
  startButton.texture = LoadTexture("resources/start_button.png");
  menuButton.texture = LoadTexture("resources/menu_button.png");
  aboutButton.texture = LoadTexture("resources/about_button.png");
  backToMenuButton.texture = LoadTexture("resources/back_to_menu_button.png");
  exitHarvestButton.texture =
      LoadTexture("resources/back_to_garden_button.png");
  exitShopButton.texture = LoadTexture("resources/back_to_garden_button.png");
  sellHoneyButton.texture = LoadTexture("resources/sell_honey_button.png");
  cancelPurchaseButton.texture =
      LoadTexture("resources/cancel_purchase_button.png");
  purchaseButtonSprite = LoadTexture("resources/purchase_button.png");
  keeperSprites[BACK] = initAnimation("resources/character_back.png", 2, 500);
  keeperSprites[FRONT] = initAnimation("resources/character_front.png", 2, 500);
  keeperSprites[SIDE] = initAnimation("resources/character_side.png", 2, 500);
  keeperSprites[WALK_DOWN] =
      initAnimation("resources/character_walk_down.png", 6, 100);
  keeperSprites[WALK_UP] =
      initAnimation("resources/character_walk_up.png", 6, 100);
  keeperSprites[WALK_LEFT] =
      initAnimation("resources/character_walk_left.png", 4, 200);
  keeperSprites[WALK_RIGHT] =
      initAnimation("resources/character_walk_right.png", 4, 200);
  honeyGlassSprites = initAnimation("resources/honey_glass.png", 11, 0);
  font20 =
      LoadFontEx("resources/fonts/Jersey_10/Jersey10-Regular.ttf", 20, 0, 250);
  font30 =
      LoadFontEx("resources/fonts/Jersey_10/Jersey10-Regular.ttf", 30, 0, 250);

  // Music
  music = LoadMusicStream(
      "resources/music/calm-acoustic-guitar-for-serene-moments.mp3");

  sellHoney = LoadSound("resources/sounds/554841__lucish__cha_ching.mp3");
  thump = LoadSound(
      "resources/sounds/431204__bunnyluvvid__rabbit-thump-on-soil-edited.wav");

  gs->hives = malloc(sizeof(Hive *) * 16);
  gs->numHives = 0;
  gs->hives[0] = initHive(9, 2);
  gs->numHives++;

  // Initialize buttons
  startButton.position = (Vector2){20, 600};
  startButton.isDisabled = false;
  menuButton.position = (Vector2){20, 600};
  menuButton.isDisabled = false;
  aboutButton.position = (Vector2){122, 600};
  aboutButton.isDisabled = false;
  backToMenuButton.position = (Vector2){549, 26};
  backToMenuButton.isDisabled = false;
  exitHarvestButton.position = (Vector2){570, 640};
  exitHarvestButton.isDisabled = false;
  cancelPurchaseButton.position = (Vector2){10, 57};
  cancelPurchaseButton.isDisabled = false;
  sellHoneyButton.position = (Vector2){570, 602};
  sellHoneyButton.isDisabled = false;
  exitShopButton.position = (Vector2){567, 27};
  exitShopButton.isDisabled = false;

  purchaseButtons[0] = initButton(purchaseButtonSprite, (Vector2){604, 160});
  purchaseButtons[1] = initButton(purchaseButtonSprite, (Vector2){604, 279});
  purchaseButtons[2] = initButton(purchaseButtonSprite, (Vector2){604, 398});
  purchaseButtons[3] = initButton(purchaseButtonSprite, (Vector2){604, 517});
  purchaseButtons[4] = initButton(purchaseButtonSprite, (Vector2){604, 636});

  PlayMusicStream(music);
  SetMusicPan(music, 0.0);
  SetMusicVolume(music, 0.8);

  // TODO: Load resources / Initialize variables at this point

  // Render texture to draw, enables screen scaling
  // NOTE: If screen is scaled, mouse input should be scaled proportionally
  target = LoadRenderTexture(screenWidth, screenHeight);
  SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
  SetTargetFPS(60); // Set our game frames-per-second
  //--------------------------------------------------------------------------------------

  // Main game loop
  while (!WindowShouldClose()) { // Detect window close button
    UpdateDrawFrame();
  }
#endif

  // De-Initialization
  //--------------------------------------------------------------------------------------
  UnloadRenderTexture(target);

  // TODO: Unload all loaded resources at this point
  for (unsigned int i = 0; i < gs->numHives; i++) {
    unloadAnimation(&gs->hives[i]->animation);
  }
  UnloadFont(font20);
  UnloadFont(font30);
  UnloadMusicStream(music);
  UnloadSound(thump);
  UnloadSound(sellHoney);

  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}

//--------------------------------------------------------------------------------------------
// Module Functions Definition
//--------------------------------------------------------------------------------------------

// Check hive/player collision
bool isHiveCollision(Vector2 playerPosition) {
  Rectangle playerRec = {
      playerPosition.x - 15,
      playerPosition.y - 20,
      28,
      50,
  };

  for (unsigned int i = 0; i < gs->numHives; i++) {
    Vector2 hivePixelPosition = gardenHexPositionToPixelPosition(
        (Vector2){gs->hives[i]->position.x, gs->hives[i]->position.y});

    Rectangle hiveRec = {
        hivePixelPosition.x - 16,
        hivePixelPosition.y - 22,
        30,
        10,
    };

    if (CheckCollisionRecs(playerRec, hiveRec)) {
      return true;
    }
  }
  return false;
}

// Returns the pixel coordinates for the garden hexgrid based on the hexgrid
// coordinates
Vector2 gardenHexPositionToPixelPosition(Vector2 hexCoordinates) {
  int column = hexCoordinates.x;
  int row = hexCoordinates.y;
  return (Vector2){
      50 + sqrt(3) * GARDEN_HEX_SIZE * column +
          (row % 2) * sqrt(3) * GARDEN_HEX_SIZE / 2,
      130 + 3 / 2.0f * GARDEN_HEX_SIZE * row,
  };
}

void drawHud(void) {
  if (gs->currentScene == MENU || gs->currentScene == ABOUT) {
    return;
  }

  const char *moneyString = TextFormat("%i", gs->money);
  Vector2 stringSize = MeasureTextEx(font30, moneyString, 30, 0);
  DrawTexture(coin, 670 - stringSize.x, 690, WHITE);
  DrawTextEx(font30, moneyString, (Vector2){690 - stringSize.x, 683}, 30, 0,
             WHITE);
}

// Update and draw frame
void UpdateDrawFrame(void) {
  // Update
  //----------------------------------------------------------------------------------
  gs->playerMoving = false;
  SetMouseCursor(MOUSE_CURSOR_DEFAULT);
  UpdateMusicStream(music);

  // Handle next placement of hex-content
  if (gs->currentScene != ABOUT && gs->currentScene != MENU) {
    float delta = GetFrameTime();
    for (int i = 0; i < gs->numHives; i++) {
      Hive *h = gs->hives[i];
      if (h->nextHexStartFill > 0) {
        h->nextHexStartFill -= delta;
      } else {
        assignHexTile(h);
        h->nextHexStartFill = NEXT_FILL_TIME_IN_SECONDS;
      }

      for (int c = 0; c < COLUMN_COUNT; c++) {
        int rowCount = c % 2 == 0 ? ROW_COUNT_EVEN : ROW_COUNT_UNEVEN;
        for (int r = 0; r < rowCount; r++) {
          HarvestHex *hh = h->hexes[c][r];
          if (hh->flowerType != FLOWER_EMPTY) {
            if (hh->timeUntilReadyMS > 0) {
              hh->timeUntilReadyMS -= delta;
            }
          }
        }
      }
    }
  }

  switch (gs->currentScene) {
  case GARDEN:
    updateGardenScene();
    break;
  case SHOP:
    updateShopScene();
    break;
  case BUILD:
    updateBuildScene();
    break;
  case MENU:
    updateMenu();
    break;
  case ABOUT:
    updateAbout();
    break;
  case HARVEST:
    updateHarvestScene();
    break;
  default:
    // TODO
    break;
  }

  frameCounter++;
  //----------------------------------------------------------------------------------

  // Draw
  //----------------------------------------------------------------------------------
  // Render game screen to a texture,
  // it could be useful for scaling or further shader postprocessing
  BeginTextureMode(target);
  {
    ClearBackground(GRASSGREEN);

    switch (gs->currentScene) {
    case MENU: {
      drawMenu();
      break;
    }
    case GARDEN: {
      drawGardenScene();
      break;
    }
    case SHOP: {
      drawShopScene();
      break;
    }
    case BUILD: {
      drawBuildScene();
      break;
    }
    case HARVEST: {
      drawHarvestScene();
      break;
    }
    case ABOUT: {
      drawAbout();
      break;
    }
    }
  }
  EndTextureMode();

  // Render to screen (main framebuffer)
  BeginDrawing();
  {
    ClearBackground(GRASSGREEN);

    // Draw render texture to screen, scaled if required
    DrawTexturePro(target.texture,
                   (Rectangle){0, 0, (float)target.texture.width,
                               -(float)target.texture.height},
                   (Rectangle){0, 0, (float)target.texture.width,
                               (float)target.texture.height},
                   (Vector2){0, 0}, 0.0f, WHITE);

    // TODO: Draw everything that requires to be drawn at this point, maybe UI?
    drawHud();
  }
  EndDrawing();
  //----------------------------------------------------------------------------------
}

// TODOS!
// - [ ] Fancy animation when honeyGlass is filled
// - [ ] Lower row of harvesting grid has funky behaviour
