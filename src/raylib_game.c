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
    HIVE,
};

enum KeeperSprite {
    BACK = 0,
    FRONT = 1,
    SIDE = 2,
    WALK_DOWN = 3,
    WALK_UP = 4,
    WALK_LEFT = 5,
    WALK_RIGHT = 6,
};

typedef enum {
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3,
} KeeperDirection;

struct GameState {
    enum CurrentScene currentScene;
    Vector2 playerPosition;
    KeeperDirection playerDirection;
    bool playerMoving;
} GameState;

typedef struct Animation {
    int frame;
    int numFrames;
    int intervalMs;
    float lastDraw;
    Texture2D texture;
} Animation;

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------
static const int screenWidth = 720;
static const int screenHeight = 720;
static const int HONEYCOMBS_COLUMNS = 16;
static const int HONEYCOMBS_ROWS = 14;
static const int MOVEMENT_SPEED = 200;
static const Color GRASSGREEN = {51, 152, 75, 1};
static float nextSceneChange = 0.0;
static Animation hive;
static Texture2D harvestBg;
static Animation keeperSprites[7];

static RenderTexture2D target = { 0 };  // Render texture to render our game
static int frameCounter = 0;

// TODO: Define global variables here, recommended to make them static
struct GameState *gs = NULL;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void);      // Update and Draw one frame
                                        
static Animation loadAnimation(char* fileName, int numFrames, int intervalMs);
static void drawAnimationFrame(Animation* animation, Vector2 position);
static void unloadAnimation(Animation* animation);


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
    gs->playerPosition = (Vector2){ 50,50 };
    gs->playerDirection = DOWN;
    gs->playerMoving = false;

    printf("%s\n", GetWorkingDirectory());

#if defined(WIN32)     
        hive = loadAnimation("../../../src/resources/hive.png", 3, 200);
        harvestBg = LoadTexture("../../../src/resources/harvest_bg.png");
        keeperSprites[BACK] = loadAnimation("../../../src/resources/character_back.png", 2, 500);
        keeperSprites[FRONT] = loadAnimation("../../../src/resources/character_front.png", 2, 500);
        keeperSprites[SIDE] = loadAnimation("../../../src/resources/character_side.png", 2, 500);
        keeperSprites[WALK_DOWN] = loadAnimation("../../../src/resources/character_walk_down.png", 6, 100);
        keeperSprites[WALK_UP] = loadAnimation("../../../src/resources/character_walk_up.png", 6, 100);
        keeperSprites[WALK_LEFT] = loadAnimation("../../../src/resources/character_walk_left.png", 6, 100);
        keeperSprites[WALK_RIGHT] = loadAnimation("../../../src/resources/character_walk_right.png", 6, 100);
#else
        hive = loadAnimation("resources/hive.png", 3, 200);
        harvestBg = LoadTexture("resources/harvest_bg.png");
        keeperSprites[BACK] = loadAnimation("resources/character_back.png", 2, 500);
        keeperSprites[FRONT] = loadAnimation("resources/character_front.png", 2, 500);
        keeperSprites[SIDE] = loadAnimation("resources/character_side.png", 2, 500);
        keeperSprites[WALK_DOWN] = loadAnimation("resources/character_walk_down.png", 6, 100);
        keeperSprites[WALK_UP] = loadAnimation("resources/character_walk_up.png", 6, 100);
        keeperSprites[WALK_LEFT] = loadAnimation("resources/character_walk_left.png", 6, 100);
        keeperSprites[WALK_RIGHT] = loadAnimation("resources/character_walk_right.png", 6, 100);
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
    unloadAnimation(&hive);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//--------------------------------------------------------------------------------------------
// Module Functions Definition
//--------------------------------------------------------------------------------------------

// Load an animation (spritesheet) into memory with a set of animation parameters.
static Animation loadAnimation(char* fileName, int numFrames, int intervalMs) {
    Animation a;

    a.texture = LoadTexture(fileName);
    a.frame = 0;
    a.intervalMs = intervalMs; 
    a.numFrames = numFrames;
    a.lastDraw = 0;

    return a;
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
        position.x, position.y, spriteWidth*2, spriteHeight*2
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

// Unload animation
static void unloadAnimation(Animation* animation) {
    UnloadTexture(animation->texture);
}

// 
//
void drawGardenScene(void) {

    if (gs->playerMoving) {
        switch (gs->playerDirection) {
            case UP:
                drawAnimationFrame(&keeperSprites[WALK_UP], gs->playerPosition);
                break;
            case DOWN:
                drawAnimationFrame(&keeperSprites[WALK_DOWN], gs->playerPosition);
                break;
            case LEFT:
                drawAnimationFrame(&keeperSprites[SIDE], gs->playerPosition);
                break;
            case RIGHT:
                drawAnimationFrame(&keeperSprites[SIDE], gs->playerPosition);
                break;
        }
    } else { // Player is not moving
        switch (gs->playerDirection) {
            case UP:
                drawAnimationFrame(&keeperSprites[BACK], gs->playerPosition);
                break;
            case DOWN:
                drawAnimationFrame(&keeperSprites[FRONT], gs->playerPosition);
                break;
            case LEFT:
            case RIGHT:
                drawAnimationFrame(&keeperSprites[SIDE], gs->playerPosition);
                break;
        }
    }

    Vector2 hivePosition = {200, 200};
    drawAnimationFrame(&hive, hivePosition);
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
        case GARDEN: {
            if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
                gs->playerPosition.y += MOVEMENT_SPEED * GetFrameTime();
                gs->playerDirection = DOWN;
                gs->playerMoving = true;
            }
            if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
                gs->playerPosition.y -= MOVEMENT_SPEED * GetFrameTime();
                gs->playerDirection = UP;
                gs->playerMoving = true;
            }
            if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
                gs->playerPosition.x -= MOVEMENT_SPEED * GetFrameTime();
                gs->playerDirection = LEFT;
                gs->playerMoving = true;
            }
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
                gs->playerPosition.x += MOVEMENT_SPEED * GetFrameTime();
                gs->playerDirection = RIGHT;
                gs->playerMoving = true;
            }
        }
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
            case HIVE: {
                DrawTexture(harvestBg, 0, 0, WHITE);
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
