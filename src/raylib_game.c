/*******************************************************************************************
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
    BEEKEEPING,
    HIVE,
};

enum KeeperSprite {
    BACK = 0,
    FRONT = 1,
    SIDE = 2,
};

struct GameState {
    enum CurrentScene currentScene;
    Vector2 playerPosition;
} GameState;

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------
static const int screenWidth = 720;
static const int screenHeight = 720;
static const int HONEYCOMBS_COLUMNS = 16;
static const int HONEYCOMBS_ROWS = 14;
static const int MOVEMENT_SPEED = 200;
static float nextSceneChange = 0.0;
static Texture2D hive;
static Texture2D harvestBg;
static Texture2D keeperSprites[3];



static RenderTexture2D target = { 0 };  // Render texture to render our game
static int frameCounter = 0;

// TODO: Define global variables here, recommended to make them static
struct GameState *gs = NULL;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void);      // Update and Draw one frame

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

    printf("%s\n", GetWorkingDirectory());

#if defined(WIN32)     
        hive = LoadTexture("../../../src/resources/hive.png");
        harvestBg = LoadTexture("../../../src/resources/harvest_bg.png");
        keeperSprites[BACK] = LoadTexture("../../../src/resources/character_back.png");
        keeperSprites[FRONT] = LoadTexture("../../../src/resources/character_front.png");
        keeperSprites[SIDE] = LoadTexture("../../../src/resources/character_side.png");
#else
        hive = LoadTexture("resources/hive.png");
        harvestBg = LoadTexture("resources/harvest_bg.png");
        keeperSprites[BACK] = LoadTexture("resources/character_back.png");
        keeperSprites[FRONT] = LoadTexture("resources/character_front.png");
        keeperSprites[SIDE] = LoadTexture("resources/character_side.png");
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

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//--------------------------------------------------------------------------------------------
// Module Functions Definition
//--------------------------------------------------------------------------------------------
// Update and draw frame
void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    if (IsKeyDown(KEY_TAB)) {
        if (nextSceneChange < 0) {
            gs->currentScene = (gs->currentScene + 1) % 3;
            nextSceneChange = 0.2;
        }
        nextSceneChange -= GetFrameTime();
    }


    switch (gs->currentScene) {
        case BEEKEEPING: {
            if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
                gs->playerPosition.y += MOVEMENT_SPEED * GetFrameTime();
            }
            if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
                gs->playerPosition.y -= MOVEMENT_SPEED * GetFrameTime();

            }
            if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
                gs->playerPosition.x -= MOVEMENT_SPEED * GetFrameTime();

            }
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
                gs->playerPosition.x += MOVEMENT_SPEED * GetFrameTime();

            }
        }
    }

    frameCounter++;
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    // Render game screen to a texture, 
    // it could be useful for scaling or further shader postprocessing
    BeginTextureMode(target);
    {
        ClearBackground(RAYWHITE);

        switch (gs->currentScene) {
            case MENU: {
                // TODO
                DrawText("This is supposed to be the menu. Sorry!", 50, 50, 28, BLACK);
                DrawText("Press <TAB> to cycle scenes (that don't do anything, really)", 50, 150, 18, BLACK);
                break;
            }
            case BEEKEEPING: {
                DrawTexture(keeperSprites[FRONT], gs->playerPosition.x, gs->playerPosition.y, WHITE);
                DrawTexture(hive, 200, 200, WHITE);
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
        ClearBackground(RAYWHITE);

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
