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
};

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

typedef struct Animation {
    int frame;
    int numFrames;
    int intervalMs;
    float lastDraw;
    Texture2D texture;
} Animation;

typedef struct Hive {
    Vector2 position;
    float** hexes;
} Hive;

struct GameState {
    enum CurrentScene currentScene;
    Vector2 playerPosition;
    KeeperDirection playerDirection;
    bool playerMoving;
    bool playerNearShop;
    Hive **hives;
    int numHives;
} GameState;

#define MAX_COLUMNS 14
#define MAX_ROWS 14

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------
static const int screenWidth = 720;
static const int screenHeight = 720;
static const Vector2 SHOP_POSITION = {660, -20};
//static const int HONEYCOMBS_COLUMNS = 16;
//static const int HONEYCOMBS_ROWS = 14;
static const int MOVEMENT_SPEED = 200;
static const Color GRASSGREEN = {51, 152, 75, 1};
static float nextSceneChange = 0.0;
static Animation hiveSprite;
static Texture2D harvestBg;
static Texture2D gardenBg;
static Texture2D shopBg;
static Animation keeperSprites[7];
static Animation keyZ;
static Camera2D gardenCamera;
static Rectangle HexGridRect = {
    .x = 73, .y = 73, .width = 575, .height = 415,
};

static RenderTexture2D target = { 0 };  // Render texture to render our game
static int frameCounter = 0;

// TODO: Define global variables here, recommended to make them static
struct GameState *gs = NULL;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void);      // Update and Draw one frame
                                        
static float vector2Distance(Vector2, Vector2);
static Animation loadAnimation(char* fileName, int numFrames, int intervalMs);
static void drawAnimationFrame(Animation* animation, Vector2 position);
static void unloadAnimation(Animation* animation);
static void drawHex(Vector2 center);
static Hive* initHive();
static void hiveDebugInfo(Hive* hive);

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

    gs = malloc(sizeof(struct GameState));
    gs->currentScene = MENU;
    gs->playerPosition = (Vector2){ 100,100 };
    gs->playerDirection = DOWN;
    gs->playerMoving = false;
    gs->playerNearShop = false;

    gs->hives = malloc(sizeof(Hive*) * 16);
    gs->hives[0] = initHive();
    gs->numHives = 0;
    
    printf("%s\n", GetWorkingDirectory());


    // Initialize garden camera
    gardenCamera.target = gs->playerPosition;
    gardenCamera.offset = (Vector2){screenWidth/2.0f, screenHeight/2.0f};
    gardenCamera.rotation = 0.0f;
    gardenCamera.zoom = 2.0f;

#if defined(WIN32)     
        hiveSprite = loadAnimation("../../../src/resources/hive.png", 3, 200);
        harvestBg = LoadTexture("../../../src/resources/harvest_bg.png");
        gardenBg = LoadTexture("../../../src/resources/garden_bg.png");
        shopBg = LoadTexture("../../../src/resources/shop_bg.png");
        keyZ = loadAnimation("../../../src/resources/key_z.png", 10, 200);
        keeperSprites[BACK] = loadAnimation("../../../src/resources/character_back.png", 2, 500);
        keeperSprites[FRONT] = loadAnimation("../../../src/resources/character_front.png", 2, 500);
        keeperSprites[SIDE] = loadAnimation("../../../src/resources/character_side.png", 2, 500);
        keeperSprites[WALK_DOWN] = loadAnimation("../../../src/resources/character_walk_down.png", 6, 100);
        keeperSprites[WALK_UP] = loadAnimation("../../../src/resources/character_walk_up.png", 6, 100);
        keeperSprites[WALK_LEFT] = loadAnimation("../../../src/resources/character_walk_left.png", 4, 200);
        keeperSprites[WALK_RIGHT] = loadAnimation("../../../src/resources/character_walk_right.png", 4, 200);
#else
        hiveSprite = loadAnimation("resources/hive.png", 3, 200);
        harvestBg = LoadTexture("resources/harvest_bg.png");
        gardenBg = LoadTexture("resources/garden_bg.png");
        shopBg = LoadTexture("resources/shop_bg.png");
        keyZ = loadAnimation("resources/key_z.png", 10, 200);
        keeperSprites[BACK] = loadAnimation("resources/character_back.png", 2, 500);
        keeperSprites[FRONT] = loadAnimation("resources/character_front.png", 2, 500);
        keeperSprites[SIDE] = loadAnimation("resources/character_side.png", 2, 500);
        keeperSprites[WALK_DOWN] = loadAnimation("resources/character_walk_down.png", 6, 100);
        keeperSprites[WALK_UP] = loadAnimation("resources/character_walk_up.png", 6, 100);
        keeperSprites[WALK_LEFT] = loadAnimation("resources/character_walk_left.png", 4, 200);
        keeperSprites[WALK_RIGHT] = loadAnimation("resources/character_walk_right.png", 4, 200);
#endif
    //static Texture2D harvestBg;
    //static Texture2D keeperSprites[3];

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
    while (!WindowShouldClose())    // Detect window close button
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(target);

    // TODO: Unload all loaded resources at this point
    unloadAnimation(&hiveSprite);

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

static void drawHex(Vector2 center) {
    DrawPoly(center, 6, 21, 210, RED);               // Draw a regular polygon (Vector version)
}

// Load an animation (spritesheet) into memory with a set of animation parameters.
static Animation loadAnimation(char* fileName, int numFrames, int intervalMs) {
    return (Animation) {
        .texture = LoadTexture(fileName),
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
        Rectangle hiveRec = {
            gs->hives[i]->position.x-16, gs->hives[i]->position.y-5, 30, 10,
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



void drawShopScene(void) {
    // Draw background
    DrawTexture(shopBg, 0, 0, WHITE);
}
// 
//
void drawGardenScene(void) {
    KeeperSprite keeperSprite = FRONT;

    BeginMode2D(gardenCamera); 

    // Draw background
    DrawTexture(gardenBg, 0, -120, WHITE);

    // Draw hives
    for (unsigned int i = 0; i < gs->numHives; i++) {
        Vector2 hiveSpritePosition = {gs->hives[i]->position.x-33, gs->hives[i]->position.y-35};
        drawAnimationFrame(&hiveSprite, hiveSpritePosition);
    }

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

    // Draw key if close to hive
    // Note: We do this in a different loop, because we want it to render after player.
    for (unsigned int i = 0; i < gs->numHives; i++) {
        float distance = vector2Distance(gs->playerPosition, gs->hives[i]->position);
        if (distance < 100) {
            Vector2 keyZPos = {gs->hives[i]->position.x-8, gs->hives[i]->position.y-35};
            drawAnimationFrame(&keyZ, keyZPos);
        }
    }
    
    // Draw key if close to shop
    if (gs->playerNearShop) {
        drawAnimationFrame(&keyZ, SHOP_POSITION);
    }

    EndMode2D();
}


void updateShopScene(void) {
    
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
    }
}


// Update and draw frame
void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    gs->playerMoving = false;


    if (IsKeyDown(KEY_TAB)) {
        if (nextSceneChange < 0) {
            gs->currentScene = (gs->currentScene + 1) % 3;
            nextSceneChange = 0.2;
        }
        nextSceneChange -= GetFrameTime();
    }


    switch (gs->currentScene) {
        case GARDEN:
            updateGardenScene();
            break;
        case SHOP:
            updateShopScene();
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
                // TODO
                DrawText("This is supposed to be the menu. Sorry!", 50, 50, 28, BLACK);
                DrawText("Press <TAB> to cycle scenes (that don't do anything, really)", 50, 150, 18, BLACK);
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
            case HARVEST: {
                DrawTexture(harvestBg, 0, 0, WHITE);
                drawHex(GetMousePosition());
                hiveDebugInfo(gs->hives[0]);
                DrawRectangleLinesEx(HexGridRect,1, RED);
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
    }
    EndDrawing();
    //----------------------------------------------------------------------------------  
}

#define INITIAL_OFFSET_X 12
#define INITIAL_OFFSET_Y 12
#define MOD_OFFSET_X  4

static void hiveDebugInfo(Hive* hive) {
    for (int c = 0; c < MAX_COLUMNS; c++) {
        for (int r = 0; r < MAX_ROWS; r++) {
            int posX = INITIAL_OFFSET_X + 90 * c + c % 2 == 0 ? 0 : MOD_OFFSET_X;
            int posY = INITIAL_OFFSET_Y + 22 * r;
            DrawText(TextFormat("%f", hive->hexes[c][r]), posX, posY, 4, WHITE);
        }
    }
}

static Hive* initHive() {
    Hive *h = malloc(sizeof(Hive));
    h->hexes = malloc(sizeof(size_t) * MAX_COLUMNS);
    for (int c = 0; c < MAX_COLUMNS; c++) {
        h->hexes[c] = malloc(sizeof(size_t) * MAX_ROWS);
        for (int r = 0; r < MAX_ROWS; r++) {
            h->hexes[c][r] = 0;
        }
    }

    h->position = (Vector2){200, 200};

    gs->numHives++;
    return h;
}
