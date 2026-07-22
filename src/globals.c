#ifndef GLOBALS_GUARD
#define GLOBALS_GUARD
#include "raylib.h"
#include "types.c"
#include "stdlib.h"

const int screenWidth = 720;
const int screenHeight = 720;
const Vector2 SHOP_POSITION = {660, -20};
const int MOVEMENT_SPEED = 200;
const int GARDEN_HEX_SIZE = 30;
const int HIVE_PRICE = 10000;
const int ZINNIAS_PRICE = 500;
const int DAHLIAS_PRICE = 1000;
const int LAVENDERS_PRICE = 2000;
const int SUNFLOWERS_PRICE = 5000;
const int STARTING_MONEY = 5000;

Texture2D hiveSprite;
Texture2D harvestBg;
Texture2D menuBg;
Texture2D gardenBg;
Texture2D shopBg;
Texture2D aboutBg;
Texture2D zinniasSprite;
Texture2D dahliasSprite;
Texture2D lavenderSprite;
Texture2D sunflowerSprite;
Animation keeperSprites[7];
Animation keyZ;
Animation honeyGlassSprites;
Texture2D coin;
Texture2D hexOutline;
Texture2D hexOutlineLight;
Texture2D hexOrange;
Texture2D hexPurple;
Texture2D hexPink;
Texture2D hexRed;
Texture2D hexYellow;
Camera2D gardenCamera;
Texture2D purchaseButtonSprite;
Rectangle HexGridRect = {
    .x = 73,
    .y = 73,
    .width = 575,
    .height = 415,
};
Music music;
Sound sellHoney;
Sound thump;

Button startButton;
Button menuButton;
Button aboutButton;
Button backToMenuButton;
Button exitHarvestButton;
Button sellHoneyButton;
Button cancelPurchaseButton;
Button exitShopButton;
Button purchaseButtons[5];

Font font20;
Font font30;

RenderTexture2D target = {0}; // Render texture to render our game
int frameCounter = 0;

struct GameState *gs = NULL;
#endif
