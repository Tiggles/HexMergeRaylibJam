/********************************************************************************************
*
*   raylib gamejam template
*
*   Code licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2022-2026 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>      // Emscripten library
#endif

#include <stdio.h>                          // Required for: printf()
#include <stdlib.h>                         // Required for: 
#include <string.h>                         // Required for:
#include <math.h>                           // Required for: sqrt(), powf()

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
typedef enum {
    SCREEN_LOGO = 0,
    SCREEN_TITLE,
    SCREEN_GAMEPLAY,
    SCREEN_ENDING
} GameScreen;

// TODO: Define your custom data types here

enum CurrentScene {
    MENU,
    GARDEN,
    HARVEST,
    SHOP,
    BUILD,
    ABOUT,
};

typedef enum {
    BUILD_NULL = 0,
    BUILD_HIVE = 1,
    BUILD_ZINNIAS = 2,
    BUILD_DAHLIAS = 3,
    BUILD_LAVENDERS = 4,
    BUILD_SUNFLOWERS = 5,
} BuildType;

typedef enum {
    BACK = 0,
    FRONT = 1,
    SIDE = 2,
    WALK_DOWN = 3,
    WALK_UP = 4,
    WALK_LEFT = 5,
    WALK_RIGHT = 6,
} KeeperSprite;

typedef enum {
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3,
} KeeperDirection;

typedef enum {
    FLOWER_EMPTY = -2,
    FLOWER_NONE = -1,
    FLOWER_ZINNIAS = 0,
    FLOWER_DAHLIAS = 1,
    FLOWER_LAVENDERS = 2,
    FLOWER_SUNFLOWERS = 3,
} FlowerType;

typedef struct Animation {
    int frame;
    int numFrames;
    int intervalMs;
    float lastDraw;
    Texture2D texture;
} Animation;

// A button with hover
typedef struct Button {
    Texture2D texture;
    bool isHovered;
    bool isClicked;
    bool isDisabled;
    Vector2 position;
} Button;

typedef struct Flower {
    FlowerType type;
    Vector2 position;
} Flower;

typedef struct HarvestHex {
    FlowerType flowerType;
    float timeUntilReadyMS;
} HarvestHex;

typedef struct Hive {
    Vector2 position; // position on garden hexgrid
    HarvestHex*** hexes;
    float nextHexStartFill;
    Animation animation;
} Hive;

typedef struct Jar {
    int value;
    int iteration;
} Jar;

struct GameState {
    enum CurrentScene currentScene;
    Vector2 playerPosition;
    KeeperDirection playerDirection;
    bool playerMoving;
    bool playerNearShop;
    Hive **hives;
    Flower **flowers;
    Jar jar;
    Jar ghostJar;
    int numHives;
    int activeHiveIndex;
    int nearestHive;
    int numFlowers;
    int money;
    BuildType currentlyBuilding;
} GameState;

#define JAR_ITERATIONS 11
#define COLUMN_COUNT 14
#define ROW_COUNT_EVEN 15
#define ROW_COUNT_UNEVEN 14
#define INITIAL_OFFSET_X 12
#define INITIAL_OFFSET_Y 12
#define MOD_OFFSET_X  4
#define UNEVEN_ROW_X_OFFSET 23
#define NEXT_FILL_TIME_IN_SECONDS 4
#define DEFAULT_TIME_UNTIL_READY 2
// CHECK(Brian): Kan hæves her for at lade kæden være større. Vi kunne også sige man kunne committe ved 3 og op efter?
#define HARVEST_CHAIN_COUNT 3

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------
static const int screenWidth = 720;
static const int screenHeight = 720;
static const Vector2 SHOP_POSITION = {660, -20};
static const int MOVEMENT_SPEED = 200;
static const Color GRASSGREEN = {51, 152, 75, 1};
static const int GARDEN_HEX_SIZE = 30;
static const int HIVE_PRICE = 10000;
static const int ZINNIAS_PRICE = 500;
static const int DAHLIAS_PRICE = 1000;
static const int LAVENDERS_PRICE = 2000;
static const int SUNFLOWERS_PRICE = 5000;
static const int STARTING_MONEY = 100000;

static Texture2D hiveSprite;
static Texture2D harvestBg;
static Texture2D menuBg;
static Texture2D gardenBg;
static Texture2D shopBg;
static Texture2D aboutBg;
static Texture2D zinniasSprite;
static Texture2D dahliasSprite;
static Texture2D lavenderSprite;
static Texture2D sunflowerSprite;
static Animation keeperSprites[7];
static Animation keyZ;
static Animation honeyGlassSprites;
static Texture2D coin;
static Texture2D hexOutline;
static Texture2D hexOutlineLight;
static Texture2D hexOrange;
static Texture2D hexPurple;
static Texture2D hexPink;
static Texture2D hexRed;
static Texture2D hexYellow;
static Camera2D gardenCamera;
static Rectangle HexGridRect = {
    .x = 73, .y = 73, .width = 575, .height = 415,
};
Music music;
Sound sellHoney;
Sound thump;

static Vector2 harvestChain[HARVEST_CHAIN_COUNT];

static Button startButton;
static Button menuButton;
static Button aboutButton;
static Button backToMenuButton;
static Button backToGardenButton;
static Button sellHoneyButton;

static Font font20;
static Font font30;

static RenderTexture2D target = { 0 };  // Render texture to render our game
static int frameCounter = 0;

// TODO: Define global variables here, recommended to make them static
struct GameState *gs = NULL;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void);      // Update and Draw one frame
                                        
static float vector2Distance(Vector2, Vector2);
static Animation initAnimation(char* fileName, int numFrames, int intervalMs);
static Animation initAnimationFromTexture(Texture2D texture, int numFrames, int intervalMs);
static void drawAnimationFrame(Animation* animation, Vector2 position);
static void unloadAnimation(Animation* animation);
static void drawHex();
static Hive* initHive(unsigned int x, unsigned int y);
static Flower* initFlower(FlowerType type, unsigned int x, unsigned int y);
static HarvestHex* initHarvestHex();
static void assignHexTile(Hive *h);
static int isTileNeighbor(Vector2 currentTile, Vector2 newTile);
static Vector2 gardenHexPositionToPixelPosition(Vector2 hexCoordinates);
static Vector2 gardenHexFromPoint(Vector2 point);
static Vector2 mouseToHexPointCoordinates();
static FlowerType chooseFlower();
static Vector2 hexDrawingCoordinates(Vector2 pos);
static void drawHarvestScene(void);
static void drawButton(Button* button);
static void updateHarvestScene(void);
static void clearHarvestChain(void);
static void drawJar(void);
static void harvestActiveChain(void);
static void printFlowerWithPos(FlowerType type, int row, int column);


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
#if !defined(_DEBUG)
    SetTraceLogLevel(LOG_NONE);         // Disable raylib trace log messages
#endif

    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "Best Bee-uddies");
    InitAudioDevice();

    // Check(Brian): Fjerner muligheden for at lukke spillet med Escape. Vil gerne bruge escape til at afbryde en kæde ved harvesting.
    SetExitKey(KEY_NULL);

    for (int i = 0; i < HARVEST_CHAIN_COUNT; i++) {
        harvestChain[i] = (Vector2){
            .x = -1,
            .y = -1,
        };
    }

    gs = malloc(sizeof(struct GameState));
    gs->currentScene = MENU;
    gs->playerPosition = (Vector2){ 100,100 };
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

    gs->hives = malloc(sizeof(Hive*) * 16);
    gs->numHives = 0;
    gs->hives[0] = initHive(2, 5);
    gs->numHives++;

    gs->flowers = malloc(sizeof(Flower*) * 200);
    gs->numFlowers = 0;
    
    // Initialize garden camera
    gardenCamera.target = gs->playerPosition;
    gardenCamera.offset = (Vector2){screenWidth/2.0f, screenHeight/2.0f};
    gardenCamera.rotation = 0.0f;
    gardenCamera.zoom = 2.0f;

#if defined(WIN32)     
        hiveSprite = LoadTexture("../../../src/resources/hive.png");
        harvestBg = LoadTexture("../../../src/resources/harvest_bg.png");
        menuBg = LoadTexture("../../../src/resources/menu_bg.png");
        gardenBg = LoadTexture("../../../src/resources/garden_bg.png");
        shopBg = LoadTexture("../../../src/resources/shop_bg.png");
        aboutBg = LoadTexture("../../../src/resources/about_bg.png");
        keyZ = initAnimation("../../../src/resources/key_z.png", 10, 200);
        coin = LoadTexture("../../../src/resources/coin.png");
        zinniasSprite = LoadTexture("../../../src/resources/zinnias.png");
        dahliasSprite = LoadTexture("../../../src/resources/dahlias.png");
        lavenderSprite = LoadTexture("../../../src/resources/lavender.png");
        sunflowerSprite = LoadTexture("../../../src/resources/sunflowers.png");
        hexOutline = LoadTexture("../../../src/resources/hex_outline.png");
        hexOutlineLight = LoadTexture("../../../src/resources/hex_outline_light.png");
        hexOrange = LoadTexture("../../../src/resources/hex_orange.png");
        hexPurple = LoadTexture("../../../src/resources/hex_purple.png");
        hexPink = LoadTexture("../../../src/resources/hex_pink.png");
        hexRed = LoadTexture("../../../src/resources/hex_red.png");
        hexYellow = LoadTexture("../../../src/resources/hex_yellow.png");
        startButton.texture = LoadTexture("../../../src/resources/start_button.png");
        menuButton.texture = LoadTexture("../../../src/resources/menu_button.png");
        aboutButton.texture = LoadTexture("../../../src/resources/about_button.png");
        backToMenuButton.texture = LoadTexture("../../../src/resources/back_to_menu_button.png");
        sellHoneyButton.texture = LoadTexture("../../../src/resources/sell_honey_button.png");
        backToGardenButton.texture = LoadTexture("../../../src/resources/back_to_garden_button.png");

        keeperSprites[BACK] = initAnimation("../../../src/resources/character_back.png", 2, 500);
        keeperSprites[FRONT] = initAnimation("../../../src/resources/character_front.png", 2, 500);
        keeperSprites[SIDE] = initAnimation("../../../src/resources/character_side.png", 2, 500);
        keeperSprites[WALK_DOWN] = initAnimation("../../../src/resources/character_walk_down.png", 6, 100);
        keeperSprites[WALK_UP] = initAnimation("../../../src/resources/character_walk_up.png", 6, 100);
        keeperSprites[WALK_LEFT] = initAnimation("../../../src/resources/character_walk_left.png", 4, 200);
        keeperSprites[WALK_RIGHT] = initAnimation("../../../src/resources/character_walk_right.png", 4, 200);
        honeyGlassSprites = initAnimation("../../../src/resources/honey_glass.png", 11, 0);
        
        // Fonts
        font20 = LoadFontEx("../../../src/resources/fonts/Jersey_10/Jersey10-Regular.ttf", 20, 0, 250);
        font30 = LoadFontEx("../../../src/resources/fonts/Jersey_10/Jersey10-Regular.ttf", 30, 0, 250);

        // Music
        music = LoadMusicStream("../../../src/resources/music/calm-acoustic-guitar-for-serene-moments.mp3");
        sellHoney = LoadSound("../../../src/resources/sounds/554841__lucish__cha_ching.mp3");
        thump = LoadSound("../../../src/resources/sounds/431204__bunnyluvvid__rabbit-thump-on-soil-edited.wav");
#else
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
        hexOutline= LoadTexture("resources/hex_outline.png");
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
        backToGardenButton.texture = LoadTexture("resources/back_to_garden_button.png");
        sellHoneyButton.texture = LoadTexture("resources/sell_honey_button.png");
        keeperSprites[BACK] = initAnimation("resources/character_back.png", 2, 500);
        keeperSprites[FRONT] = initAnimation("resources/character_front.png", 2, 500);
        keeperSprites[SIDE] = initAnimation("resources/character_side.png", 2, 500);
        keeperSprites[WALK_DOWN] = initAnimation("resources/character_walk_down.png", 6, 100);
        keeperSprites[WALK_UP] = initAnimation("resources/character_walk_up.png", 6, 100);
        keeperSprites[WALK_LEFT] = initAnimation("resources/character_walk_left.png", 4, 200);
        keeperSprites[WALK_RIGHT] = initAnimation("resources/character_walk_right.png", 4, 200);
        honeyGlassSprites = initAnimation("resources/honey_glass.png", 11, 0);
        font20 = LoadFontEx("resources/fonts/Jersey_10/Jersey10-Regular.ttf", 20, 0, 250);
        font30 = LoadFontEx("resources/fonts/Jersey_10/Jersey10-Regular.ttf", 30, 0, 250);

        // Music
        music = LoadMusicStream("resources/music/calm-acoustic-guitar-for-serene-moments.mp3");

        sellHoney = LoadSound("resources/sounds/554841__lucish__cha_ching.mp3");
        thump = LoadSound("resources/sounds/431204__bunnyluvvid__rabbit-thump-on-soil-edited.wav");

#endif
    //static Texture2D harvestBg;
    //static Texture2D keeperSprites[3];
    
    // Initialize buttons
    startButton.position = (Vector2){20, 600};
    startButton.isDisabled = false;
    menuButton.position = (Vector2){20, 600};
    menuButton.isDisabled = false;
    aboutButton.position = (Vector2){122, 600};
    aboutButton.isDisabled = false;
    backToMenuButton.position = (Vector2){549, 26};
    backToMenuButton.isDisabled = false;
    backToGardenButton.position = (Vector2){570, 640};
    backToGardenButton.isDisabled = false;
    sellHoneyButton.position = (Vector2){570, 602};
    sellHoneyButton.isDisabled = false;

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
    SetTargetFPS(60);     // Set our game frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) {   // Detect window close button
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

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//--------------------------------------------------------------------------------------------
// Module Functions Definition
//--------------------------------------------------------------------------------------------

// Return euclidean distance between two vectors
static float vector2Distance(Vector2 a, Vector2 b) {
    return sqrtf(powf((a.x - b.x), 2) + powf((a.y - b.y), 2));
}

#define SMALL_DELTA_BETWEEN_HEXES 3
static void drawHex() {
    Vector2 point = mouseToHexPointCoordinates(); // x is row, y is column
    if (point.x == -1 || point.y == -1) return;
    Vector2 pos = hexDrawingCoordinates(point);
    DrawTexture(hexOutlineLight, pos.x, pos.y, WHITE);
}

static Vector2 hexDrawingCoordinates(Vector2 pos) {
    int evenColumn = ((int)pos.y) % 2 == 0; 
    int xOffset = (evenColumn ? 4 : UNEVEN_ROW_X_OFFSET) + (SMALL_DELTA_BETWEEN_HEXES * pos.x);
    int x = hexOutline.width * pos.x + HexGridRect.x + (xOffset);
    int y = (hexOutline.height - 9) * pos.y + HexGridRect.y + (SMALL_DELTA_BETWEEN_HEXES * pos.y);
    return (Vector2){.x = x, .y = y};
}

static void drawGardenHex(Vector2 center) {
    DrawPolyLines(center, 6, 30, 210, DARKGRAY);
}

static void drawGardenHexFilled(Vector2 center, Color color) {
    DrawPoly(center, 6, 30, 210, color);
}

// Load an animation (spritesheet) into memory with a set of animation parameters.
static Animation initAnimation(char* fileName, int numFrames, int intervalMs) {
    return (Animation) {
        .texture = LoadTexture(fileName),
        .frame = 0,
        .intervalMs = intervalMs,
        .numFrames = numFrames,
        .lastDraw = 0,
    };
}

static Animation initAnimationFromTexture(Texture2D texture, int numFrames, int intervalMs) {
    return (Animation) {
        .texture = texture,
        .frame = 0,
        .intervalMs = intervalMs,
        .numFrames = numFrames,
        .lastDraw = 0,
    };
}

// Draw animation frame. Frames should step automatically.
static void drawAnimationFrame(Animation* animation, Vector2 position) {
    animation->lastDraw += GetFrameTime();    

    float textureWidth = animation->texture.width;
    float spriteWidth = textureWidth/animation->numFrames;
    float spriteHeight = animation->texture.height;
    Rectangle frameRec = {
        spriteWidth * animation->frame,
        0,
        spriteWidth,
        spriteHeight,
    };

    Rectangle destRec = {
        position.x, position.y, spriteWidth, spriteHeight
    };

    Vector2 origin = {0, 0};
    //DrawTextureRec(animation->texture, frameRec, position, WHITE);
    DrawTexturePro(animation->texture, frameRec, destRec, origin, 0, WHITE);

    if (animation->lastDraw * 1000 > animation->intervalMs) {
        animation->lastDraw = 0;
        animation->frame += 1;

        if (animation->frame >= animation->numFrames) {
            animation->frame = 0;
        }
    }
}

// Check hive/player collision
bool isHiveCollision(Vector2 playerPosition) {
    Rectangle playerRec = {
        playerPosition.x-15, playerPosition.y-20, 28, 50,
    };

    for (unsigned int i = 0; i < gs->numHives; i++) {
        Vector2 hivePixelPosition = gardenHexPositionToPixelPosition((Vector2){gs->hives[i]->position.x, gs->hives[i]->position.y});

        Rectangle hiveRec = {
            hivePixelPosition.x-16, hivePixelPosition.y-22, 30, 10,
        };

        if (CheckCollisionRecs(playerRec, hiveRec)) {
            return true;
        }
    }         
    return false;
}

// Unload animation
static void unloadAnimation(Animation* animation) {
    UnloadTexture(animation->texture);
}

// Returns the pixel coordinates for the garden hexgrid based on the hexgrid coordinates
static Vector2 gardenHexPositionToPixelPosition(Vector2 hexCoordinates) {
    int column = hexCoordinates.x;
    int row = hexCoordinates.y;
    return (Vector2){
        50 + sqrt(3) * GARDEN_HEX_SIZE * column + (row % 2) * sqrt(3) * GARDEN_HEX_SIZE / 2,
        130 + 3/2.0f * GARDEN_HEX_SIZE * row,
    };
}

// Return nearest hex coordinates from pixel point
// TODO: Definitely there is a better way to do this.
static Vector2 gardenHexFromPoint(Vector2 point) {
    Vector2 closest;
    int closestDistance = 9999;

    for (unsigned int row = 0; row < 13; row++) {
        int numColumns = 13;
        if (row % 2 == 1) numColumns = 12;
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

void drawHives(void) {
    for (unsigned int i = 0; i < gs->numHives; i++) {
        Vector2 hivePixelPosition = gardenHexPositionToPixelPosition(gs->hives[i]->position);
        hivePixelPosition.x -= 32;
        hivePixelPosition.y -= 55;
        drawAnimationFrame(&gs->hives[i]->animation, hivePixelPosition);
    }
}

void drawFlowers(void) {
    for (unsigned int i = 0; i < gs->numFlowers; i++) {
        Vector2 flowerPixelPosition = gardenHexPositionToPixelPosition(gs->flowers[i]->position);

        switch (gs->flowers[i]->type) {
            case FLOWER_ZINNIAS: {
                flowerPixelPosition.x -= 20;
                flowerPixelPosition.y -= 30;

                DrawTextureV(zinniasSprite, flowerPixelPosition, WHITE);
                break;
            }
            case FLOWER_DAHLIAS: {
                flowerPixelPosition.x -= 20;
                flowerPixelPosition.y -= 30;

                DrawTextureV(dahliasSprite, flowerPixelPosition, WHITE);
                break;
            }
            case FLOWER_LAVENDERS: {
                flowerPixelPosition.x -= 20;
                flowerPixelPosition.y -= 30;

                DrawTextureV(lavenderSprite, flowerPixelPosition, WHITE);
                break;
            }
            case FLOWER_SUNFLOWERS: {
                flowerPixelPosition.x -= 20;
                flowerPixelPosition.y -= 30;

                DrawTextureV(sunflowerSprite, flowerPixelPosition, WHITE);
                break;
            }
            case FLOWER_NONE: {
                // Relevant elsewhere
                break;
            }
        }
    }
}

void drawShopScene(void) {
    // Draw background
    DrawTexture(shopBg, 0, 0, WHITE);
    char priceString[10];

    // Draw prices
    DrawTexture(coin, 500, 170, WHITE);
    if (gs->money >= HIVE_PRICE) {
        sprintf(priceString, "%d", HIVE_PRICE);
        DrawText(priceString, 520, 170, 20, BLACK);
    } else {
        sprintf(priceString, "%d", HIVE_PRICE);
        DrawText(priceString, 520, 170, 20, RED);
    }

    DrawTexture(coin, 500, 289, WHITE);
    if(gs->money >= ZINNIAS_PRICE) {
        sprintf(priceString, "%d", ZINNIAS_PRICE);
        DrawText(priceString, 520, 289, 20, BLACK);
    } else {
        sprintf(priceString, "%d", ZINNIAS_PRICE);
        DrawText(priceString, 520, 289, 20, RED);
    }

    DrawTexture(coin, 500, 408, WHITE);
    if(gs->money >= DAHLIAS_PRICE) {
        sprintf(priceString, "%d", DAHLIAS_PRICE);
        DrawText(priceString, 520, 408, 20, BLACK);
    } else {
        sprintf(priceString, "%d", DAHLIAS_PRICE);
        DrawText(priceString, 520, 408, 20, RED);
    }

    DrawTexture(coin, 500, 527, WHITE);
    if(gs->money >= LAVENDERS_PRICE) {
        sprintf(priceString, "%d", LAVENDERS_PRICE);
        DrawText(priceString, 520, 527, 20, BLACK);
    } else {
        sprintf(priceString, "%d", LAVENDERS_PRICE);
        DrawText(priceString, 520, 527, 20, RED);
    }

    DrawTexture(coin, 500, 646, WHITE);
    if(gs->money >= SUNFLOWERS_PRICE) {
        sprintf(priceString, "%d", SUNFLOWERS_PRICE);
        DrawText(priceString, 520, 646, 20, BLACK);
    } else {
        sprintf(priceString, "%d", SUNFLOWERS_PRICE);
        DrawText(priceString, 520, 646, 20, RED);
    }
}

void drawBuildScene(void) {
    SetMouseCursor(MOUSE_CURSOR_CROSSHAIR);
    Vector2 cursor = GetMousePosition(); 

    // Draw background
    DrawTexture(gardenBg, 0, -110, WHITE);

    // Draw hexgrid
    for (unsigned int row = 0; row < 13; row++) {
        int numColumns = 13;
        if (row % 2 == 1) numColumns = 12;

        for (unsigned int column = 0; column < numColumns; column++) {
            Vector2 hexPos = gardenHexPositionToPixelPosition((Vector2){column, row});

            drawGardenHex(hexPos);
        }        
    }

    // Draw objects
    drawHives();
    drawFlowers();


    // Draw build 
    Vector2 chosenHexCoord = gardenHexFromPoint(cursor);
    Vector2 chosenHexPixelCoord = gardenHexPositionToPixelPosition(chosenHexCoord);
    drawGardenHexFilled(chosenHexPixelCoord, SKYBLUE); 

    switch (gs->currentlyBuilding) {
        case BUILD_HIVE:
            DrawTextureRec(hiveSprite, (Rectangle){0, 0, hiveSprite.width/3.0f, hiveSprite.height}, (Vector2){chosenHexPixelCoord.x - 32, chosenHexPixelCoord.y - 55}, WHITE);
            break;
        case BUILD_ZINNIAS:
            DrawTextureV(zinniasSprite, (Vector2){chosenHexPixelCoord.x - 20, chosenHexPixelCoord.y-30}, WHITE);
            break;
        case BUILD_DAHLIAS:
            DrawTextureV(dahliasSprite, (Vector2){chosenHexPixelCoord.x - 20, chosenHexPixelCoord.y-30}, WHITE);
            break;
        case BUILD_LAVENDERS:
            DrawTextureV(lavenderSprite, (Vector2){chosenHexPixelCoord.x - 20, chosenHexPixelCoord.y-30}, WHITE);
            break;
        case BUILD_SUNFLOWERS:
            DrawTextureV(sunflowerSprite, (Vector2){chosenHexPixelCoord.x - 20, chosenHexPixelCoord.y-30}, WHITE);
            break;
        case BUILD_NULL:
            // Ignore
            break;
    }
}

void updateBuildScene(void) {
    Vector2 cursor = GetMousePosition(); 

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        Vector2 chosenHexCoord = gardenHexFromPoint(cursor);
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
                gs->flowers[gs->numFlowers++] = initFlower(FLOWER_ZINNIAS, chosenHexCoord.x, chosenHexCoord.y);
                gs->money -= ZINNIAS_PRICE;
                gs->currentScene = GARDEN;
                didBuild = true;

                break;
            }
            case BUILD_DAHLIAS: {
                gs->flowers[gs->numFlowers++] = initFlower(FLOWER_DAHLIAS, chosenHexCoord.x, chosenHexCoord.y);
                gs->money -= DAHLIAS_PRICE;
                gs->currentScene = GARDEN;
                didBuild = true;

                break;
            }
            case BUILD_LAVENDERS: {
                gs->flowers[gs->numFlowers++] = initFlower(FLOWER_LAVENDERS, chosenHexCoord.x, chosenHexCoord.y);
                gs->money -= LAVENDERS_PRICE;
                gs->currentScene = GARDEN;
                didBuild = true;

                break;
            }
            case BUILD_SUNFLOWERS: {
                gs->flowers[gs->numFlowers++] = initFlower(FLOWER_SUNFLOWERS, chosenHexCoord.x, chosenHexCoord.y);
                gs->money -= SUNFLOWERS_PRICE;
                gs->currentScene = GARDEN;
                didBuild = true;

                break;
            }
            default: {
                // If item is not covered, return to garden.
                gs->currentScene = GARDEN;
            }    
        }
        if (didBuild) {
            PlaySound(thump);
        }
    }
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
            if (gs->playerMoving) { keeperSprite = WALK_UP; } else { keeperSprite = BACK; }
            break;
        case DOWN:
            if (gs->playerMoving) { keeperSprite = WALK_DOWN; } else { keeperSprite = FRONT; } 
            break;
        case LEFT:
            if (gs->playerMoving) { keeperSprite = WALK_LEFT; } else { keeperSprite = SIDE; } 
            break;
        case RIGHT:
            if (gs->playerMoving) { keeperSprite = WALK_RIGHT; } else { keeperSprite = SIDE; } 
            break;
    }

    Vector2 playerSpritePosition = {gs->playerPosition.x-24, gs->playerPosition.y-28};
    drawAnimationFrame(&keeperSprites[keeperSprite], playerSpritePosition);


    // If nearest hive is close, show key-z button.
    Vector2 nearestHivePixelPos = gardenHexPositionToPixelPosition(gs->hives[gs->nearestHive]->position);
    float distanceToNearestHive = vector2Distance(gs->playerPosition, nearestHivePixelPos);
    if (distanceToNearestHive < 100) {
        Vector2 keyZPos = nearestHivePixelPos;
        keyZPos.x -= 8;
        keyZPos.y -= 42;
        drawAnimationFrame(&keyZ, keyZPos);
    }

    // Draw key if close to shop
    if (gs->playerNearShop) {
        drawAnimationFrame(&keyZ, SHOP_POSITION);
    }

    EndMode2D();
}

void updateShopScene(void) {
    SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    Vector2 cursor = GetMousePosition(); 
    
    Rectangle exitButtonRec = {584, 27, 103, 34};
    Rectangle purchaseButtons[5];
    purchaseButtons[0] = (Rectangle){605, 161, 82, 34};
    purchaseButtons[1] = (Rectangle){605, 280, 82, 34};
    purchaseButtons[2] = (Rectangle){605, 399, 82, 34};
    purchaseButtons[3] = (Rectangle){605, 518, 82, 34};
    purchaseButtons[4] = (Rectangle){605, 637, 82, 34};


    if (CheckCollisionPointRec(cursor, exitButtonRec)) {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            gs->currentScene = GARDEN;
            return;
        }
    }

    for (unsigned int i = 0; i < 5; i++) {
        if (CheckCollisionPointRec(cursor, purchaseButtons[i])) {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);

            if (i == 0 && gs->money < HIVE_PRICE) {
                SetMouseCursor(MOUSE_CURSOR_NOT_ALLOWED);
            } else if (i == 1 && gs->money < ZINNIAS_PRICE) {
                SetMouseCursor(MOUSE_CURSOR_NOT_ALLOWED);
            } else if (i == 2 && gs->money < DAHLIAS_PRICE) {
                SetMouseCursor(MOUSE_CURSOR_NOT_ALLOWED);
            } else if (i == 3 && gs->money < LAVENDERS_PRICE) {
                SetMouseCursor(MOUSE_CURSOR_NOT_ALLOWED);
            } else if (i == 4 && gs->money < SUNFLOWERS_PRICE) {
                SetMouseCursor(MOUSE_CURSOR_NOT_ALLOWED);
            }
        

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                if (i == 0 && gs->money >= HIVE_PRICE) {
                    gs->currentScene = BUILD;
                    gs->currentlyBuilding = BUILD_HIVE;
                } else if (i == 1 && gs->money >= ZINNIAS_PRICE) {
                    gs->currentScene = BUILD;
                    gs->currentlyBuilding = BUILD_ZINNIAS;
                } else if (i == 2 && gs->money >= DAHLIAS_PRICE) {
                    gs->currentScene = BUILD;
                    gs->currentlyBuilding = BUILD_DAHLIAS;
                } else if (i == 3 && gs->money >= LAVENDERS_PRICE) {
                    gs->currentScene = BUILD;
                    gs->currentlyBuilding = BUILD_LAVENDERS;
                } else if (i == 4 && gs->money >= SUNFLOWERS_PRICE) {
                    gs->currentScene = BUILD;
                    gs->currentlyBuilding = BUILD_SUNFLOWERS;
                }
                break;
            }
        }
    }
}

void updateGardenScene(void) {
    gs->playerNearShop = false;

    // Update camera
    static Vector2 bbox = { 0.2f, 0.2f };

    int width = 720;
    int height = 720;

    Vector2 bboxWorldMin = GetScreenToWorld2D((Vector2){ (1 - bbox.x)*0.5f*width, (1 - bbox.y)*0.5f*height }, gardenCamera);
    Vector2 bboxWorldMax = GetScreenToWorld2D((Vector2){ (1 + bbox.x)*0.5f*width, (1 + bbox.y)*0.5f*height }, gardenCamera);
    gardenCamera.offset = (Vector2){ (1 - bbox.x)*0.5f*width, (1 - bbox.y)*0.5f*height };

    if (gs->playerPosition.x < bboxWorldMin.x) gardenCamera.target.x = gs->playerPosition.x;
    if (gs->playerPosition.y < bboxWorldMin.y) gardenCamera.target.y = gs->playerPosition.y;
    if (gs->playerPosition.x > bboxWorldMax.x) gardenCamera.target.x = bboxWorldMin.x + (gs->playerPosition.x - bboxWorldMax.x);
    if (gs->playerPosition.y > bboxWorldMax.y) gardenCamera.target.y = bboxWorldMin.y + (gs->playerPosition.y - bboxWorldMax.y);

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
        }

        // If close to hive, enter harvest mode
        Vector2 nearestHivePixelPos = gardenHexPositionToPixelPosition(gs->hives[gs->nearestHive]->position);
        float distanceToNearestHive = vector2Distance(gs->playerPosition, nearestHivePixelPos);
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

void drawHud(void) {
    if (gs->currentScene == MENU || gs->currentScene == ABOUT) {
        return;
    }

    const char* moneyString = TextFormat("%i", gs->money);
    Vector2 stringSize = MeasureTextEx(font30, moneyString, 30, 0);
    DrawTexture(coin, 670-stringSize.x, 690, WHITE); 
    DrawTextEx(font30, moneyString, (Vector2){690-stringSize.x, 683}, 30, 0, WHITE);
}

static void updateButton(Button* button) {
    Vector2 cursor = GetMousePosition();
    button->isHovered = false; 
    button->isClicked = false; 
    Rectangle buttonRec = {button->position.x, button->position.y, button->texture.width/2.0f, button->texture.height};

    if (CheckCollisionPointRec(cursor, buttonRec)) {
        button->isHovered = true; 

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            button->isClicked = true;
        }
    }
}

static void drawButton(Button* button) {
    int buttonWidth = button->texture.width / 2;
    Rectangle buttonRec = {0, 0, buttonWidth, button->texture.height};

    if (button->isHovered && !button->isDisabled) {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        buttonRec.x = buttonWidth;
    }

    Color color = WHITE;

    if (button->isDisabled) {
        color = (Color){ .r = GRAY.r, .g = GRAY.g, .b = GRAY.b, .a = 150 };
    }
    DrawTextureRec(button->texture, buttonRec, button->position, color);
}

void drawMenu(void) {
    // Draw background 
    DrawTexture(menuBg, 0, 0, WHITE);

    // Draw buttons
    drawButton(&startButton);
    drawButton(&aboutButton);
}

void updateMenu(void) {
    Vector2 cursor = GetMousePosition();
    startButton.isHovered = false;
    aboutButton.isHovered = false;

    if (vector2Distance(cursor, (Vector2){startButton.position.x + 50, startButton.position.y + 50}) < 50) {
        startButton.isHovered = true; 

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            gs->currentScene = GARDEN;
        }
    }

    if (vector2Distance(cursor, (Vector2){aboutButton.position.x + 50, aboutButton.position.y + 50}) < 50) {
        aboutButton.isHovered = true; 

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            gs->currentScene = ABOUT;
        }
    }
}

void drawAbout(void) {
    // Draw background 
    DrawTexture(aboutBg, 0, 0, WHITE);

    DrawTextEx(font20, "Best Bee-uddies is a small game about producing and harvesting honey.", (Vector2){50, 104}, 20, 0, BLACK);
    DrawTextEx(font20, "The game was made in one week in July 2026 for the Raylib GameJam 6.x.", (Vector2){50, 134}, 20, 0, BLACK);
    DrawTextEx(font20, "The theme was HEX + MERGE.", (Vector2){50, 164}, 20, 0, BLACK);
    DrawTextEx(font20, "Brian Ravn (xiroV): Design, Programming, Artwork", (Vector2){50, 214}, 20, 0, BLACK);
    DrawTextEx(font20, "Jonas Hinchely (Tiggles): Programming", (Vector2){50, 244}, 20, 0, BLACK);

    DrawTextEx(font20, "Music:", (Vector2) {50, 274}, 20, 0, BLACK);
    DrawTextEx(font20, "Calm Acoustic Guitar for Serene Moments by Gustavo_Alivera", (Vector2) { 50, 304 }, 20, 0, BLACK);
    DrawTextEx(font20, "-- https://freesound.org/s/761373/ -- License: Attribution 4.0", (Vector2) { 50, 316 }, 20, 0, BLACK);
    DrawTextEx(font20, "Sounds:", (Vector2) { 50, 344 }, 20, 0, BLACK);
    DrawTextEx(font20, "Cha_Ching.mp3 by Lucish_ -- https://freesound.org/s/554841/ -- License: Attribution 3.0", (Vector2) { 50, 374 }, 20, 0, BLACK);
    DrawTextEx(font20, "rabbit - thump - on - soil - edited.wav by bunnyluvvid", (Vector2) { 50, 404 }, 20, 0, BLACK);
    DrawTextEx(font20, "-- https://freesound.org/s/431204/ -- License: Creative Commons 0", (Vector2) { 50, 416 }, 20, 0, BLACK);
    DrawTextEx(font20, "Made 100% without the use of generative AI/Large Language Models.", (Vector2){50, 444}, 20, 0, BLACK);

    // Draw buttons
    drawButton(&backToMenuButton);
}

void updateAbout(void) {
    updateButton(&backToMenuButton);

    if (backToMenuButton.isClicked) {
        gs->currentScene = MENU;
    }
}


// Update and draw frame
void UpdateDrawFrame(void)
{
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
                //printf("hive idx: %i, c: %i, r: %i, ", i, h->position.y, h->position.x);
                assignHexTile(h);
                h->nextHexStartFill = NEXT_FILL_TIME_IN_SECONDS;
            }

            for (int c = 0; c < COLUMN_COUNT; c++) {
                int rowCount = c % 2 == 0 ? ROW_COUNT_EVEN : ROW_COUNT_UNEVEN;
                for (int r = 0; r < rowCount; r++) {
                    HarvestHex* hh = h->hexes[c][r];
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
        DrawTexturePro(target.texture, (Rectangle) { 0, 0, (float)target.texture.width, -(float)target.texture.height },
            (Rectangle) {
            0, 0, (float)target.texture.width, (float)target.texture.height
        }, (Vector2) { 0, 0 }, 0.0f, WHITE);

        // TODO: Draw everything that requires to be drawn at this point, maybe UI?
        drawHud();
    }
    EndDrawing();
    //----------------------------------------------------------------------------------  
}

Vector2 mouseToHexPointCoordinates() {
    Vector2 mouse = GetMousePosition();
    int heightOfHex = HexGridRect.height / COLUMN_COUNT;
    int column = (mouse.y - HexGridRect.y) / heightOfHex;
    int isEven = column % 2 == 0;
    if (!isEven) {
        if (mouse.x < HexGridRect.x + 24 || mouse.x > HexGridRect.x + HexGridRect.width - 24) {
            return (Vector2){.x = -1, .y = -1};
        }
        mouse.x -= UNEVEN_ROW_X_OFFSET;
    }
    
    int widthOfHex = HexGridRect.width / (ROW_COUNT_EVEN);
    int row = (mouse.x - HexGridRect.x) / widthOfHex; 
    return (Vector2) {
        .x = row >= 14 ? 14 : row,
        .y = column > COLUMN_COUNT - 1 ? COLUMN_COUNT - 1 : column,
    };
}

static Hive* initHive(unsigned int x, unsigned int y) {
    Hive *h = malloc(sizeof(Hive));
    h->hexes = malloc(sizeof(size_t) * COLUMN_COUNT);
    for (int c = 0; c < COLUMN_COUNT; c++) {
        h->hexes[c] = malloc(sizeof(size_t) * ROW_COUNT_EVEN);
        for (int r = 0; r < ROW_COUNT_EVEN; r++) {
            h->hexes[c][r] = initHarvestHex();
        }
    }

    h->position = (Vector2){x, y};
    h->nextHexStartFill = NEXT_FILL_TIME_IN_SECONDS;
    h->animation = initAnimationFromTexture(hiveSprite, 3, 200);

    return h;
}

static HarvestHex *initHarvestHex() {
    HarvestHex *h = malloc(sizeof(HarvestHex));
    h->flowerType = FLOWER_EMPTY;
    h->timeUntilReadyMS = -1;
    return h;
}

static Flower* initFlower(FlowerType type, unsigned int x, unsigned int y) {
    Flower *f = (Flower*) malloc(sizeof(Flower));
    f->type = type;
    f->position = (Vector2){x, y};
    return f;
}

static int isTileNeighbor(Vector2 t1, Vector2 t2) {

    if (t1.y == t2.y) {
        return t1.x - 1 == t2.x || t1.x + 1 == t2.x;
    }

    int isEven = (int)(t1.y) % 2 == 0;
    if (isEven) {
        if (fabsf(t1.y - t2.y) > 1) return false;
        return t1.x - 1 == t2.x || t1.x == t2.x;
    } else {
        if (fabsf(t1.y - t2.y) > 1) return false;
        return t1.x == t2.x || t1.x + 1 == t2.x;
    }

    return false;
}

static void printFlowerWithPos(FlowerType type, int row, int column) {
    switch (type) {
        case FLOWER_NONE:
            printf("None! ");
            break;
        case FLOWER_ZINNIAS: {
            printf("Zinnias! ");
            break;
        }
        case FLOWER_DAHLIAS: {
            printf("DAHLIAS! ");
            break;
        }
        case FLOWER_LAVENDERS: {
            printf("LAVENDERS! ");
            break;
        }
        case FLOWER_SUNFLOWERS: {
            printf("SUNFLOWERS! ");
            break;
        }
    }
    printf(", c: %i, r: %i\n", column, row);
}

static int isInHive(Hive* h, int row, int column) {
    return (column == h->position.y && row == h->position.x);
}

static FlowerType chooseFlower(Hive *h) {
    FlowerType t = FLOWER_SUNFLOWERS;
    Vector2 hivePosition = h->position;
    int column = GetRandomValue(-1 + h->position.y, 1 + h->position.y);
    if (column < 0) column = 0;
    if (column >= 13) column = 13 - 1;
    int row = 0;
    int isEven = column % 2 == 0;
    if (isEven) {
        row = GetRandomValue(h->position.x, h->position.x + 1);
        if (row >= 13) row = 13 - 1;
    } else {
        row = GetRandomValue(h->position.x - 1, h->position.x);
        if (row >= 12) row = 12 - 1;
    }
    if (row < 0) row = 0;

    /*
    if (isInHive(h, row, column)) {
        if (column <= 0) column += 1;
        else if (column >= 13 - 1) column -= 1;
        else if (row <= 0) row += 1;
        else if (isEven) {
            if (row >= 13 - 1) row -= 1;
        } else {
            if (row >= 12 - 1) row -= 1;
        }
    }
    */

    for (int i = 0; i < gs->numFlowers; i++) {
        Flower *f = gs->flowers[i];
        if (f->position.x == row && f->position.y == column) {
            //printFlowerWithPos(f->type, row, column);
            return f->type;
        }
    }

    //printf("Not found, ");
    //printFlowerWithPos(t, row, column);
    return t;
}

static void assignHexTile(Hive *h) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int c = GetRandomValue(0, COLUMN_COUNT - 1);
            int rowCount = c % 2 == 0 ? ROW_COUNT_EVEN : ROW_COUNT_UNEVEN;
            int r = GetRandomValue(0, rowCount - 1);
            if (h->hexes[c][r]->flowerType != FLOWER_EMPTY) continue;
            h->hexes[c][r]->flowerType = chooseFlower(h);
            h->hexes[c][r]->timeUntilReadyMS = DEFAULT_TIME_UNTIL_READY;
            return;   
        }
    }
    // TODO(Jonas): Check everything filled
}

static void drawHarvestScene(void) {
    if (gs->activeHiveIndex == -1) {
        printf("No activeHiveIndex set. Bailing from function.\n");
        return;
    }
    DrawTexture(harvestBg, 0, 0, WHITE);
    drawButton(&sellHoneyButton);
    drawButton(&backToGardenButton);
    
    Hive *h = gs->hives[gs->activeHiveIndex];
    for (int c = 0; c < COLUMN_COUNT; c++) {
        int rowCount = c % 2 == 0 ? ROW_COUNT_EVEN : ROW_COUNT_UNEVEN;
        for (int r = 0; r < rowCount; r++) {
            HarvestHex *hh = h->hexes[c][r];
            if (hh->flowerType != FLOWER_EMPTY) {
                Vector2 pos = hexDrawingCoordinates((Vector2){.x = r, .y = c});
                Color color = WHITE;
                if (hh->timeUntilReadyMS > 0) {
                    color.a = 255 - (hh->timeUntilReadyMS / DEFAULT_TIME_UNTIL_READY) * 255;
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
                }
                DrawTexture(*t, pos.x, pos.y, color);
            }
        } 
    }

    if (CheckCollisionPointRec(GetMousePosition(), HexGridRect)) {
        drawHex();
    }

    for (int i = 0; i < HARVEST_CHAIN_COUNT; i++) {
        Vector2 v = harvestChain[i];
        if (v.x == -1 && v.y == -1) break;
        Vector2 pos = hexDrawingCoordinates(v);
        DrawTexture(hexOutline, pos.x, pos.y, WHITE);
    }

    drawJar();
}

static void updateHarvestScene() {
    updateButton(&sellHoneyButton);
    sellHoneyButton.isDisabled = gs->jar.iteration != JAR_ITERATIONS - 1;
    updateButton(&backToGardenButton);

    if (!sellHoneyButton.isDisabled && sellHoneyButton.isClicked) {
        gs->jar.iteration = 0;
        gs->money += gs->jar.value;
        gs->jar.value = 0;
        PlaySound(sellHoney);
    }

    if (backToGardenButton.isClicked) {
        clearHarvestChain();
        gs->currentScene = GARDEN;
    }

    if (gs->activeHiveIndex == -1) {
        printf("No activeHiveIndex set. Bailing from function.\n");
        return;
    }

    Hive *h = gs->hives[gs->activeHiveIndex];

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (gs->jar.iteration == JAR_ITERATIONS - 1) return;

        int isNeighbor = false;

        Vector2 point = mouseToHexPointCoordinates();
        if (point.x == -1 && point.y == -1) return;
        HarvestHex *hex = h->hexes[(int)point.y][(int)point.x];
        if (hex->flowerType == FLOWER_EMPTY || hex->timeUntilReadyMS > 0) return;
        // Check for existing harvest-chain selection to disallow duplicates
        int duplicate = false;
        int firstFreeIdx = -1;

        for (int i = 0; i < HARVEST_CHAIN_COUNT; i++) {
            Vector2 hex = harvestChain[i];
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
                    Vector2 hex = harvestChain[firstFreeIdx - 1];
                    if (!isNeighbor) {
                        return;
                    }
                }
                harvestChain[firstFreeIdx] = point;
            }
        }

        if (firstFreeIdx == HARVEST_CHAIN_COUNT - 1) {
            harvestActiveChain();
            clearHarvestChain();
        }
    } else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsKeyDown(KEY_ESCAPE)) {
        clearHarvestChain();
    }
}

// CHECK(Brian): Not sure about values here.
static int flowerTypeToMoney(FlowerType t) {
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
            return 0;
        case FLOWER_EMPTY:
            return 0;
    }
}

static void harvestActiveChain(void) {
    int money = 0;
    float multiplier = 1.0;
    FlowerType lastFlowerType = FLOWER_EMPTY;
    for (int i = 0; i < HARVEST_CHAIN_COUNT; i++) {
        Vector2 *p = &harvestChain[i];
        HarvestHex *h = gs->hives[gs->activeHiveIndex]->hexes[(int)p->y][(int)p->x];
        FlowerType currentFlowerType = h->flowerType;
        if (currentFlowerType == lastFlowerType) {
            // CHECK(Brian): Multiplier fine?
            multiplier += 0.2;
        }
        lastFlowerType = currentFlowerType;
        money += flowerTypeToMoney(currentFlowerType) * multiplier;
        
        p->x = -1;
        p->y = -1;
        
        h->flowerType = FLOWER_NONE;
        h->timeUntilReadyMS = -1;
    }
    gs->jar.iteration += 1;
    gs->jar.value += money;
}

static void clearHarvestChain() {
    for (int i = 0; i < HARVEST_CHAIN_COUNT; i++) {
        Vector2 *hex = &harvestChain[i];
        hex->x = -1;
        hex->y = -1;
    }
}

static void drawJar() {
    float textureWidth = honeyGlassSprites.texture.width;
    float spriteWidth = textureWidth / honeyGlassSprites.numFrames;
    float spriteHeight = (float) honeyGlassSprites.texture.height;
    Rectangle frameRec = {
        spriteWidth * gs->jar.iteration,
        0,
        spriteWidth,
        spriteHeight,
    };

    Rectangle destRec = {
       0, 520, spriteWidth, spriteHeight
    };

    Vector2 origin = { 0, 0 };
    DrawTexturePro(honeyGlassSprites.texture, frameRec, destRec, origin, 0, WHITE);

    DrawTextEx(font30, TextFormat("value: %i", gs->jar.value), (Vector2){180, 650}, 30, 0, WHITE);
}

// TODOS!
// - [X] Harvesting chain
// - [X] Fill jars (?) and sell
// - [X] Leave HARVEST scene
// - [X] Flowers must affect hexes
// - [X] Music
// - [ ] Sounds
// - [ ] Fancy animation when honeyGlass is filled
// - [ ] Lower row of harvesting grid has funky behaviour
// - [ ] Game description on web page
