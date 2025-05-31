#include "raylib.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define PI 3.14159265358979323846f
#define TILE_SIZE 32
#define MAP_WIDTH 50
#define MAP_HEIGHT 50
#define SELL_TILE_X 3
#define SELL_TILE_Y 2
#define PLAYER_SPEED 200
#define PLAYER_WIDTH 64 
#define PLAYER_HEIGHT 64   
#define SHOP_WIDTH 3 
#define SHOP_HEIGHT 3 


typedef struct Player {
    Vector2 position;
} Player;

typedef enum {
    GAME_TITLE,
    GAME_START,
    GAME_RUNNING
} GameState;

typedef enum {
    TILE_NORMAL,
    TILE_PLANTING,
    TILE_WATER,
    TILE_SAND,
    TILE_GRASS,
    TILE_CORRUPTED,
} TileType;

typedef enum {
    GAMEOVER_DAYS_OVER,
    GAMEOVER_KILLSWITCH,
    GAMEOVER_NO_RESOURCES,
    GAMEOVER_ALL_CORRUPTED  
} GameOverReason;

typedef struct {
    TileType type;
    bool hasSeed;
    int growthStage;  
    bool isWatered;
    float growthTimer; 
} Tile;

typedef enum {
    TOOL_SEED,
    TOOL_HOE,
    TOOL_WATERING_CAN,
    TOOL_PURIFIER,
} ToolType;

// Texturi si Variabile Globale
Texture2D backgroundTexture;
Texture2D playerTexture;
Texture2D texNormal;
Texture2D texPlanting;
Texture2D texWater;
Texture2D texSand;
Texture2D texGrass;
Texture2D texShop;
Texture2D texCropStage1;  
Texture2D texCropStage2;  
Texture2D texCropStage3;  
Texture2D texWateredTile; 
Texture2D texCorrupted;
Texture2D playerTextures[4][2]; 

bool hasBoughtPurifier = false;
bool dayTransition = false;
Vector2 playerStartPos = { 5 * TILE_SIZE, 5 * TILE_SIZE };
int totalEarned = 0;
float animTimer = 0.0f;
int currentFrame = 0;
int currentDirection = 0; 
bool isMoving = false;
int purifierCharges = 1;  
int purifierPrice = 1500;
bool scoreSaved = false;
char playerName[32] = "";
int nameCharIndex = 0;
float dayTimer = 0.0f;
bool isDay = true;
int currentDay = 1;
const float CYCLE_DURATION = 180.0f; // timp per zi
ToolType currentTool = TOOL_SEED;
Tile map[MAP_HEIGHT][MAP_WIDTH];
int harvestedCrops = 0;
int balance = 0;
int wateringCanUses = 10;
bool shopMenuOpen = false;
int seedCount = 5;
bool gameEnded = false;
GameState currentGameState = GAME_TITLE;
GameOverReason gameOverReason = GAMEOVER_DAYS_OVER;

// Declaratii functii
void DrawToolbar();
void DrawMap();
void DrawPlayer(Player player);
void PlantSeed(Player player);
void Harvest(Player player);
void TryOpenShopMenu(Player player);
void DrawShopMenu();
void HandleShopMenuInput();
void DrawStartScreen();
void GetTopFarmer();
void GenerateOvalGrass();
void DrawMapBackground(Camera2D camera);
void PurifyCorruption(Player Player);
void DrawDayTransitionScreen();
void SpreadCorruption();

int main() {
    InitWindow(800, 600, "Farming Simulator");
    SetTargetFPS(60);
    SetExitKey(0);
    srand(time(NULL));
    
    //initializare texturi
    texNormal = LoadTexture("tilesnormal.png");
    backgroundTexture = LoadTexture("spacebackground.png");
texPlanting = LoadTexture("tilesplanting.png");
texWater = LoadTexture("tileswater.png");
texSand = LoadTexture("tilessand.png");
texGrass = LoadTexture("tilesgrass.png");
texShop = LoadTexture("tilesshop.png");
texCorrupted = LoadTexture("tilescorrupted.png");
texCropStage1 = LoadTexture("cropstage1.png");
texCropStage2 = LoadTexture("cropstage2.png");
texCropStage3 = LoadTexture("cropstage3.png");
texWateredTile = LoadTexture("tileswatered.png");
playerTexture = LoadTexture("player.png");
playerTextures[0][0] = LoadTexture("playerdown1.png");
playerTextures[0][1] = LoadTexture("playerdown2.png");
playerTextures[1][0] = LoadTexture("playerleft1.png");
playerTextures[1][1] = LoadTexture("playerleft2.png");
playerTextures[2][0] = LoadTexture("playerright1.png");
playerTextures[2][1] = LoadTexture("playerright2.png");
playerTextures[3][0] = LoadTexture("playerup1.png");
playerTextures[3][1] = LoadTexture("playerup2.png");


    // Initializare tileuri
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            map[y][x].type = TILE_NORMAL;
            map[y][x].hasSeed = false;
            map[y][x].growthTimer = 0.0f;
            map[y][x].isWatered = false;
        }
    }

    // Apa
    for (int y = MAP_HEIGHT - 5; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < 5; x++) {
            map[y][x].type = TILE_WATER;
        }
    }

    for (int y = 0; y < 5; y++) {
        for (int x = MAP_WIDTH - 5; x < MAP_WIDTH; x++) {
            map[y][x].type = TILE_WATER;
        }
    }

    for (int y = MAP_HEIGHT - 5; y < MAP_HEIGHT; y++) {
        for (int x = MAP_WIDTH - 5; x < MAP_WIDTH; x++) {
            map[y][x].type = TILE_WATER;
        }
    }

    int centerX = MAP_WIDTH / 2;
    int centerY = MAP_HEIGHT / 2;
    for (int y = -5; y <= 5; y++) {
        for (int x = -5; x <= 5; x++) {
            if (centerY + y >= 0 && centerY + y < MAP_HEIGHT && centerX + x >= 0 && centerX + x < MAP_WIDTH) {
                if (x * x + y * y <= 25) {
                    map[centerY + y][centerX + x].type = TILE_WATER;
                }
            }
        }
    }

    // Nisip
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (map[y][x].type == TILE_WATER) {
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        int nx = x + dx;
                        int ny = y + dy;
                        if (nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT) {
                            if (map[ny][nx].type == TILE_NORMAL) {
                                map[ny][nx].type = TILE_SAND;
                            }
                        }
                    }
                }
            }
        }
    }
    
    //grass
int ovalCount = 5;
int ovalDistance = 12; 
int ovalSizeX = 8;     
int ovalSizeY = 5;     

int lakeCenterX = MAP_WIDTH / 2;
int lakeCenterY = MAP_HEIGHT / 2;
int lakeRadius = 5;

// Generare ovale de iarba in jurul lacului de apa
for (int i = 0; i < ovalCount; i++) {
    float angle = 2 * PI * i / ovalCount;
    int ovalX = lakeCenterX + (int)((lakeRadius + ovalDistance) * cos(angle));
    int ovalY = lakeCenterY + (int)((lakeRadius + ovalDistance) * sin(angle));
    
    GenerateOvalGrass(ovalX, ovalY, ovalSizeX, ovalSizeY);
}
//Generare Coruptie
for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
        if (map[y][x].type == TILE_GRASS) {
            if (rand() % 100 < 80) {
                map[y][x].type = TILE_CORRUPTED;
            }
        }
    }
}

    Player player = { .position = { 5 * TILE_SIZE, 5 * TILE_SIZE } };
    Camera2D camera = { 0 };
    camera.zoom = 1.35f;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        switch (currentGameState) {
            case GAME_TITLE:
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("SPACE FARM", GetScreenWidth()/2 - MeasureText("SPACE FARM", 50)/2, 200, 50, WHITE);
        DrawStartScreen();
        EndDrawing();

        if (IsKeyPressed(KEY_ENTER)) {
            currentGameState = GAME_START;
        }
        break;
            case GAME_START:
            BeginDrawing();
    ClearBackground(BLACK);
    
    DrawText("Survive For 5 Days On This Planet, Carefully Managing Your Resources",30,50,20,WHITE);
    DrawText("Use 1,2,3,4 For Changing Between SEEDS, HOE, WATER And PURIFIER",30,100,20,WHITE); 
    DrawText("Use SPACE To Plant, Hoe, Water and Purify The Grass",30,150,20,WHITE);
    DrawText("Use E to interact with CROPS and the ROCKET SHOP",30,200,20,WHITE);
    
    DrawText("Enter Your Name:", 300, 400, 20, WHITE);
    DrawRectangle(280, 425, 220, 30, WHITE);
    DrawText(playerName, 285, 430, 20, GREEN);
    
    DrawText("Beware of the ALIEN CORRUPTION",30,250,20,RED);

    int key = GetCharPressed();
    if (key > 0 && nameCharIndex < 31) {
        playerName[nameCharIndex++] = (char)key;
        playerName[nameCharIndex] = '\0';
    }

    if (IsKeyPressed(KEY_BACKSPACE) && nameCharIndex > 0) {
        nameCharIndex--;
        playerName[nameCharIndex] = '\0';
    }

    if (IsKeyPressed(KEY_ENTER) && nameCharIndex > 0) {
        currentGameState = GAME_RUNNING;
    }

    EndDrawing();
    break;
            case GAME_RUNNING:
    if (dayTransition) {
    if (IsKeyPressed(KEY_ENTER)) {
        dayTransition = false;
        player.position = playerStartPos; 
        SpreadCorruption(); 
    }
} else {
    // Update Timp
    dayTimer += deltaTime;
    if (dayTimer >= CYCLE_DURATION) {
        dayTransition = true;
        dayTimer = 0;
        currentDay++;
    }
}


                if (!gameEnded) {
                    if (!shopMenuOpen) {
                        Vector2 nextPos = player.position;
                        
                         if (currentDay > 5) {
        gameEnded = true;
        gameOverReason = GAMEOVER_DAYS_OVER;
    }
    else if (IsKeyPressed(KEY_K)) {
        gameEnded = true;
        gameOverReason = GAMEOVER_KILLSWITCH;
    }
    else if (hasBoughtPurifier && seedCount <= 0 && balance < 50) {
    bool hasHarvestableCrops = false;
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (map[y][x].hasSeed && map[y][x].growthStage >= 3) {
                hasHarvestableCrops = true;
                break;
            }
        }
        if (hasHarvestableCrops) break;
    }
    
    bool hasGrowingCrops = false;
    if (!hasHarvestableCrops) {
        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                if (map[y][x].hasSeed && map[y][x].growthStage < 3) {
                    hasGrowingCrops = true;
                    break;
                }
            }
            if (hasGrowingCrops) break;
        }
    }
    
    if (!hasHarvestableCrops && !hasGrowingCrops) {
        gameEnded = true;
        gameOverReason = GAMEOVER_NO_RESOURCES;
    }
}else if (hasBoughtPurifier)
{
    if(seedCount <= 0 && balance < 50) {
    bool hasHarvestableCrops = false;
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (map[y][x].hasSeed && map[y][x].growthStage >= 3) {
                hasHarvestableCrops = true;
                break;
            }
        }
        if (hasHarvestableCrops) break;
    }
    
    bool hasGrowingCrops = false;
    if (!hasHarvestableCrops) {
        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                if (map[y][x].hasSeed && map[y][x].growthStage < 3) {
                    hasGrowingCrops = true;
                    break;
                }
            }
            if (hasGrowingCrops) break;
        }
    }
    
    if (!hasHarvestableCrops && !hasGrowingCrops) {
        gameEnded = true;
        gameOverReason = GAMEOVER_NO_RESOURCES;
    }
}
else
    hasBoughtPurifier=false;
}
    else {
        bool grassRemaining = false;
        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                if (map[y][x].type == TILE_GRASS) {
                    grassRemaining = true;
                    break;
                }
            }
            if (grassRemaining) break;
        }
        
        if (!grassRemaining) {
            gameEnded = true;
            gameOverReason = GAMEOVER_ALL_CORRUPTED;
        }
    }


                        if (IsKeyPressed(KEY_ONE)) currentTool = TOOL_SEED;
                        if (IsKeyPressed(KEY_TWO)) currentTool = TOOL_HOE;
                        if (IsKeyPressed(KEY_THREE)) currentTool = TOOL_WATERING_CAN;
                        if (IsKeyPressed(KEY_FOUR)) currentTool = TOOL_PURIFIER;

                        isMoving = false; 

if (IsKeyDown(KEY_UP)) {
    nextPos.y -= PLAYER_SPEED * deltaTime;
    currentDirection = 3; // Up
    isMoving = true;
}
if (IsKeyDown(KEY_DOWN)) {
    nextPos.y += PLAYER_SPEED * deltaTime;
    currentDirection = 0; // Down
    isMoving = true;
}
if (IsKeyDown(KEY_LEFT)) {
    nextPos.x -= PLAYER_SPEED * deltaTime;
    currentDirection = 1; // Left
    isMoving = true;
}
if (IsKeyDown(KEY_RIGHT)) {
    nextPos.x += PLAYER_SPEED * deltaTime;
    currentDirection = 2; // Right
    isMoving = true;
}

//animatie mers
float animSpeed = 0.2f;
animTimer += deltaTime;

if (isMoving) {
    if (animTimer >= animSpeed) {
        animTimer = 0.0f;
        currentFrame = (currentFrame + 1) % 2; 
    }
} else {
    currentFrame = 0;
}

                        if (nextPos.x < 0) nextPos.x = 0;
                        if (nextPos.y < 0) nextPos.y = 0;
                        if (nextPos.x >= MAP_WIDTH * TILE_SIZE) nextPos.x = MAP_WIDTH * TILE_SIZE - 1;
                        if (nextPos.y >= MAP_HEIGHT * TILE_SIZE) nextPos.y = MAP_HEIGHT * TILE_SIZE - 1;

                        int tx = (int)(nextPos.x / TILE_SIZE);
                        int ty = (int)(nextPos.y / TILE_SIZE);

                        if (map[ty][tx].type != TILE_WATER) {
                            player.position = nextPos;
                        }

                        if (IsKeyPressed(KEY_SPACE)) PlantSeed(player);
                        if (IsKeyPressed(KEY_E)) {
                            Harvest(player);
                            TryOpenShopMenu(player);
                        }
                    } else {
                        HandleShopMenuInput();
                    }

                    // Crop growth
                    for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
        if (map[y][x].hasSeed) {
            float growthRate = map[y][x].isWatered ? 0.002f : 0.001f;
            map[y][x].growthTimer += growthRate;
            
            // Update growth
            if (map[y][x].growthTimer > 0.33f && map[y][x].growthStage < 1) {
                map[y][x].growthStage = 1;
            }
            else if (map[y][x].growthTimer > 0.66f && map[y][x].growthStage < 2) {
                map[y][x].growthStage = 2;
            }
            else if (map[y][x].growthTimer >= 1.0f && map[y][x].growthStage < 3) {
                map[y][x].growthStage = 3;
            }
        }
    }
}

                    // Refill apa
                    int x = (int)(player.position.x / TILE_SIZE);
                    int y = (int)(player.position.y / TILE_SIZE);
                    if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) {
                        if (map[y][x].type == TILE_SAND) {
                            wateringCanUses = 10;
                        }
                    }

                    camera.target = (Vector2){ player.position.x + TILE_SIZE / 2, player.position.y + TILE_SIZE / 2 };
                    camera.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
                }

                // Draw section
                BeginDrawing();
                ClearBackground(BLACK);

                if (gameEnded) {
                      if (!scoreSaved) {
        FILE *file = fopen("scores.txt", "a");
        if (file) {
            fprintf(file, "Player: %s | Earnings: $%d | Day: %d\n", playerName, totalEarned, currentDay);
            fclose(file);
        }
        scoreSaved = true;  // scor salvat
    }

    // Draw "Game Over"
    const char* endMsg = "Game Over";
    const char* earningsMsg = TextFormat("Total Money Earned: $%d", totalEarned);
    const char* prompt = "Press ENTER to Exit";
    
    // motive game over
    const char* reasonMsg = "";
    switch (gameOverReason) {
        case GAMEOVER_DAYS_OVER:
            reasonMsg = "Your 5-day mission is complete!";
            break;
        case GAMEOVER_KILLSWITCH:
            reasonMsg = "Mission aborted by command.";
            break;
        case GAMEOVER_NO_RESOURCES:
            reasonMsg = "You ran out of seeds and money!";
            break;
        case GAMEOVER_ALL_CORRUPTED:
            reasonMsg = "The corruption has consumed all land!";
            break;
    }

    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    DrawText(endMsg, screenW/2 - MeasureText(endMsg, 40)/2, screenH/2 - 80, 40, RED);
    DrawText(reasonMsg, screenW/2 - MeasureText(reasonMsg, 30)/2, screenH/2 - 40, 30, WHITE);
    DrawText(earningsMsg, screenW/2 - MeasureText(earningsMsg, 30)/2, screenH/2, 30, DARKGREEN);
    DrawText(TextFormat("Day: %d", currentDay), screenW/2 - MeasureText("Day: X", 25)/2, screenH/2 + 40, 25, WHITE);
    DrawText(prompt, screenW/2 - MeasureText(prompt, 20)/2, screenH/2 + 80, 20, GRAY);

    if (IsKeyPressed(KEY_ENTER)) {
        CloseWindow();
        return 0;
    }
                } else {
                    BeginDrawing();
                    ClearBackground(BLACK);
                    if(dayTransition){
                        DrawDayTransitionScreen();
                    }
                    else
                    {
                    BeginMode2D(camera);
                    DrawMapBackground(camera);
                    DrawMap();
                    DrawPlayer(player);
                    EndMode2D();

                    DrawToolbar();
                    DrawText(TextFormat("Day: %d", currentDay), 10, 90, 20, WHITE);
                    DrawText(TextFormat("Harvested: %d", harvestedCrops), 10, 10, 20, WHITE);
                    DrawText(TextFormat("Balance: $%d", balance), 10, 30, 20, WHITE);
                    DrawText(TextFormat("Seeds: %d", seedCount), 10, 50, 20, WHITE);
                    
                    if (shopMenuOpen) DrawShopMenu();
                    }
                }

                EndDrawing();
                break;
        }
    }

    CloseWindow();
    return 0;

}

void DrawPlayer(Player player) {
    Rectangle destRect = {
        player.position.x - (PLAYER_WIDTH - TILE_SIZE)/2,  // Centrare pe tile
        player.position.y - (PLAYER_HEIGHT - TILE_SIZE),   // aliniere pe tile
        PLAYER_WIDTH,
        PLAYER_HEIGHT
    };
    
     Texture2D toDraw;
    if (isMoving) {
        toDraw = playerTextures[currentDirection][currentFrame];
    } else {
        toDraw = playerTextures[currentDirection][0]; 
    }
    
    DrawTexturePro(
        toDraw,
        (Rectangle){ 0, 0, toDraw.width, toDraw.height },
        destRect,
        (Vector2){ 0, 0 },
        0.0f,
        WHITE
    );
}

void DrawToolbar() {
    int toolbarHeight = 40;
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    DrawRectangleLines(0,screenHeight - toolbarHeight, screenWidth, toolbarHeight, WHITE);
    DrawRectangle(0, screenHeight - toolbarHeight, screenWidth, toolbarHeight, BLACK);
    DrawRectangle(10, screenHeight - toolbarHeight + 5, 30, 30, (currentTool == TOOL_SEED) ? DARKGREEN : BLACK);
    DrawText("1", 20, screenHeight - toolbarHeight + 10, 20, WHITE);
    DrawRectangle(50, screenHeight - toolbarHeight + 5, 30, 30, (currentTool == TOOL_HOE) ? BROWN : BLACK);
    DrawText("2", 60, screenHeight - toolbarHeight + 10, 20, WHITE);
    DrawRectangle(90, screenHeight - toolbarHeight + 5, 30, 30, (currentTool == TOOL_WATERING_CAN) ? BLUE : BLACK);
    DrawText("3", 100, screenHeight - toolbarHeight + 10, 20, WHITE);
    DrawText(TextFormat("Water: %d", wateringCanUses), screenWidth - 120, screenHeight - toolbarHeight + 10, 20, WHITE);
    DrawRectangle(130, screenHeight - toolbarHeight + 5, 30, 30, 
             (currentTool == TOOL_PURIFIER) ? PURPLE : BLACK);
DrawText("4", 140, screenHeight - toolbarHeight + 10, 20, WHITE);
 DrawText(TextFormat("Purify: %d", purifierCharges), screenWidth - 220, screenHeight - toolbarHeight + 10, 20, WHITE);
 float dayProgress = dayTimer / CYCLE_DURATION;
    DrawRectangle(10, screenHeight - toolbarHeight - 15, 
                 (screenWidth - 20) * dayProgress, 10, WHITE);
}

void GenerateOvalGrass(int centerX, int centerY, int radiusX, int radiusY) {
    for (int y = centerY - radiusY; y <= centerY + radiusY; y++) {
        for (int x = centerX - radiusX; x <= centerX + radiusX; x++) {
            if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) {
                if (map[y][x].type != TILE_WATER) {
                    float dx = (x - centerX) / (float)radiusX;
                    float dy = (y - centerY) / (float)radiusY;
                    if (dx*dx + dy*dy <= 1.0f) {
                        map[y][x].type = TILE_GRASS;
                    }
                }
            }
        }
    }
}

void DrawDayTransitionScreen() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    
    DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.8f));
    
    DrawText(TextFormat("DAY %d ENDED", currentDay-1), 
            screenW/2 - MeasureText("DAY X ENDED", 50)/2, 150, 50, GOLD);
    
    DrawText(TextFormat("Total Earned: $%d", totalEarned),
            screenW/2 - MeasureText("Total Earned: $XXXX", 30)/2, 250, 30, WHITE);
    
    DrawText("Some land was corrupted by aliens...",
            screenW/2 - MeasureText("Some land was corrupted by aliens...", 25)/2, 300, 25, RED);
    
    DrawText("Press ENTER to continue",
            screenW/2 - MeasureText("Press ENTER to continue", 20)/2, 400, 20, LIGHTGRAY);
}

    void PlantSeed(Player player) {
    int x = (int)(player.position.x / TILE_SIZE);
    int y = (int)(player.position.y / TILE_SIZE);

    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) return;

    switch (currentTool) {
        case TOOL_HOE:
            if (map[y][x].type == TILE_GRASS) {
                map[y][x].type = TILE_PLANTING;
                map[y][x].isWatered = false; 
            }
            break;
            
        case TOOL_SEED:
            if (seedCount > 0 && 
                map[y][x].type == TILE_PLANTING && 
                map[y][x].isWatered && 
                !map[y][x].hasSeed) {
                
                map[y][x].hasSeed = true;
                map[y][x].growthStage = 0;
                map[y][x].growthTimer = 0.0f;
                seedCount--;
            }
            break;
            
        case TOOL_WATERING_CAN:
            if (wateringCanUses > 0 && map[y][x].type == TILE_PLANTING) {
                map[y][x].isWatered = true;
                wateringCanUses--;
            }
            break;
            
        case TOOL_PURIFIER:
            PurifyCorruption(player);
            break;
    }
    }
    
void DrawMapBackground(Camera2D camera) {
    Rectangle mapBounds = {
        0, 0, 
        MAP_WIDTH * TILE_SIZE, 
        MAP_HEIGHT * TILE_SIZE
    };
    
    Vector2 cameraTopLeft = GetScreenToWorld2D((Vector2){0, 0}, camera);
    
    // fundalul e tot pe tileuri
    for (float y = cameraTopLeft.y - fmod(cameraTopLeft.y, backgroundTexture.height) - backgroundTexture.height; 
         y < cameraTopLeft.y + GetScreenHeight() + backgroundTexture.height; 
         y += backgroundTexture.height) {
        for (float x = cameraTopLeft.x - fmod(cameraTopLeft.x, backgroundTexture.width) - backgroundTexture.width;
             x < cameraTopLeft.x + GetScreenWidth() + backgroundTexture.width;
             x += backgroundTexture.width) {
            
            // nu deseneaza pe map
            if (!CheckCollisionPointRec((Vector2){x, y}, mapBounds) ||
                !CheckCollisionPointRec((Vector2){x+backgroundTexture.width, y+backgroundTexture.height}, mapBounds)) {
                DrawTexture(backgroundTexture, x, y, WHITE);
            }
        }
    }
}

void PurifyCorruption(Player player) {
      if (purifierCharges <= 0) return;
    
    int x = (int)(player.position.x / TILE_SIZE);
    int y = (int)(player.position.y / TILE_SIZE);
    bool purifiedAny = false;
    
    // Purify 3x3
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT) {
                if (map[ny][nx].type == TILE_CORRUPTED) {
                    map[ny][nx].type = TILE_GRASS;
                    purifiedAny = true;
                }
            }
        }
    }
    
    if (purifiedAny) {
        purifierCharges--;
    }
}

void DrawMap() {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            Vector2 pos = { x * TILE_SIZE, y * TILE_SIZE };

            // Draw base tile
            if (map[y][x].isWatered && map[y][x].type == TILE_PLANTING) {
                DrawTexture(texWateredTile, pos.x, pos.y, WHITE);
            } else {
                switch (map[y][x].type) {
                    case TILE_NORMAL: DrawTexture(texNormal, pos.x, pos.y, WHITE); break;
                    case TILE_PLANTING: DrawTexture(texPlanting, pos.x, pos.y, WHITE); break;
                    case TILE_WATER: DrawTexture(texWater, pos.x, pos.y, WHITE); break;
                    case TILE_SAND: DrawTexture(texSand, pos.x, pos.y, WHITE); break;
                    case TILE_GRASS: DrawTexture(texGrass, pos.x, pos.y, WHITE); break;
                    case TILE_CORRUPTED: DrawTexture(texCorrupted, pos.x, pos.y, WHITE); break;
                    default: DrawRectangle(pos.x, pos.y, TILE_SIZE, TILE_SIZE, GRAY); break;
                }
            }

            // Draw crop
            if (map[y][x].hasSeed) {
                switch (map[y][x].growthStage) {
                    case 0: 
                        DrawTexture(texCropStage1, pos.x, pos.y, ColorAlpha(WHITE, 0.5f)); // Semi-transparent
                        break;
                    case 1:
                        DrawTexture(texCropStage1, pos.x, pos.y, WHITE);
                        break;
                    case 2:
                        DrawTexture(texCropStage2, pos.x, pos.y, WHITE);
                        break;
                    case 3: 
                        DrawTexture(texCropStage3, pos.x, pos.y, WHITE);
                        break;
                }
            }

            // Draw shop
            for (int dy = -1; dy <= 1; dy++) {
    for (int dx = -1; dx <= 1; dx++) {
        int shopX = SELL_TILE_X + dx;
        int shopY = SELL_TILE_Y + dy;
        
        if (shopX >= 0 && shopX < MAP_WIDTH && shopY >= 0 && shopY < MAP_HEIGHT) {
            Vector2 pos = { shopX * TILE_SIZE, shopY * TILE_SIZE };
            
            //Draw pe 96x96
            Rectangle source = {
                (dx+1) * TILE_SIZE, 
                (dy+1) * TILE_SIZE, 
                TILE_SIZE,
                TILE_SIZE
            };
            
            DrawTexturePro(
                texShop,
                source,
                (Rectangle){pos.x, pos.y, TILE_SIZE, TILE_SIZE},
                (Vector2){0, 0},
                0.0f,
                WHITE
            );
        }
    }
}
        }
    }
}

void Harvest(Player player) {
    int x = (int)player.position.x / TILE_SIZE;
    int y = (int)player.position.y / TILE_SIZE;
    if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) {
        if (map[y][x].hasSeed && map[y][x].growthStage >= 3) {
            map[y][x].hasSeed = false;
            map[y][x].growthStage = 0;
            map[y][x].growthTimer = 0.0f;
            harvestedCrops++;
        }
    }
}

void TryOpenShopMenu(Player player) {
      int x = (int)player.position.x / TILE_SIZE;
    int y = (int)player.position.y / TILE_SIZE;
    
    // interactul e pe 3x3
    if (x >= SELL_TILE_X-1 && x <= SELL_TILE_X+1 &&
        y >= SELL_TILE_Y-1 && y <= SELL_TILE_Y+1) {
        shopMenuOpen = true;
    }
}

void DrawShopMenu() {
    int menuWidth = 300;
    int menuHeight = 300;
    int x = GetScreenWidth() / 2 - menuWidth / 2;
    int y = GetScreenHeight() / 2 - menuHeight / 2;

    DrawRectangle(x, y, menuWidth, menuHeight, BLACK);
    DrawRectangleLines(x, y, menuWidth, menuHeight, WHITE);
    DrawText("Shop Menu", x + 20, y + 20, 20, WHITE);
    DrawText(TextFormat("You have %d crops", harvestedCrops), x + 20, y + 60, 18, WHITE);
    DrawText("[ENTER] Sell All", x + 20, y + 90, 18, WHITE);
    DrawText("[B] Buy Seed ($50)", x + 20, y + 120, 18, WHITE);
    DrawText("[ESC] Close", x + 20, y + 180, 18, WHITE);
    DrawText("[P] Buy Purifier ($1500)", x + 20, y + 150, 18, WHITE);
}

void HandleShopMenuInput() {
    if (IsKeyPressed(KEY_ENTER)) {
        balance += harvestedCrops * 100;
        totalEarned +=harvestedCrops * 100;
        harvestedCrops = 0;
    }
    if (IsKeyPressed(KEY_B)) {
        if (balance >= 50) {
            seedCount++;
            balance -= 50;
        }
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        shopMenuOpen = false;
    }
    if (IsKeyPressed(KEY_P)) {
        if (balance >= purifierPrice) {
            purifierCharges += 3; // 3 charges 
            balance -= purifierPrice;
            hasBoughtPurifier = true;
        }
    }
}

void DrawStartScreen() {
 int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    const char *startMsg = "Press ENTER to Start";

    char topName[64] = "None";
    int topScore = 0;
    GetTopFarmer(topName, &topScore);

    DrawText(startMsg, screenW / 2 - MeasureText(startMsg, 30) / 2, screenH / 2, 30, GRAY);

    char topScoreMsg[128];
    sprintf(topScoreMsg, "Top Astronaut: %s ($%d)", topName, topScore);
    DrawText(topScoreMsg, screenW / 2 - MeasureText(topScoreMsg, 20) / 2, screenH / 2 + 50, 20, RED);
}

void SpreadCorruption() {
    // resetare toate tile crop pe tile grass
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (map[y][x].type == TILE_PLANTING) {
                map[y][x].type = TILE_GRASS;
                map[y][x].hasSeed = false;
                map[y][x].growthStage = 0;
                map[y][x].isWatered = false;
            }
        }
    }

    bool corruptionOccurred = false;
    int corruptionTargets[MAP_HEIGHT][MAP_WIDTH] = {0}; 
    
    // identificare tiles necorupte
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (map[y][x].type == TILE_CORRUPTED) {
                int directions[4][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};
                for (int i = 0; i < 4; i++) {
                    int nx = x + directions[i][0];
                    int ny = y + directions[i][1];
                    
                    if (nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT) {
                        if (map[ny][nx].type == TILE_GRASS) {
                            // 15% sa corupa
                            if (rand() % 100 < 15) {
                                corruptionTargets[ny][nx] = 1;
                                corruptionOccurred = true;
                            }
                        }
                    }
                }
            }
        }
    }
    
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (corruptionTargets[y][x]) {
                map[y][x].type = TILE_CORRUPTED;
            }
        }
    }
    
    // fortare coruptie daca nu merge rand
    if (!corruptionOccurred) {
        int attempts = 0;
        while (attempts < 100) {
            int x = rand() % MAP_WIDTH;
            int y = rand() % MAP_HEIGHT;
            
            if (map[y][x].type == TILE_CORRUPTED) {
                int directions[4][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};
                for (int i = 0; i < 4; i++) {
                    int nx = x + directions[i][0];
                    int ny = y + directions[i][1];
                    
                    if (nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT) {
                        if (map[ny][nx].type == TILE_GRASS) {
                            map[ny][nx].type = TILE_CORRUPTED;
                            return;
                        }
                    }
                }
            }
            attempts++;
        }
    }
}

void GetTopFarmer(char *topName, int *topScore) {
    FILE *file = fopen("scores.txt", "r");
    *topScore = 0;
    if (!file) return;

    char line[128];
    while (fgets(line, sizeof(line), file)) {
        char name[64];
        int score;
        if (sscanf(line, "Player: %63[^|]| Earnings: $%d", name, &score) == 2) {
            int len = strlen(name);
            if (len > 0 && name[len - 1] == ' ') name[len - 1] = '\0';

            if (score > *topScore) {
                *topScore = score;
                strcpy(topName, name);
            }
        }
    }

    fclose(file);
}
