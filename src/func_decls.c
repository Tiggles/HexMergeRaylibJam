#ifndef FUNC_DECLS_GUARD
#define FUNC_DECLS_GUARD

#include "raylib.h"
#include "types.c"

void UpdateDrawFrame(GameState*); // Update and Draw one frame
float vector2Distance(Vector2, Vector2);
Animation initAnimation(char *fileName, int numFrames, int intervalMs);
void drawAnimationFrame(Animation *animation, Vector2 position);
void unloadAnimation(Animation *animation);
void drawHarvestHex(GameState*);
Hive *initHive(unsigned int x, unsigned int y);
Flower *initFlower(FlowerType type, unsigned int x, unsigned int y);
HarvestHex *initHarvestHex();
void assignHexTile(GameState*, Hive *h);
int isTileNeighbor(Vector2 currentTile, Vector2 newTile);
Vector2 gardenHexPositionToPixelPosition(Vector2 hexCoordinates);
Vector2 gardenHexFromPoint(Vector2 point);
void drawHarvestScene(GameState*);
Button initButton(Texture2D texture, Vector2 position);
void updateButton(Button *button);
void drawButton(Button *button);
void updateHarvestScene(GameState*);
void clearHarvestChain(GameState*);
void drawJar(GameState*);
void harvestActiveChain(GameState*);
bool isGardenHexOccupied(GameState*, Vector2 position);
Vector2 gardenHexFromPoint(Vector2 point);
void drawHives(GameState*);
void drawFlowers(GameState*);
void drawGardenHex(Vector2 center);
void drawGardenScene(GameState*);
void updateShopScene(GameState*);
void drawGardenHexFilled(Vector2, Color);
void updateGardenScene(GameState*);
bool isHiveCollision(GameState*, Vector2);
Vector2 mouseToHexPointCoordinates(GameState*);
Vector2 harvestHexDrawingCoordinates(Vector2 pos);
#endif
