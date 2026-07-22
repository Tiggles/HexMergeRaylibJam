#ifndef FUNC_DECLS_GUARD
#define FUNC_DECLS_GUARD

#include "raylib.h"
#include "types.c"

void UpdateDrawFrame(void); // Update and Draw one frame
float vector2Distance(Vector2, Vector2);
Animation initAnimation(char *fileName, int numFrames, int intervalMs);
void drawAnimationFrame(Animation *animation, Vector2 position);
void unloadAnimation(Animation *animation);
void drawHarvestHex();
Hive *initHive(unsigned int x, unsigned int y);
Flower *initFlower(FlowerType type, unsigned int x, unsigned int y);
HarvestHex *initHarvestHex();
void assignHexTile(Hive *h);
int isTileNeighbor(Vector2 currentTile, Vector2 newTile);
Vector2 gardenHexPositionToPixelPosition(Vector2 hexCoordinates);
Vector2 gardenHexFromPoint(Vector2 point);
void drawHarvestScene(void);
Button initButton(Texture2D texture, Vector2 position);
void updateButton(Button *button);
void drawButton(Button *button);
void updateHarvestScene(void);
void clearHarvestChain(void);
void drawJar(void);
void harvestActiveChain(void);
bool isGardenHexOccupied(Vector2 position);
Vector2 gardenHexFromPoint(Vector2 point);
void drawHives(void);
void drawFlowers(void);
void drawGardenHex(Vector2 center);
void drawGardenScene(void);
void updateShopScene(void);
void drawGardenHexFilled(Vector2, Color);
void updateGardenScene(void);
bool isHiveCollision(Vector2);
Vector2 mouseToHexPointCoordinates(void);
Vector2 harvestHexDrawingCoordinates(Vector2 pos);
#endif
