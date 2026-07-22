#ifndef TYPES_GUARD
#define TYPES_GUARD

#include "raylib.h"

typedef enum {
    SCREEN_LOGO = 0,
    SCREEN_TITLE,
    SCREEN_GAMEPLAY,
    SCREEN_ENDING
} GameScreen;

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

#define HARVEST_CHAIN_COUNT 3
struct GameState {
    Vector2 harvestChain[HARVEST_CHAIN_COUNT];
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
#endif
