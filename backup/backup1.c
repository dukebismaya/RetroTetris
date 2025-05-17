#include "include/raylib.h"
#include "include/raymath.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <sys/stat.h>
#include <direct.h>

// Constants
// Reference design resolution
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 900    
#define REFERENCE_WIDTH 1024
#define REFERENCE_HEIGHT 900   
#define BLOCK_SIZE 40
#define ROWS 20
#define COLS 10
#define BOARD_WIDTH (COLS * BLOCK_SIZE)
#define BOARD_HEIGHT (ROWS * BLOCK_SIZE)
#define MAX_PARTICLES 500
#define MAX_ANIMATIONS 10
#define MAX_FLOATING_TEXT 20
#define MAX_MENU_ITEMS 5

#define SIDEBAR_WIDTH 250       // Width for sidebar panels
#define PANEL_PADDING 20        // Standard padding for panels
#define PREVIEW_SIZE 180        // Size for piece preview panels

#define DARKRED (Color){ 139, 0, 0, 255 }

// Add these cyberpunk color constants at the top with your other defines
#define NEON_PINK     (Color){ 255, 41, 117, 255 }
#define NEON_BLUE     (Color){ 41, 173, 255, 255 }
#define NEON_GREEN    (Color){ 57, 255, 20, 255 }
#define NEON_PURPLE   (Color){ 187, 41, 255, 255 }
#define NEON_YELLOW   (Color){ 255, 236, 41, 255 }
#define CYBER_BLACK   (Color){ 13, 17, 23, 255 }
#define CYBER_DARKBLUE (Color){ 18, 30, 49, 255 }
#define NEON_RED      (Color){ 255, 41, 67, 255 }


#define AI_HEIGHT_WEIGHT -0.510066
#define AI_LINES_WEIGHT 0.760666
#define AI_HOLES_WEIGHT -0.35663
#define AI_BUMPINESS_WEIGHT -0.184483
#define AI_MAX_ROTATIONS 4
#define AI_DELAY_MIN 0.05f
#define AI_DELAY_MAX 0.5f
#define AI_LEVELS 3

// Game states
typedef enum {
    STATE_SPLASH,
    STATE_MAIN_MENU,
    STATE_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAMEOVER
} GameState;

// Tetromino types
typedef enum {
    I_PIECE, O_PIECE, T_PIECE, S_PIECE, Z_PIECE, J_PIECE, L_PIECE
} TetrominoType;

// Animation types
typedef enum {
    ANIM_NONE,
    ANIM_LINE_CLEAR,
    ANIM_PIECE_LOCK,
    ANIM_PIECE_SPAWN
} AnimationType;

// Particle struct
typedef struct {
    Vector2 position;
    Vector2 velocity;
    Color color;
    float size;
    float life;
    float maxLife;
    bool active;
} Particle;

// Animation struct
typedef struct {
    AnimationType type;
    float timer;
    float duration;
    int row;
    bool active;
    Color color;
} Animation;

// Floating text struct for score popups
typedef struct {
    char text[32];
    Vector2 position;
    Vector2 velocity;
    float timer;
    float duration;
    float scale;
    Color color;
    bool active;
} FloatingText;

// Structs
typedef struct {
    int shape[4][4];
    int x, y;
    Color color;
    TetrominoType type;
    float breathFactor; // For breathing animation
    float breathSpeed;
    float lockTimer;    // For locking animation
} Tetromino;

// Game struct
typedef struct {
    int grid[ROWS][COLS];
    int colorGrid[ROWS][COLS];
    Tetromino tetrominoes[7];
    Tetromino current;
    Tetromino next;
    Tetromino held;
    bool canHold;
    bool hasHeld;
    int score;
    int level;
    int lines;
    GameState state;
    float timer;
    float delay;
    float softDropY;    // For smooth soft drop
    Particle particles[MAX_PARTICLES];
    Animation animations[MAX_ANIMATIONS];
    int particleCount;
    int highScore;
    Font gameFont;
    Font arrowFont;      // Dedicated font for arrow icons
    bool customFontLoaded;
    bool arrowFontLoaded;
    int combo;
    int menuSelection;
    bool soundEnabled;
    Sound rotateSound;
    Sound dropSound;
    Sound clearSound;
    Sound tetrisSound;
    Sound menuSound;
    Sound gameoverSound;
    
    // Sound settings
    bool musicEnabled;
    float masterVolume;
    float musicVolume;
    float sfxVolume;
    float sfxPitch;
    float sfxPan;
    
    // Sound resources
    Sound moveSound;
    Sound holdSound;
    Sound levelUpSound;
    
    // Background music
    Music backgroundMusic;
    
    // Audio settings menu
    bool showAudioSettings;
    int audioMenuSelection;

    // Auto-play settings
    bool autoPlayEnabled;
    float autoPlayDelay;
    int autoPlayLevel;        // AI difficulty level
    bool showAutoPlaySettings;
    int autoPlayMenuSelection;
} Game;

// Global Variables
Game game;
Shader glowShader;
Shader blockShader;
int glowTimeLocation;
int blockTimeLocation;
FloatingText floatingTexts[MAX_FLOATING_TEXT];
float splashTimer = 0.0f;
int mainMenuSelection = 0;
Texture2D engineLogo;
bool logoLoaded = false;
bool shouldExitGame = false;
// Scaling factors - updated when window size changes
float scaleX = 1.0f;
float scaleY = 1.0f;
float BOARD_OFFSET_X;
float BOARD_OFFSET_Y;

// Function Prototypes
void InitGame();
void InitTetrominoes();
Tetromino GetRandomTetromino();
bool CheckCollision(Tetromino t);
void MergeToGrid(Tetromino t);
int ClearLines();
void RotateTetromino(Tetromino *t);
void DrawTetrisGrid();
void DrawTetromino(Tetromino t, int offsetX, int offsetY, float alpha);
void DrawGhostPiece(Tetromino t);
void HandleInput();
void UpdateGame();
void DrawGame();
void DrawUI();
int GetDropPosition(Tetromino t);
void HoldPiece();
void SpawnParticles(Vector2 position, Color color, int count);
void UpdateParticles();
void DrawParticles();
void AddLineAnimation(int row, Color color);
void AddLockAnimation(Tetromino t);
void UpdateAnimations();
void DrawAnimations();
void LoadShaders();
void UnloadShaders();
void LoadGameResources();
void UnloadGameResources();
void DrawPauseMenu();
void DrawAudioSettings();
float ClampValue(float value, float min, float max);
bool IsMouseOverRect(Rectangle rect);
void HandleMouseInput();
void InitFloatingTexts();
void AddFloatingText(const char* text, Vector2 position, Color color, float scale);
void UpdateFloatingTexts();
void DrawFloatingTexts();
void DrawMouseCursor();
void SaveHighScore(int score);
int LoadHighScore();
void DrawGameOverScreen();
void DrawSplashScreen();
void DrawMainMenu();
void HandleMainMenuInput();
void ProcessMainMenuSelection();
void ToggleAutoPlay();
void UpdateAutoPlay();
void DrawAutoPlayIndicator();
void EvaluateAndExecuteBestMove();
float EvaluateMove(Tetromino t, int dropPosition, int rotations);
void AddAutoPlaySettings();
void DrawAutoPlaySettings();
void HandleAutoPlaySettings();

// Scale a value based on horizontal scaling factor
float ScaleX(float value) {
    return value * scaleX;
}

// Scale a value based on vertical scaling factor
float ScaleY(float value) {
    return value * scaleY;
}

// Scale a position from reference coordinates to actual screen coordinates
Vector2 ScalePosition(float x, float y) {
    return (Vector2){ x * scaleX, y * scaleY };
}

// Get a rectangle scaled to the current screen dimensions
Rectangle ScaleRect(float x, float y, float width, float height) {
    return (Rectangle){ 
        x * scaleX, 
        y * scaleY, 
        width * scaleX, 
        height * scaleY 
    };
}

// Scale font size with better readability at any resolution
int ScaleFontSize(int size) {
    return (int)((scaleX + scaleY) * size / 2.0f);
}

void DrawArrow(const char* arrowChar, float x, float y, float fontSize, Color color) {
    if (game.arrowFontLoaded) {
        // Draw with Arrow font
        DrawTextEx(game.arrowFont, arrowChar, (Vector2){x, y}, fontSize, 1, color);
    } else {
        // Fallback to regular DrawText
        DrawText(arrowChar, x, y, fontSize, color);
    }
}

// Initialize floating text system
void InitFloatingTexts() {
    for (int i = 0; i < MAX_FLOATING_TEXT; i++) {
        floatingTexts[i].active = false;
    }
}

// Add floating text at position
void AddFloatingText(const char* text, Vector2 position, Color color, float scale) {
    for (int i = 0; i < MAX_FLOATING_TEXT; i++) {
        if (!floatingTexts[i].active) {
            strcpy(floatingTexts[i].text, text);
            floatingTexts[i].position = position;
            floatingTexts[i].velocity = (Vector2){ 0, -100.0f };
            floatingTexts[i].timer = 0.0f;
            floatingTexts[i].duration = 1.5f;
            floatingTexts[i].color = color;
            floatingTexts[i].scale = scale;
            floatingTexts[i].active = true;
            break;
        }
    }
}


// Update floating text system for cyberpunk style
void UpdateFloatingTexts() {
    float deltaTime = GetFrameTime();
    
    for (int i = 0; i < MAX_FLOATING_TEXT; i++) {
        if (floatingTexts[i].active) {
            floatingTexts[i].timer += deltaTime;
            
            // Apply smoother easing curve
            float progress = floatingTexts[i].timer / floatingTexts[i].duration;
            float easeFactor = 1.0f - powf(progress, 2.0f); // Quadratic ease-out
            
            Vector2 currentVelocity = floatingTexts[i].velocity;
            currentVelocity.y *= easeFactor;
            
            floatingTexts[i].position.x += currentVelocity.x * deltaTime;
            floatingTexts[i].position.y += currentVelocity.y * deltaTime;
            
            // Add cyberpunk-style horizontal wave motion
            floatingTexts[i].position.x += sinf(floatingTexts[i].timer * 4.0f) * 1.2f;
            
            // Fade out near the end
            if (floatingTexts[i].timer >= floatingTexts[i].duration) {
                floatingTexts[i].active = false;
            }
        }
    }
}

void DrawFloatingTexts() {
    for (int i = 0; i < MAX_FLOATING_TEXT; i++) {
        if (floatingTexts[i].active) {
            float progress = floatingTexts[i].timer / floatingTexts[i].duration;
            float alpha = 1.0f;
            
            // Improved fade in/out curve
            if (progress < 0.2f) {
                alpha = progress / 0.2f;
            } else if (progress > 0.8f) {
                alpha = (1.0f - progress) / 0.2f;
            }
            
            // Dynamic scale with pulsing effect
            float baseScale = floatingTexts[i].scale;
            float scaleEffect = 1.0f;
            
            if (progress < 0.2f) {
                // Initial pop-in effect
                scaleEffect = 0.4f + (progress / 0.2f) * 0.6f;
            } else {
                // Subtle pulsing
                scaleEffect = 1.0f + sinf(progress * 15.0f) * 0.05f;
            }
            
            Color textColor = ColorAlpha(floatingTexts[i].color, alpha);
            
            // Draw text with glitch effect for special scores
            if (strcmp(floatingTexts[i].text, "TETRIS!") == 0 || strstr(floatingTexts[i].text, "COMBO") != NULL) {
                // Draw glitch copies
                for (int j = 0; j < 2; j++) {
                    float glitchX = sinf(GetTime() * 20.0f + j * 1.5f) * 2.0f;
                    float glitchY = cosf(GetTime() * 18.0f + j * 2.1f) * 2.0f;
                    
                    if (game.customFontLoaded) {
                        Vector2 position = floatingTexts[i].position;
                        float fontSize = 30.0f * baseScale * scaleEffect;
                        Vector2 textSize = MeasureTextEx(game.gameFont, floatingTexts[i].text, fontSize, 2);
                        position.x -= textSize.x / 2;
                        
                        DrawTextEx(game.gameFont, 
                                floatingTexts[i].text, 
                                (Vector2){position.x + glitchX, position.y + glitchY}, 
                                fontSize, 
                                2, 
                                ColorAlpha(NEON_BLUE, alpha * 0.3f));
                    }
                }
            }
            
            // Draw main text
            if (game.customFontLoaded) {
                Vector2 position = floatingTexts[i].position;
                float fontSize = 30.0f * baseScale * scaleEffect;
                Vector2 textSize = MeasureTextEx(game.gameFont, floatingTexts[i].text, fontSize, 2);
                position.x -= textSize.x / 2;
                
                // Draw subtle shadow for depth
                DrawTextEx(game.gameFont, 
                        floatingTexts[i].text, 
                        (Vector2){position.x + 2, position.y + 2}, 
                        fontSize, 
                        2, 
                        ColorAlpha(BLACK, alpha * 0.5f));
                
                // Draw main text
                DrawTextEx(game.gameFont, 
                        floatingTexts[i].text, 
                        position, 
                        fontSize, 
                        2, 
                        textColor);
            } else {
                // Fallback with regular DrawText
                int fontSize = (int)(30 * baseScale * scaleEffect);
                int textWidth = MeasureText(floatingTexts[i].text, fontSize);
                
                // Draw shadow
                DrawText(floatingTexts[i].text, 
                        (int)(floatingTexts[i].position.x - textWidth/2) + 2, 
                        (int)floatingTexts[i].position.y + 2, 
                        fontSize, 
                        ColorAlpha(BLACK, alpha * 0.5f));
                
                // Draw main text
                DrawText(floatingTexts[i].text, 
                        (int)(floatingTexts[i].position.x - textWidth/2), 
                        (int)floatingTexts[i].position.y, 
                        fontSize, 
                        textColor);
            }
        }
    }
}

// Initialize the game
void InitGame() {
    // Clear the grid
    memset(game.grid, 0, sizeof(game.grid));
    memset(game.colorGrid, 0, sizeof(game.colorGrid));
    
    // Initialize game parameters
    game.score = 0;
    game.level = 1;
    game.lines = 0;
    game.timer = 0;
    game.delay = 1.0f;
    game.canHold = true;
    game.hasHeld = false;
    game.softDropY = 0;
    game.particleCount = 0;
    game.combo = 0;
    game.menuSelection = 0;
    mainMenuSelection = 0;

    // Save current auto-play enabled state
    bool wasAutoPlayEnabled = game.autoPlayEnabled;
    
    // Clear animations and particles
    for (int i = 0; i < MAX_PARTICLES; i++) {
        game.particles[i].active = false;
    }
    
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        game.animations[i].active = false;
    }
    
    // Initialize tetrominoes
    InitTetrominoes();
    
    // Set initial pieces
    game.current = GetRandomTetromino();
    game.current.breathFactor = 0.0f;
    game.current.breathSpeed = GetRandomValue(1, 3) / 10.0f;
    game.next = GetRandomTetromino();
    game.next.breathFactor = 0.0f;
    game.next.breathSpeed = GetRandomValue(1, 3) / 10.0f;
    
    // Initialize floating texts
    InitFloatingTexts();
    
    // Start playing background music if enabled
    if (game.musicEnabled) {
        PlayMusicStream(game.backgroundMusic);
    }

    // Restore auto-play state if it was enabled
    if (wasAutoPlayEnabled) {
        game.autoPlayEnabled = true;
        
        // Show a notification that auto-play is active
        AddFloatingText("AUTO-PLAY ACTIVE", 
                      (Vector2){SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 100},
                      NEON_GREEN, 1.5f);
    }
}

// Initialize Tetromino shapes
void InitTetrominoes() {
    // I-piece
    game.tetrominoes[I_PIECE] = (Tetromino){
        {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}, 
        3, 0, SKYBLUE, I_PIECE, 0.0f, 0.0f, 0.0f
    };
    
    // O-piece
    game.tetrominoes[O_PIECE] = (Tetromino){
        {{1,1,0,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}}, 
        4, 0, YELLOW, O_PIECE, 0.0f, 0.0f, 0.0f
    };
    
    // T-piece
    game.tetrominoes[T_PIECE] = (Tetromino){
        {{0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}}, 
        3, 0, PURPLE, T_PIECE, 0.0f, 0.0f, 0.0f
    };
    
    // S-piece
    game.tetrominoes[S_PIECE] = (Tetromino){
        {{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}}, 
        3, 0, GREEN, S_PIECE, 0.0f, 0.0f, 0.0f
    };
    
    // Z-piece
    game.tetrominoes[Z_PIECE] = (Tetromino){
        {{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}}, 
        3, 0, RED, Z_PIECE, 0.0f, 0.0f, 0.0f
    };
    
    // J-piece
    game.tetrominoes[J_PIECE] = (Tetromino){
        {{1,0,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}}, 
        3, 0, BLUE, J_PIECE, 0.0f, 0.0f, 0.0f
    };
    
    // L-piece
    game.tetrominoes[L_PIECE] = (Tetromino){
        {{0,0,1,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}}, 
        3, 0, ORANGE, L_PIECE, 0.0f, 0.0f, 0.0f
    };
}

// Load shaders
void LoadShaders() {
    // Glow shader for pieces
    glowShader = LoadShader(0, TextFormat("resources/shaders/glow.fs"));
    
    // Create a time location for the shader
    int glowTimeLoc = GetShaderLocation(glowShader, "time");
    
    // Block shader for breathing effect
    blockShader = LoadShader(0, TextFormat("resources/shaders/block.fs"));
    
    // Create a time location for the shader
    int blockTimeLoc = GetShaderLocation(blockShader, "time");
    
    // Store the time locations for updating later
    glowTimeLocation = glowTimeLoc;
    blockTimeLocation = blockTimeLoc;
}

// Unload shaders
void UnloadShaders() {
    UnloadShader(glowShader);
    UnloadShader(blockShader);
}

void LoadGameResources() {
    // Initialize font with proper error handling
    game.customFontLoaded = false;
    game.arrowFontLoaded = false;
    
    const char* fontPath = "resources/fonts/RobotoCondensed-SemiBold.ttf";
    const char* arrowFontPath = "resources/fonts/ARIAL.TTF";
    
    // First check if the font file exists
    if (FileExists(fontPath)) {
        TraceLog(LOG_INFO, "Font file found, attempting to load: %s", fontPath);
        
        // Attempt to load the font
        int fontSize = 32; // Base font size
        game.gameFont = LoadFontEx(fontPath, fontSize, NULL, 0); // Load all glyphs
        
        // Properly validate the font with multiple checks
        if (game.gameFont.texture.id > 0 && 
            game.gameFont.glyphCount > 0 && 
            game.gameFont.baseSize > 0) {
            
            game.customFontLoaded = true;
            TraceLog(LOG_INFO, "Custom font loaded successfully: %d glyphs", 
                    game.gameFont.glyphCount);
        } else {
            TraceLog(LOG_WARNING, "Font loaded but appears invalid: texture.id=%d, glyphCount=%d, baseSize=%d", 
                    game.gameFont.texture.id, game.gameFont.glyphCount, game.gameFont.baseSize);
            
            // Clean up invalid font to avoid memory leaks
            if (game.gameFont.texture.id > 0) {
                UnloadFont(game.gameFont);
            }
            
            // Fall back to default font
            game.gameFont = GetFontDefault();
            game.customFontLoaded = false;
        }
    } else {
        TraceLog(LOG_WARNING, "Font file not found: %s", fontPath);
        game.gameFont = GetFontDefault();
        game.customFontLoaded = false;
    }

    if (FileExists(arrowFontPath)) {
        TraceLog(LOG_INFO, "Arrow font found, loading for arrows: %s", arrowFontPath);
        
        game.arrowFont = LoadFontEx(arrowFontPath, 24, NULL, 0);
        
        if (game.arrowFont.texture.id > 0 && 
            game.arrowFont.glyphCount > 0 && 
            game.arrowFont.baseSize > 0) {
            
            game.arrowFontLoaded = true;
            TraceLog(LOG_INFO, "Arrow font loaded successfully");
        } else {
            TraceLog(LOG_WARNING, "Arrow font loaded but appears invalid");
            
            // Clean up invalid font
            if (game.arrowFont.texture.id > 0) {
                UnloadFont(game.arrowFont);
            }
        }
    } else {
        TraceLog(LOG_WARNING, "Custom arrow font not found, using default for arrows");
    }
    
    // Use the font info to log what we're using
    TraceLog(LOG_INFO, "Using font with %d glyphs, baseSize: %d", 
            game.gameFont.glyphCount, game.gameFont.baseSize);
    

    // Load game sounds (only once)
    game.soundEnabled = true;
    game.moveSound = LoadSound("resources/sounds/move.wav");
    game.rotateSound = LoadSound("resources/sounds/rotate.wav");
    game.dropSound = LoadSound("resources/sounds/drop.wav");
    game.clearSound = LoadSound("resources/sounds/clear.wav");
    game.tetrisSound = LoadSound("resources/sounds/tetris.wav");
    
    // Better error handling for missing sound files
    const char* levelupSoundPath = "resources/sounds/levelup.wav";
    if (FileExists(levelupSoundPath)) {
        game.levelUpSound = LoadSound(levelupSoundPath);
    } else {
        TraceLog(LOG_WARNING, "Sound file not found: %s", levelupSoundPath);
        // Initialize with empty sound to avoid crashes
        game.levelUpSound = (Sound){ 0 };
    }
    
    game.gameoverSound = LoadSound("resources/sounds/gameover.wav");
    
    // Handle potentially missing menu sound
    const char* menuSoundPath = "resources/sounds/menu.wav";
    if (FileExists(menuSoundPath)) {
        game.menuSound = LoadSound(menuSoundPath);
    } else {
        TraceLog(LOG_WARNING, "Sound file not found: %s", menuSoundPath);
        // Use another sound as fallback to avoid crashes
        game.menuSound = game.moveSound;
    }
    
    // Handle potentially missing hold sound
    const char* holdSoundPath = "resources/sounds/hold.wav";
    if (FileExists(holdSoundPath)) {
        game.holdSound = LoadSound(holdSoundPath);
    } else {
        TraceLog(LOG_WARNING, "Sound file not found: %s", holdSoundPath);
        // Use another sound as fallback
        game.holdSound = game.rotateSound;
    }
    
    // Load background music
    const char* musicPath = "resources/music/tetris_theme.ogg";
    if (FileExists(musicPath)) {
        game.backgroundMusic = LoadMusicStream(musicPath);
        SetMusicVolume(game.backgroundMusic, 0.7f);  // Default music volume
    } else {
        TraceLog(LOG_WARNING, "Music file not found: %s", musicPath);
        // Initialize with empty music stream to avoid crashes
        game.backgroundMusic = (Music){ 0 };
    }
    
    // Initialize sound settings
    game.soundEnabled = true;
    game.musicEnabled = true;
    game.masterVolume = 1.0f;
    game.musicVolume = 0.7f;
    game.sfxVolume = 1.0f;
    game.sfxPitch = 1.0f;
    game.sfxPan = 0.0f;
    game.showAudioSettings = false;
    game.audioMenuSelection = 0;
}

void UnloadGameResources() {
    if (game.customFontLoaded) {
        UnloadFont(game.gameFont);
    }

    if (game.arrowFontLoaded) {
        UnloadFont(game.arrowFont);
    }
    
    // Unload all sounds (only once)
    UnloadSound(game.moveSound);
    UnloadSound(game.rotateSound);
    UnloadSound(game.dropSound);
    UnloadSound(game.clearSound);
    UnloadSound(game.tetrisSound);
    UnloadSound(game.levelUpSound);
    UnloadSound(game.gameoverSound);
    UnloadSound(game.menuSound);
    UnloadSound(game.holdSound);
    
    // Unload music
    UnloadMusicStream(game.backgroundMusic);
}

// Helper function to play sound with current settings
void PlayGameSound(Sound sound) {
    if (!game.soundEnabled) return;
    
    // Apply current settings
    SetSoundVolume(sound, game.masterVolume * game.sfxVolume);
    SetSoundPitch(sound, game.sfxPitch);
    SetSoundPan(sound, game.sfxPan);
    
    PlaySound(sound);
}

// Get a random Tetromino
Tetromino GetRandomTetromino() {
    Tetromino t = game.tetrominoes[rand() % 7];
    t.breathFactor = 0.0f;
    t.breathSpeed = GetRandomValue(1, 3) / 10.0f;
    t.lockTimer = 0.0f;
    return t;
}

// Check if a tetromino collides with the grid or boundaries
bool CheckCollision(Tetromino t) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (t.shape[i][j]) {
                int x = t.x + j;
                int y = t.y + i;

                if (x < 0 || x >= COLS || y >= ROWS || (y >= 0 && game.grid[y][x])) {
                    return true;
                }
            }
        }
    }
    return false;
}

// Merge Tetromino into the grid
void MergeToGrid(Tetromino t) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (t.shape[i][j]) {
                int y = t.y + i;
                int x = t.x + j;
                if (y >= 0 && y < ROWS && x >= 0 && x < COLS) {
                    game.grid[y][x] = 1;
                    game.colorGrid[y][x] = t.type;
                    
                    // Spawn particles at lock position
                    Vector2 particlePos = {
                        BOARD_OFFSET_X + (x * BLOCK_SIZE) + BLOCK_SIZE/2,
                        BOARD_OFFSET_Y + (y * BLOCK_SIZE) + BLOCK_SIZE/2
                    };
                    SpawnParticles(particlePos, t.color, 5);
                }
            }
        }
    }
    
    // Add lock animation
    AddLockAnimation(t);
}

// Clear full lines and return the number of lines cleared
int ClearLines() {
    int linesCleared = 0;
    
    for (int i = ROWS - 1; i >= 0; i--) {
        bool full = true;
        for (int j = 0; j < COLS; j++) {
            if (!game.grid[i][j]) {
                full = false;
                break;
            }
        }
        
        if (full) {
            // Add line clear animation
            Color rowColor = game.tetrominoes[game.colorGrid[i][0]].color;
            AddLineAnimation(i, rowColor);
            
            // Spawn particles along the line
            for (int j = 0; j < COLS; j++) {
                Vector2 particlePos = {
                    BOARD_OFFSET_X + (j * BLOCK_SIZE) + BLOCK_SIZE/2,
                    BOARD_OFFSET_Y + (i * BLOCK_SIZE) + BLOCK_SIZE/2
                };
                Color blockColor = game.tetrominoes[game.colorGrid[i][j]].color;
                SpawnParticles(particlePos, blockColor, 10);
            }
            
            linesCleared++;
            
            // Move all lines above down (done after animation completes)
            for (int k = i; k > 0; k--) {
                for (int j = 0; j < COLS; j++) {
                    game.grid[k][j] = game.grid[k - 1][j];
                    game.colorGrid[k][j] = game.colorGrid[k - 1][j];
                }
            }
            
            // Clear the top line
            for (int j = 0; j < COLS; j++) {
                game.grid[0][j] = 0;
                game.colorGrid[0][j] = 0;
            }
            
            i++; // Recheck this row
        }
    }
    
    return linesCleared;
}

// Get the lowest valid position for the current piece (for ghost piece)
int GetDropPosition(Tetromino t) {
    int lowestY = t.y;
    
    while (true) {
        t.y++;
        if (CheckCollision(t)) {
            return lowestY;
        }
        lowestY = t.y;
    }
}

// Improved rotation with wall kicks
void RotateTetromino(Tetromino *t) {
    // Special case for O piece (doesn't rotate)
    if (t->type == O_PIECE) return;
    
    int temp[4][4] = {0};
    
    // Rotate the piece
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            temp[i][j] = t->shape[3 - j][i];
        }
    }
    
    Tetromino test = *t;
    memcpy(test.shape, temp, sizeof(temp));
    
    // Try normal rotation
    if (!CheckCollision(test)) {
        memcpy(t->shape, temp, sizeof(temp));
        return;
    }
    
    // Try wall kicks - left, right, up, and down
    const int kicks[5][2] = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}
    };
    
    for (int i = 0; i < 5; i++) {
        test = *t;
        memcpy(test.shape, temp, sizeof(temp));
        test.x += kicks[i][0];
        test.y += kicks[i][1];
        
        if (!CheckCollision(test)) {
            memcpy(t->shape, temp, sizeof(temp));
            t->x += kicks[i][0];
            t->y += kicks[i][1];
            return;
        }
    }
    
    // For I piece, try additional kicks
    if (t->type == I_PIECE) {
        const int i_kicks[3][2] = {
            {-2, 0}, {2, 0}, {0, -2}
        };
        
        for (int i = 0; i < 3; i++) {
            test = *t;
            memcpy(test.shape, temp, sizeof(temp));
            test.x += i_kicks[i][0];
            test.y += i_kicks[i][1];
            
            if (!CheckCollision(test)) {
                memcpy(t->shape, temp, sizeof(temp));
                t->x += i_kicks[i][0];
                t->y += i_kicks[i][1];
                return;
            }
        }
    }
    
    // If all else fails, don't rotate
}

// Hold the current piece
void HoldPiece() {
    if (!game.canHold) return;
    
    if (!game.hasHeld) {
        game.held = game.current;
        game.held.x = 3;
        game.held.y = 0;
        game.current = game.next;
        game.next = GetRandomTetromino();
        game.hasHeld = true;
    } else {
        Tetromino temp = game.current;
        game.current = game.held;
        game.held = temp;
        game.current.x = 3;
        game.current.y = 0;
    }
    
    // Reset breathing animation
    game.current.breathFactor = 0.0f;
    game.current.breathSpeed = GetRandomValue(1, 3) / 10.0f;
    
    // Reset soft drop position
    game.softDropY = game.current.y;
    
    game.canHold = false;
}

// Update the particle system with more variety and cyberpunk effects
void SpawnParticles(Vector2 position, Color color, int count) {
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < MAX_PARTICLES; j++) {
            if (!game.particles[j].active) {
                // Randomize particle behavior
                int particleType = GetRandomValue(0, 2);
                
                game.particles[j].position = position;
                
                // Different velocity patterns based on type
                if (particleType == 0) {
                    // Standard particles
                    game.particles[j].velocity.x = (float)GetRandomValue(-100, 100) / 60.0f;
                    game.particles[j].velocity.y = (float)GetRandomValue(-100, 100) / 60.0f;
                    game.particles[j].size = (float)GetRandomValue(2, 5);
                } else if (particleType == 1) {
                    // Directional particles (mostly upward)
                    game.particles[j].velocity.x = (float)GetRandomValue(-60, 60) / 60.0f;
                    game.particles[j].velocity.y = (float)GetRandomValue(-150, -30) / 60.0f;
                    game.particles[j].size = (float)GetRandomValue(1, 3);
                } else {
                    // Glowing pixel dust
                    game.particles[j].velocity.x = (float)GetRandomValue(-40, 40) / 80.0f;
                    game.particles[j].velocity.y = (float)GetRandomValue(-40, 40) / 80.0f;
                    game.particles[j].size = (float)GetRandomValue(1, 2);
                }
                
                // Brighten color a bit for glow effect
                game.particles[j].color = ColorBrightness(color, 0.2f);
                game.particles[j].life = 1.0f;
                game.particles[j].maxLife = (float)GetRandomValue(80, 120) / 100.0f; // Varied lifespans
                game.particles[j].active = true;
                game.particleCount++;
                break;
            }
        }
    }
}

void UpdateParticles() {
    float deltaTime = GetFrameTime();
    
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (game.particles[i].active) {
            // Apply velocity with slight deceleration
            game.particles[i].velocity.y += 0.01f; // Slight gravity
            
            game.particles[i].position.x += game.particles[i].velocity.x;
            game.particles[i].position.y += game.particles[i].velocity.y;
            
            // Update life with non-linear decay for smoother fadeout
            game.particles[i].life -= deltaTime / game.particles[i].maxLife;
            
            if (game.particles[i].life <= 0) {
                game.particles[i].active = false;
                game.particleCount--;
            }
        }
    }
}

void DrawParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (game.particles[i].active) {
            // Calculate alpha with smooth falloff
            float alpha = game.particles[i].life;
            
            // For small particles, draw as pixels or tiny squares
            if (game.particles[i].size <= 2.0f) {
                // Draw with subtle glow effect
                DrawRectangle(
                    (int)game.particles[i].position.x - 1, 
                    (int)game.particles[i].position.y - 1,
                    2, 2,
                    ColorAlpha(ColorBrightness(game.particles[i].color, 0.2f), alpha * 0.3f)
                );
                
                DrawRectangle(
                    (int)game.particles[i].position.x, 
                    (int)game.particles[i].position.y,
                    1, 1,
                    ColorAlpha(game.particles[i].color, alpha)
                );
            } else {
                // Draw larger particles as circles with glow
                Color glowColor = ColorAlpha(ColorBrightness(game.particles[i].color, 0.2f), alpha * 0.5f);
                Color particleColor = ColorAlpha(game.particles[i].color, alpha);
                
                // Draw outer glow
                DrawCircleV(game.particles[i].position, 
                           game.particles[i].size * 1.5f, 
                           ColorAlpha(glowColor, alpha * 0.3f));
                
                // Draw main particle
                DrawCircleV(game.particles[i].position, 
                           game.particles[i].size, 
                           particleColor);
            }
        }
    }
}

// Add line clear animation
void AddLineAnimation(int row, Color color) {
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        if (!game.animations[i].active) {
            game.animations[i].type = ANIM_LINE_CLEAR;
            game.animations[i].timer = 0.0f;
            game.animations[i].duration = 0.5f;
            game.animations[i].row = row;
            game.animations[i].color = color;
            game.animations[i].active = true;
            break;
        }
    }
}

// Piece lock animation
void AddLockAnimation(Tetromino t) {
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        if (!game.animations[i].active) {
            game.animations[i].type = ANIM_PIECE_LOCK;
            game.animations[i].timer = 0.0f;
            game.animations[i].duration = 0.3f;
            game.animations[i].color = t.color;
            game.animations[i].active = true;
            break;
        }
    }
}

// Update animations
void UpdateAnimations() {
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        if (game.animations[i].active) {
            game.animations[i].timer += GetFrameTime();
            
            if (game.animations[i].timer >= game.animations[i].duration) {
                game.animations[i].active = false;
            }
        }
    }
}

void DrawAnimations() {
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        if (game.animations[i].active) {
            if (game.animations[i].type == ANIM_LINE_CLEAR) {
                // Improved line clear with cyberpunk glitch effect
                float progress = game.animations[i].timer / game.animations[i].duration;
                int row = game.animations[i].row;
                
                // Phase 1: Glowing flash
                if (progress < 0.4f) {
                    float flashIntensity = sinf(progress * 25.0f) * 0.5f + 0.5f;
                    Color flashColor = ColorAlpha(WHITE, flashIntensity * (1.0f - progress/0.4f));
                    
                    DrawRectangle(
                        BOARD_OFFSET_X, 
                        BOARD_OFFSET_Y + row * BLOCK_SIZE, 
                        COLS * BLOCK_SIZE, 
                        BLOCK_SIZE, 
                        flashColor
                    );
                }
                
                // Phase 2: Glitch dissolve
                else if (progress < 0.85f) {
                    float normalizedProgress = (progress - 0.4f) / 0.45f;
                    
                    // Generate glitch blocks that disappear over time
                    for (int j = 0; j < COLS; j++) {
                        // Use deterministic "random" based on position and time
                        float glitchRandom = sinf(j * 1.3f + GetTime() * 10.0f);
                        float glitchThreshold = normalizedProgress * 0.8f;
                        
                        if (fabsf(glitchRandom) > glitchThreshold) {
                            float heightMod = sinf(j * 3.7f + GetTime() * 5.0f) * 0.4f + 0.6f;
                            float widthMod = 0.5f + fabsf(sinf(j * 2.1f + GetTime() * 8.0f)) * 0.5f;
                            
                            DrawRectangle(
                                BOARD_OFFSET_X + j * BLOCK_SIZE + (int)(BLOCK_SIZE * (1.0f - widthMod) / 2),
                                BOARD_OFFSET_Y + row * BLOCK_SIZE + (int)(BLOCK_SIZE * (1.0f - heightMod) / 2),
                                (int)(BLOCK_SIZE * widthMod),
                                (int)(BLOCK_SIZE * heightMod),
                                ColorAlpha(NEON_BLUE, (1.0f - normalizedProgress) * 0.8f)
                            );
                        }
                    }
                }
                
                // Phase 3: Horizontal line collapse effect
                else {
                    float normalizedProgress = (progress - 0.85f) / 0.15f;
                    int collapseWidth = (int)(COLS * BLOCK_SIZE * (1.0f - normalizedProgress));
                    
                    DrawRectangle(
                        BOARD_OFFSET_X + (COLS * BLOCK_SIZE - collapseWidth) / 2,
                        BOARD_OFFSET_Y + row * BLOCK_SIZE,
                        collapseWidth,
                        BLOCK_SIZE,
                        ColorAlpha(NEON_BLUE, (1.0f - normalizedProgress) * 0.5f)
                    );
                }
            }
            else if (game.animations[i].type == ANIM_PIECE_LOCK) {
                // Improved piece lock animation with shockwave effect
                float progress = game.animations[i].timer / game.animations[i].duration;
                float shockwaveRadius = progress * BOARD_WIDTH * 0.6f;
                float ringThickness = 6.0f * (1.0f - progress);
                
                // Draw shockwave rings
                if (ringThickness > 0.5f) {
                    DrawRing(
                        (Vector2){BOARD_OFFSET_X + BOARD_WIDTH/2, BOARD_OFFSET_Y + BOARD_HEIGHT/2},
                        shockwaveRadius - ringThickness/2,
                        shockwaveRadius + ringThickness/2,
                        0, 360,
                        36,
                        ColorAlpha(game.animations[i].color, (1.0f - progress) * 0.4f)
                    );
                }
                
                // Draw screen flash
                float flashAlpha = (1.0f - progress) * 0.15f;
                if (flashAlpha > 0.01f) {
                    DrawRectangle(
                        BOARD_OFFSET_X, 
                        BOARD_OFFSET_Y, 
                        COLS * BLOCK_SIZE, 
                        ROWS * BLOCK_SIZE, 
                        ColorAlpha(game.animations[i].color, flashAlpha)
                    );
                }
            }
        }
    }
}

// Update the DrawTetrisGrid function with cyber-style grid
void DrawTetrisGrid() {
    float scaledBlockSize = ScaleY(BLOCK_SIZE);
    float boardWidth = scaledBlockSize * COLS;
    float boardHeight = scaledBlockSize * ROWS;
    Vector2 boardPos = ScalePosition(BOARD_OFFSET_X, BOARD_OFFSET_Y);
    
    // Draw outer border glow with pulsing effect
    float glowIntensity = sinf(GetTime() * 2.0f) * 0.2f + 0.8f;
    
    for(int i = 0; i < 3; i++) {
        float glowSize = ScaleY(15.0f - i * 5.0f);
        DrawRectangle(
            boardPos.x - glowSize, 
            boardPos.y - glowSize,
            boardWidth + glowSize * 2, 
            boardHeight + glowSize * 2,
            ColorAlpha(NEON_BLUE, 0.1f * glowIntensity * (3-i)/3)
        );
    }
    
    // Draw board background
    DrawRectangle(
        boardPos.x,
        boardPos.y,
        boardWidth,
        boardHeight,
        ColorAlpha(CYBER_BLACK, 0.8f)
    );
    
    // Draw grid with cyber effect
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            // Draw grid cell
            DrawRectangleLines(
                boardPos.x + j * scaledBlockSize,
                boardPos.y + i * scaledBlockSize,
                scaledBlockSize,
                scaledBlockSize,
                ColorAlpha(NEON_BLUE, 0.1f + (float)i/ROWS * 0.05f)
            );
        }
    }
    
    // Draw filled blocks with neon effect
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (game.grid[i][j]) {
                Color blockColor = game.tetrominoes[game.colorGrid[i][j]].color;
                Color glowColor = blockColor;
                Color darkColor = ColorBrightness(blockColor, -0.4f);
                
                // Calculate glow intensity with time
                float blockGlow = (sinf(GetTime() * 3.0f + i*0.1f + j*0.1f) * 0.2f + 0.8f);
                
                // Draw block with improved neon effect
                DrawRectangle(
                    boardPos.x + j * scaledBlockSize + ScaleY(1), 
                    boardPos.y + i * scaledBlockSize + ScaleY(1), 
                    scaledBlockSize - ScaleY(2), scaledBlockSize - ScaleY(2),
                    darkColor
                );
                
                DrawRectangle(
                    boardPos.x + j * scaledBlockSize + ScaleY(3), 
                    boardPos.y + i * scaledBlockSize + ScaleY(3), 
                    scaledBlockSize - ScaleY(6), scaledBlockSize - ScaleY(6),
                    blockColor
                );
                
                // Draw inner highlight/glow
                DrawRectangle(
                    boardPos.x + j * scaledBlockSize + ScaleY(5), 
                    boardPos.y + i * scaledBlockSize + ScaleY(5), 
                    scaledBlockSize - ScaleY(10), scaledBlockSize - ScaleY(10),
                    ColorAlpha(ColorBrightness(blockColor, 0.3f), blockGlow)
                );
                
                // Draw glow border
                DrawRectangleLines(
                    boardPos.x + j * scaledBlockSize, 
                    boardPos.y + i * scaledBlockSize, 
                    scaledBlockSize, scaledBlockSize,
                    ColorAlpha(glowColor, 0.3f * blockGlow)
                );
            }
        }
    }
    
    // Draw border with neon effect
    DrawRectangleLines(
        boardPos.x - 1, boardPos.y - 1,
        boardWidth + 2, boardHeight + 2,
        ColorAlpha(NEON_BLUE, 0.7f * glowIntensity)
    );
    
    // Draw animations
    DrawAnimations();
}

// Update the DrawTetromino function for smoother animations
void DrawTetromino(Tetromino t, int offsetX, int offsetY, float alpha) {
    float scaledBlockSize = ScaleY(BLOCK_SIZE);
    
    // Apply improved breathing and movement effects
    float breathEffect = 0.0f;
    float glowIntensity = 0.0f;
    
    if (&t == &game.current) {
        t.breathFactor += t.breathSpeed * GetFrameTime();
        breathEffect = sinf(t.breathFactor * 4.0f) * 0.07f;
        glowIntensity = sinf(t.breathFactor * 3.0f) * 0.3f + 0.7f;
    } else if (&t == &game.next || (&t == &game.held && game.hasHeld)) {
        t.breathFactor += t.breathSpeed * GetFrameTime() * 0.5f;
        breathEffect = sinf(t.breathFactor * 3.0f) * 0.05f;
        glowIntensity = sinf(t.breathFactor * 2.0f) * 0.2f + 0.5f;
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (t.shape[i][j]) {
                Color blockColor = ColorAlpha(t.color, alpha);
                Color darkColor = ColorAlpha(ColorBrightness(t.color, -0.4f), alpha);
                Color glowColor = ColorAlpha(t.color, alpha * 0.5f * glowIntensity);
                
                // Calculate position with smoother breathing effect
                float breathScale = 1.0f + breathEffect;
                int breathOffset = (int)((scaledBlockSize * breathScale - scaledBlockSize) / 2);
                
                if (&t == &game.current || &t == &game.next || (&t == &game.held && game.hasHeld)) {
                    // Draw glow effect first
                    if (glowIntensity > 0.5f) {
                        DrawRectangle(
                            ScaleX(offsetX) + (t.x + j) * scaledBlockSize - breathOffset - ScaleY(2), 
                            ScaleY(offsetY) + (t.y + i) * scaledBlockSize - breathOffset - ScaleY(2), 
                            (int)(scaledBlockSize * breathScale) + ScaleY(4), 
                            (int)(scaledBlockSize * breathScale) + ScaleY(4),
                            ColorAlpha(glowColor, 0.15f * glowIntensity)
                        );
                    }
                    
                    // Draw block with breathing effect
                    DrawRectangle(
                        ScaleX(offsetX) + (t.x + j) * scaledBlockSize - breathOffset, 
                        ScaleY(offsetY) + (t.y + i) * scaledBlockSize - breathOffset, 
                        (int)(scaledBlockSize * breathScale) - 1, 
                        (int)(scaledBlockSize * breathScale) - 1,
                        darkColor
                    );
                    
                    DrawRectangle(
                        ScaleX(offsetX) + (t.x + j) * scaledBlockSize + ScaleY(2) - breathOffset, 
                        ScaleY(offsetY) + (t.y + i) * scaledBlockSize + ScaleY(2) - breathOffset, 
                        (int)(scaledBlockSize * breathScale) - ScaleY(5), 
                        (int)(scaledBlockSize * breathScale) - ScaleY(5),
                        blockColor
                    );
                    
                    // Draw inner highlight with pulsing effect
                    DrawRectangle(
                        ScaleX(offsetX) + (t.x + j) * scaledBlockSize + ScaleY(4) - breathOffset, 
                        ScaleY(offsetY) + (t.y + i) * scaledBlockSize + ScaleY(4) - breathOffset, 
                        (int)(scaledBlockSize * breathScale) - ScaleY(9), 
                        (int)(scaledBlockSize * breathScale) - ScaleY(9),
                        ColorAlpha(ColorBrightness(t.color, 0.3f), alpha * glowIntensity)
                    );
                    
                    DrawRectangleLines(
                        ScaleX(offsetX) + (t.x + j) * scaledBlockSize - breathOffset - ScaleY(1),
                        ScaleY(offsetY) + (t.y + i) * scaledBlockSize - breathOffset - ScaleY(1),
                        (int)(scaledBlockSize * breathScale) + ScaleY(1),
                        (int)(scaledBlockSize * breathScale) + ScaleY(1),
                        ColorAlpha(glowColor, alpha * 0.7f * glowIntensity)
                    );

                } else {
                    // Simpler draw for ghost piece with subtle glow
                    DrawRectangle(
                        ScaleX(offsetX) + (t.x + j) * scaledBlockSize + ScaleY(1), 
                        ScaleY(offsetY) + (t.y + i) * scaledBlockSize + ScaleY(1),
                        scaledBlockSize - ScaleY(3), scaledBlockSize - ScaleY(3),
                        darkColor
                    );
                    
                    DrawRectangle(
                        ScaleX(offsetX) + (t.x + j) * scaledBlockSize + ScaleY(3), 
                        ScaleY(offsetY) + (t.y + i) * scaledBlockSize + ScaleY(3), 
                        scaledBlockSize - ScaleY(7), scaledBlockSize - ScaleY(7),
                        blockColor
                    );
                    
                    
                    // Draw subtle border glow for ghost piece
                    DrawRectangleLines(
                        ScaleX(offsetX) + (t.x + j) * scaledBlockSize,
                        ScaleY(offsetY) + (t.y + i) * scaledBlockSize,
                        scaledBlockSize - ScaleY(1), scaledBlockSize - ScaleY(1),
                        ColorAlpha(t.color, alpha * 0.5f)
                    );
                }
            }
        }
    }
}

// Draw the ghost piece
void DrawGhostPiece(Tetromino t) {
    t.y = GetDropPosition(t);
    DrawTetromino(t, BOARD_OFFSET_X, BOARD_OFFSET_Y, 0.3f);
}

// Handle user input
void HandleInput() {
    if (shouldExitGame) return;

    if (game.state == STATE_SPLASH) {
        return;
    }

    // Main menu input is handled separately
    if (game.state == STATE_MAIN_MENU) {
        return; // HandleMainMenuInput is called from UpdateGame
    }

    // ALT+A shortcut to toggle auto-play
    if (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) {
        if (IsKeyPressed(KEY_A)) {
            ToggleAutoPlay();
        }
    }

    // Check for auto-play settings first - ADD THIS SECTION
    if (game.showAutoPlaySettings) {
        HandleAutoPlaySettings();
        return;
    }
    
    // Disable regular controls when auto-play is active (except for ALT+A and pause)
    if (game.autoPlayEnabled && game.state == STATE_PLAYING) {
        if (IsKeyPressed(KEY_P)) {
            game.state = STATE_PAUSED;
        }
        return;
    }
    
    // Handle mouse input first
    HandleMouseInput();
    
    if (game.showAudioSettings) {
        // Audio settings menu navigation
        if (IsKeyPressed(KEY_UP)) {
            game.audioMenuSelection--;
            if (game.audioMenuSelection < 0) game.audioMenuSelection = 7;
            PlayGameSound(game.menuSound);
        }
        
        if (IsKeyPressed(KEY_DOWN)) {
            game.audioMenuSelection++;
            if (game.audioMenuSelection > 7) game.audioMenuSelection = 0;
            PlayGameSound(game.menuSound);
        }
        
        // Adjust values with left/right
        if (IsKeyDown(KEY_LEFT) || IsKeyPressed(KEY_LEFT)) {
            switch(game.audioMenuSelection) {
                case 0: // Master Volume
                    game.masterVolume = ClampValue(game.masterVolume - 0.01f, 0.0f, 1.0f);
                    break;
                case 1: // Music Volume
                    game.musicVolume = ClampValue(game.musicVolume - 0.01f, 0.0f, 1.0f);
                    break;
                case 2: // SFX Volume
                    game.sfxVolume = ClampValue(game.sfxVolume - 0.01f, 0.0f, 1.0f);
                    break;
                case 5: // SFX Pitch
                    game.sfxPitch = ClampValue(game.sfxPitch - 0.01f, 0.5f, 1.5f);
                    break;
                case 6: // SFX Pan
                    game.sfxPan = ClampValue(game.sfxPan - 0.01f, -1.0f, 1.0f);
                    break;
            }
        }
        
        if (IsKeyDown(KEY_RIGHT) || IsKeyPressed(KEY_RIGHT)) {
            switch(game.audioMenuSelection) {
                case 0: // Master Volume
                    game.masterVolume = ClampValue(game.masterVolume + 0.01f, 0.0f, 1.0f);
                    break;
                case 1: // Music Volume
                    game.musicVolume = ClampValue(game.musicVolume + 0.01f, 0.0f, 1.0f);
                    break;
                case 2: // SFX Volume
                    game.sfxVolume = ClampValue(game.sfxVolume +  0.01f, 0.0f, 1.0f);
                    break;
                case 5: // SFX Pitch
                    game.sfxPitch = ClampValue(game.sfxPitch + 0.01f, 0.5f, 1.5f);
                    break;
                case 6: // SFX Pan
                    game.sfxPan = ClampValue(game.sfxPan + 0.01f, -1.0f, 1.0f);
                    break;
            }
        }
        
        // Toggle options or exit menu
        if (IsKeyPressed(KEY_ENTER)) {
            PlayGameSound(game.menuSound);
            
            switch(game.audioMenuSelection) {
                case 3: // Music toggle
                    game.musicEnabled = !game.musicEnabled;
                    if (game.musicEnabled) {
                        PlayMusicStream(game.backgroundMusic);
                    } else {
                        StopMusicStream(game.backgroundMusic);
                    }
                    break;
                case 4: // Sound effects toggle
                    game.soundEnabled = !game.soundEnabled;
                    break;
                case 7: // Back
                    game.showAudioSettings = false;
                    break;
            }
        }
        
        // Exit menu with ESC
        if (IsKeyPressed(KEY_ESCAPE)) {
            game.showAudioSettings = false;
            PlayGameSound(game.menuSound);
        }
        
        return;
    }

    // Pause menu case for showing audio settings
    else if (game.state == STATE_PAUSED) {
        // Update menu to include audio settings option
        if (IsKeyPressed(KEY_DOWN)) {
            game.menuSelection++;
            if (game.menuSelection > 4) game.menuSelection = 0;  // 5 options
            PlayGameSound(game.menuSound);
        }
        
        if (IsKeyPressed(KEY_UP)) {
            game.menuSelection--;
            if (game.menuSelection < 0) game.menuSelection = 4;  // 5 options
            PlayGameSound(game.menuSound);
        }
        
        // Menu selection
        if (IsKeyPressed(KEY_ENTER)) {
            PlayGameSound(game.menuSound);
            
            switch (game.menuSelection) {
                case 0: // Resume
                    game.state = STATE_PLAYING;
                    break;
                case 1: // Restart
                    InitGame();
                    break;
                case 2: // Audio settings
                    game.showAudioSettings = true;
                    game.audioMenuSelection = 0;
                    break;
                case 3: // Auto-play settings
                    game.showAutoPlaySettings = true;
                    game.autoPlayMenuSelection = 0;
                    break;
                case 4: // Quit
                    shouldExitGame = true;
                    break;
            }
        }
    }
    
    // Add sound effects for movement
    if (game.state == STATE_PLAYING) {
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
            PlayGameSound(game.moveSound);
        }
        
        if (IsKeyPressed(KEY_UP)) {
            // Sound is now handled in the rotation logic
        }
        
        if (IsKeyPressed(KEY_C)) {
            // Play hold sound
            PlayGameSound(game.holdSound);
        }
        
        // The rest of your existing game controls...
        if (IsKeyPressed(KEY_P)) {
            game.state = STATE_PAUSED;
            return;
        }

        // For movement
        if (IsKeyPressed(KEY_LEFT) || (IsKeyDown(KEY_LEFT) && (GetTime() * 4) - (int)(GetTime() * 4) < 0.5)) {
            game.current.x--;
            if (CheckCollision(game.current)) {
                game.current.x++;
            }
        }

        if (IsKeyPressed(KEY_RIGHT) || (IsKeyDown(KEY_RIGHT) && (GetTime() * 4) - (int)(GetTime() * 4) < 0.5)) {
            game.current.x++;
            if (CheckCollision(game.current)) {
                game.current.x--;
            }
        }

        if (IsKeyPressed(KEY_UP)) {
            Tetromino oldT = game.current;
            RotateTetromino(&game.current);
            
            // Play rotate sound if rotation was successful
            if (memcmp(&oldT.shape, &game.current.shape, sizeof(oldT.shape)) != 0 && game.soundEnabled) {
                PlaySound(game.rotateSound);
            }
        }
        
        if (IsKeyDown(KEY_DOWN)) {
            // Smooth soft drop animation
            game.softDropY += GetFrameTime() * 15.0f;
            
            // Make sure we don't go beyond collision point
            Tetromino testDrop = game.current;
            testDrop.y = (int)game.softDropY + 1;
            
            if (CheckCollision(testDrop)) {
                game.softDropY = floor(game.softDropY);
            }
            
            game.current.y = (int)game.softDropY;
            game.timer += GetFrameTime() * 10; // Speed up when down is pressed
        } else {
            // Keep soft drop position in sync with current position when not pressing down
            game.softDropY = game.current.y;
        }
        
        // For hard drop
        if (IsKeyPressed(KEY_SPACE)) {
            // Hard drop
            game.current.y = GetDropPosition(game.current);
            MergeToGrid(game.current);
            
            // Play drop sound
            if (game.soundEnabled) {
                PlaySound(game.dropSound);
            }
            
            int linesCleared = ClearLines();
            
            // Update score
            if (linesCleared > 0) {
                int points = 0;
                const char* clearText = NULL;
                float textScale = 1.0f;
                Color textColor = YELLOW;
                
                // Increment combo
                game.combo++;
                
                // Calculate points with combo multiplier
                switch (linesCleared) {
                    case 1: 
                        points = 100 * game.level; 
                        clearText = "+100";
                        break;
                    case 2: 
                        points = 300 * game.level;
                        clearText = "+300";
                        textScale = 1.2f;
                        textColor = GREEN;
                        break;
                    case 3: 
                        points = 500 * game.level;
                        clearText = "+500";
                        textScale = 1.4f;
                        textColor = BLUE;
                        break;
                    case 4: 
                        points = 800 * game.level;
                        clearText = "TETRIS!";
                        textScale = 1.8f;
                        textColor = PURPLE;
                        
                        // Play special sound for Tetris
                        if (game.soundEnabled) {
                            PlaySound(game.tetrisSound);
                        }
                        break;
                }
                
                // Apply combo multiplier (starting from 2nd consecutive clear)
                if (game.combo > 1) {
                    float multiplier = 1.0f + (game.combo * 0.1f); // 10% increase per combo
                    points = (int)(points * multiplier);
                    
                    // Add combo text
                    char comboText[32];
                    sprintf(comboText, "COMBO x%d!", game.combo);
                    
                    Vector2 comboPos = {
                        BOARD_OFFSET_X + BOARD_WIDTH / 2,
                        BOARD_OFFSET_Y + BOARD_HEIGHT / 2 - 60
                    };
                    
                    AddFloatingText(comboText, comboPos, ORANGE, 1.5f);
                }
                
                // Add floating score text
                Vector2 textPos = {
                    BOARD_OFFSET_X + BOARD_WIDTH / 2,
                    BOARD_OFFSET_Y + BOARD_HEIGHT / 2 - 30
                };
                
                // For Tetris, make it more dramatic
                if (linesCleared == 4) {
                    textPos.y -= 20; // Position higher for emphasis
                }
                
                AddFloatingText(clearText, textPos, textColor, textScale);
                
                // Add points display
                char pointsText[32];
                sprintf(pointsText, "+%d", points);
                
                Vector2 pointsPos = {
                    BOARD_OFFSET_X + BOARD_WIDTH / 2,
                    BOARD_OFFSET_Y + BOARD_HEIGHT / 2
                };
                
                // Only show points separately if we have a special clear text
                if (linesCleared == 4) {
                    AddFloatingText(pointsText, pointsPos, YELLOW, 1.3f);
                }
                
                game.score += points;
                game.lines += linesCleared;
                
                // Play sound effect for line clear
                if (game.soundEnabled) {
                    PlayGameSound(game.clearSound);
                }
                
                // Level up every 10 lines
                int oldLevel = game.level;
                game.level = 1 + (game.lines / 10);
                
                // Play level up sound if level changed
                if (game.level > oldLevel) {
                    PlayGameSound(game.levelUpSound);
                }
                
                game.delay = 1.0f - (game.level - 1) * 0.1f;
                if (game.delay < 0.1f) game.delay = 0.1f;
                
                // Update high score if needed
                if (game.score > game.highScore) {
                    game.highScore = game.score;
                    // Save high score immediately
                    SaveHighScore(game.highScore);
                }
            } else {
                // Reset combo if no lines cleared
                game.combo = 0;
            }
            
            game.current = game.next;
            game.next = GetRandomTetromino();
            game.softDropY = game.current.y;
            game.canHold = true;
            
            // Check for game over
            if (CheckCollision(game.current)) {
                game.state = STATE_GAMEOVER;
                
                // Play game over sound
                if (game.soundEnabled) {
                    PlaySound(game.gameoverSound);
                }
                
                // Stop background music
                if (game.musicEnabled && IsMusicStreamPlaying(game.backgroundMusic)) {
                    StopMusicStream(game.backgroundMusic);
                }
            }
        }
        
        if (IsKeyPressed(KEY_C)) {
            HoldPiece();
        }
    } else if (game.state == STATE_GAMEOVER) {
        if (IsKeyPressed(KEY_ENTER)) {
            InitGame();
            game.state = STATE_PLAYING;
            if (game.musicEnabled && !IsMusicStreamPlaying(game.backgroundMusic)) {
                PlayMusicStream(game.backgroundMusic);
                SetMusicVolume(game.backgroundMusic, game.masterVolume * game.musicVolume);
            }
        }
    }
    
    // Update this part in your HandleInput function
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (game.showAudioSettings) {
            game.showAudioSettings = false;
            PlayGameSound(game.menuSound);
        } else if (game.state == STATE_PAUSED) {
            game.state = STATE_PLAYING;
        }
    }
}

// Update game logic
void UpdateGame() {
    // Update shader time parameters
    float currentTime = (float)GetTime();
    SetShaderValue(glowShader, glowTimeLocation, &currentTime, SHADER_UNIFORM_FLOAT);
    SetShaderValue(blockShader, blockTimeLocation, &currentTime, SHADER_UNIFORM_FLOAT);
    
    // Update particles, animations, and floating texts
    UpdateParticles();
    UpdateAnimations();
    UpdateFloatingTexts();

    // Update auto-play
    if (game.state == STATE_PLAYING) {
        UpdateAutoPlay();
    }

    // Handle splash screen timer and transition
    if (game.state == STATE_SPLASH) {
        splashTimer += GetFrameTime();
        
        // Allow skipping splash screen with any key
        if (splashTimer > 1.0f && (IsKeyPressed(0) || IsMouseButtonPressed(0))) {
            game.state = STATE_MAIN_MENU;
            PlayGameSound(game.menuSound);
        }
        
        // Auto-transition after 4 seconds
        if (splashTimer > 4.0f) {
            game.state = STATE_MAIN_MENU;
        }
        
        return;
    }
    
    // Handle main menu state
    if (game.state == STATE_MAIN_MENU) {
        if (game.showAudioSettings) {
            // Handle audio settings input
            if (IsKeyPressed(KEY_UP)) {
                game.audioMenuSelection--;
                if (game.audioMenuSelection < 0) game.audioMenuSelection = 7;
                PlayGameSound(game.menuSound);
            }
            
            if (IsKeyPressed(KEY_DOWN)) {
                game.audioMenuSelection++;
                if (game.audioMenuSelection > 7) game.audioMenuSelection = 0;
                PlayGameSound(game.menuSound);
            }
            
            // Adjust values with left/right
            if (IsKeyDown(KEY_LEFT) || IsKeyPressed(KEY_LEFT)) {
                switch(game.audioMenuSelection) {
                    case 0: // Master Volume
                        game.masterVolume = ClampValue(game.masterVolume - 0.01f, 0.0f, 1.0f);
                        break;
                    case 1: // Music Volume
                        game.musicVolume = ClampValue(game.musicVolume - 0.01f, 0.0f, 1.0f);
                        break;
                    case 2: // SFX Volume
                        game.sfxVolume = ClampValue(game.sfxVolume - 0.01f, 0.0f, 1.0f);
                        break;
                    case 5: // SFX Pitch
                        game.sfxPitch = ClampValue(game.sfxPitch - 0.01f, 0.5f, 1.5f);
                        break;
                    case 6: // SFX Pan
                        game.sfxPan = ClampValue(game.sfxPan - 0.01f, -1.0f, 1.0f);
                        break;
                }
            }
            
            if (IsKeyDown(KEY_RIGHT) || IsKeyPressed(KEY_RIGHT)) {
                switch(game.audioMenuSelection) {
                    case 0: // Master Volume
                        game.masterVolume = ClampValue(game.masterVolume + 0.01f, 0.0f, 1.0f);
                        break;
                    case 1: // Music Volume
                        game.musicVolume = ClampValue(game.musicVolume + 0.01f, 0.0f, 1.0f);
                        break;
                    case 2: // SFX Volume
                        game.sfxVolume = ClampValue(game.sfxVolume +  0.01f, 0.0f, 1.0f);
                        break;
                    case 5: // SFX Pitch
                        game.sfxPitch = ClampValue(game.sfxPitch + 0.01f, 0.5f, 1.5f);
                        break;
                    case 6: // SFX Pan
                        game.sfxPan = ClampValue(game.sfxPan + 0.01f, -1.0f, 1.0f);
                        break;
                }
            }
            
            // Toggle options or exit menu
            if (IsKeyPressed(KEY_ENTER)) {
                PlayGameSound(game.menuSound);
                
                switch(game.audioMenuSelection) {
                    case 3: // Music toggle
                        game.musicEnabled = !game.musicEnabled;
                        if (game.musicEnabled) {
                            PlayMusicStream(game.backgroundMusic);
                        } else {
                            StopMusicStream(game.backgroundMusic);
                        }
                        break;
                    case 4: // Sound effects toggle
                        game.soundEnabled = !game.soundEnabled;
                        break;
                    case 7: // Back
                        game.showAudioSettings = false;
                        break;
                }
            }
            
            // Exit menu with ESC
            if (IsKeyPressed(KEY_ESCAPE)) {
                game.showAudioSettings = false;
                PlayGameSound(game.menuSound);
            }
            
            // Handle mouse input for audio settings
            HandleMouseInput();
        } else if (game.showAutoPlaySettings) {
            // Handle auto-play settings input
            HandleAutoPlaySettings();
        } else {
            HandleMainMenuInput();
        }
        
        // Continue updating music in menu
        if (game.musicEnabled) {
            UpdateMusicStream(game.backgroundMusic);
            
            // Start music if not playing
            if (!IsMusicStreamPlaying(game.backgroundMusic)) {
                PlayMusicStream(game.backgroundMusic);
            }
            
            // Apply current volume settings
            SetMusicVolume(game.backgroundMusic, game.masterVolume * game.musicVolume);
        } else if (IsMusicStreamPlaying(game.backgroundMusic)) {
            StopMusicStream(game.backgroundMusic);
        }
        
        return;
    }

    // Only update music if not in game over state
    if (game.state != STATE_GAMEOVER) {
        // Update music stream
        if (game.musicEnabled) {
            UpdateMusicStream(game.backgroundMusic);
            
            // Start music if not playing
            if (!IsMusicStreamPlaying(game.backgroundMusic)) {
                PlayMusicStream(game.backgroundMusic);
            }
            
            // Apply current volume settings
            SetMusicVolume(game.backgroundMusic, game.masterVolume * game.musicVolume);
        } else if (IsMusicStreamPlaying(game.backgroundMusic)) {
            StopMusicStream(game.backgroundMusic);
        }
    }
    
    if (game.state != STATE_PLAYING) return;
    
    // Update tetromino breathing effect
    game.current.breathFactor += game.current.breathSpeed * GetFrameTime();
    if (game.hasHeld) {
        game.held.breathFactor += game.held.breathSpeed * GetFrameTime();
    }
    game.next.breathFactor += game.next.breathSpeed * GetFrameTime();
    
    game.timer += GetFrameTime();
    if (game.timer >= game.delay) {
        game.current.y++;
        game.softDropY = game.current.y;
        
        if (CheckCollision(game.current)) {
            game.current.y--;
            game.softDropY = game.current.y;
            MergeToGrid(game.current);
            int linesCleared = ClearLines();
            
            // Update score
            if (linesCleared > 0) {
                int points = 0;
                const char* clearText = NULL;
                float textScale = 1.0f;
                Color textColor = YELLOW;
                
                // Increment combo
                game.combo++;
                
                // Calculate points with combo multiplier
                switch (linesCleared) {
                    case 1: 
                        points = 100 * game.level; 
                        clearText = "+100";
                        break;
                    case 2: 
                        points = 300 * game.level;
                        clearText = "+300";
                        textScale = 1.2f;
                        textColor = GREEN;
                        break;
                    case 3: 
                        points = 500 * game.level;
                        clearText = "+500";
                        textScale = 1.4f;
                        textColor = BLUE;
                        break;
                    case 4: 
                        points = 800 * game.level;
                        clearText = "TETRIS!";
                        textScale = 1.8f;
                        textColor = PURPLE;
                        
                        // Play special sound for Tetris
                        if (game.soundEnabled) {
                            PlaySound(game.tetrisSound);
                        }
                        break;
                }
                
                // Apply combo multiplier (starting from 2nd consecutive clear)
                if (game.combo > 1) {
                    float multiplier = 1.0f + (game.combo * 0.1f); // 10% increase per combo
                    points = (int)(points * multiplier);
                    
                    // Add combo text
                    char comboText[32];
                    sprintf(comboText, "COMBO x%d!", game.combo);
                    
                    Vector2 comboPos = {
                        BOARD_OFFSET_X + BOARD_WIDTH / 2,
                        BOARD_OFFSET_Y + BOARD_HEIGHT / 2 - 60
                    };
                    
                    AddFloatingText(comboText, comboPos, ORANGE, 1.5f);
                }
                
                // Add floating score text
                Vector2 textPos = {
                    BOARD_OFFSET_X + BOARD_WIDTH / 2,
                    BOARD_OFFSET_Y + BOARD_HEIGHT / 2 - 30
                };
                
                // For Tetris, make it more dramatic
                if (linesCleared == 4) {
                    textPos.y -= 20; // Position higher for emphasis
                }
                
                AddFloatingText(clearText, textPos, textColor, textScale);
                
                // Add points display
                char pointsText[32];
                sprintf(pointsText, "+%d", points);
                
                Vector2 pointsPos = {
                    BOARD_OFFSET_X + BOARD_WIDTH / 2,
                    BOARD_OFFSET_Y + BOARD_HEIGHT / 2
                };
                
                // Only show points separately if we have a special clear text
                if (linesCleared == 4) {
                    AddFloatingText(pointsText, pointsPos, YELLOW, 1.3f);
                }
                
                game.score += points;
                game.lines += linesCleared;
                
                // Play sound effect for line clear
                if (game.soundEnabled) {
                    PlayGameSound(game.clearSound);
                }
                
                // Level up every 10 lines
                int oldLevel = game.level;
                game.level = 1 + (game.lines / 10);
                
                // Play level up sound if level changed
                if (game.level > oldLevel) {
                    PlayGameSound(game.levelUpSound);
                }
                
                game.delay = 1.0f - (game.level - 1) * 0.1f;
                if (game.delay < 0.1f) game.delay = 0.1f;
                
                // Update high score if needed
                if (game.score > game.highScore) {
                    game.highScore = game.score;
                    // Save high score immediately
                    SaveHighScore(game.highScore);
                }
            } else {
                // Reset combo if no lines cleared
                game.combo = 0;
            }
            
            game.current = game.next;
            game.next = GetRandomTetromino();
            game.softDropY = game.current.y;
            game.canHold = true;
            
            // Check for game over
            if (CheckCollision(game.current)) {
                game.state = STATE_GAMEOVER;
                
                // Play game over sound
                if (game.soundEnabled) {
                    PlaySound(game.gameoverSound);
                }
                
                // Stop background music
                if (game.musicEnabled && IsMusicStreamPlaying(game.backgroundMusic)) {
                    StopMusicStream(game.backgroundMusic);
                }
            }
        }
        
        game.timer = 0;
    }
    
    // Update music stream
    if (game.musicEnabled) {
        UpdateMusicStream(game.backgroundMusic);
        
        // Start music if not playing
        if (!IsMusicStreamPlaying(game.backgroundMusic)) {
            PlayMusicStream(game.backgroundMusic);
        }
        
        // Apply current volume settings
        SetMusicVolume(game.backgroundMusic, game.masterVolume * game.musicVolume);
    } else if (IsMusicStreamPlaying(game.backgroundMusic)) {
        StopMusicStream(game.backgroundMusic);
    }
}

// Draw the UI (score, level, next piece, etc.)
void DrawUI() {
    // Calculate panel positions based on board position
    int leftSidebarX = BOARD_OFFSET_X - SIDEBAR_WIDTH - PANEL_PADDING;
    int rightSidebarX = BOARD_OFFSET_X + BOARD_WIDTH + PANEL_PADDING;
    int panelWidth = SIDEBAR_WIDTH;
    int panelHeight = 80;
    int panelSpacing = 20;
    
    // Game title with cyberpunk style
    DrawRectangle(leftSidebarX, 30, panelWidth, 50, ColorAlpha(CYBER_BLACK, 0.8f));
    
    // Draw border glow
    float titleGlow = sinf(GetTime() * 2.0f) * 0.2f + 0.8f;
    DrawRectangleLines(leftSidebarX - 1, 30 - 1, panelWidth + 2, 50 + 2, 
                      ColorAlpha(NEON_PURPLE, 0.7f * titleGlow));
    
    // Title with special effect
    if (game.customFontLoaded) {
        Vector2 titleSize = MeasureTextEx(game.gameFont, "TETRIS", 40, 2);
        Vector2 titlePos = { leftSidebarX + (panelWidth - titleSize.x) / 2, 35 };
        
        // Draw title with glow effect
        DrawTextEx(game.gameFont, "TETRIS", 
                  (Vector2){ titlePos.x + 1, titlePos.y + 1 }, 
                  40, 2, ColorAlpha(NEON_PURPLE, 0.5f * titleGlow));
        
        DrawTextEx(game.gameFont, "TETRIS", titlePos, 40, 2, WHITE);
    } else {
        DrawText("TETRIS", leftSidebarX + (panelWidth - MeasureText("TETRIS", 40)) / 2, 35, 40, WHITE);
    }
    
    // ---- Left Sidebar Panels ----
    
    // Score panel with cyber styling
    DrawRectangle(leftSidebarX, 100, panelWidth, panelHeight, ColorAlpha(CYBER_BLACK, 0.8f));
    DrawRectangleLines(leftSidebarX, 100, panelWidth, panelHeight, ColorAlpha(NEON_BLUE, 0.7f));
    
    // Add diagonal lines for cyber effect
    int cornerSize = 15;
    DrawLineEx(
        (Vector2){leftSidebarX, 100 + cornerSize},
        (Vector2){leftSidebarX + cornerSize, 100},
        1.0f, ColorAlpha(NEON_BLUE, 0.9f)
    );
    DrawLineEx(
        (Vector2){leftSidebarX + panelWidth, 100 + cornerSize},
        (Vector2){leftSidebarX + panelWidth - cornerSize, 100},
        1.0f, ColorAlpha(NEON_BLUE, 0.9f)
    );
    
    // Score label with glow
    if (game.customFontLoaded) {
        DrawTextEx(game.gameFont, "SCORE", 
                  (Vector2){ leftSidebarX + 15, 110 }, 
                  24, 2, NEON_BLUE);
    } else {
        DrawText("SCORE", leftSidebarX + 15, 110, 24, NEON_BLUE);
    }
    
    // Score value with glow effect
    const char* scoreText = TextFormat("%d", game.score);
    float scoreGlow = sinf(GetTime() * 3.0f) * 0.2f + 0.8f;
    
    if (game.customFontLoaded) {
        Vector2 scoreSize = MeasureTextEx(game.gameFont, scoreText, 28, 2);
        Vector2 scorePos = { leftSidebarX + (panelWidth - scoreSize.x) / 2, 140 };
        
        // Draw with subtle glow
        DrawTextEx(game.gameFont, scoreText, 
                  (Vector2){ scorePos.x + 1, scorePos.y + 1 }, 
                  28, 2, ColorAlpha(NEON_YELLOW, 0.3f * scoreGlow));
                  
        DrawTextEx(game.gameFont, scoreText, scorePos, 28, 2, NEON_YELLOW);
    } else {
        DrawText(scoreText, 
                leftSidebarX + (panelWidth - MeasureText(scoreText, 28)) / 2, 
                140, 28, NEON_YELLOW);
    }
    
    // Level panel (using panelSpacing for vertical positioning)
    int levelY = 100 + panelHeight + panelSpacing;
    DrawRectangle(leftSidebarX, levelY, panelWidth, panelHeight, ColorAlpha(CYBER_BLACK, 0.8f));
    DrawRectangleLines(leftSidebarX, levelY, panelWidth, panelHeight, ColorAlpha(NEON_GREEN, 0.7f));
    
    // Add diagonal corner effect
    DrawLineEx(
        (Vector2){leftSidebarX, levelY + cornerSize},
        (Vector2){leftSidebarX + cornerSize, levelY},
        1.0f, ColorAlpha(NEON_GREEN, 0.9f)
    );
    DrawLineEx(
        (Vector2){leftSidebarX + panelWidth, levelY + cornerSize},
        (Vector2){leftSidebarX + panelWidth - cornerSize, levelY},
        1.0f, ColorAlpha(NEON_GREEN, 0.9f)
    );
    
    // Level label
    if (game.customFontLoaded) {
        DrawTextEx(game.gameFont, "LEVEL", 
                  (Vector2){ leftSidebarX + 15, levelY + 10 }, 
                  24, 2, NEON_GREEN);
    } else {
        DrawText("LEVEL", leftSidebarX + 15, levelY + 10, 24, NEON_GREEN);
    }
    
    // Level value with pulsing effect
    const char* levelText = TextFormat("%d", game.level);
    float levelPulse = sinf(GetTime() * 2.0f) * 0.2f + 0.8f;
    
    if (game.customFontLoaded) {
        Vector2 levelSize = MeasureTextEx(game.gameFont, levelText, 30, 2);
        Vector2 levelPos = { leftSidebarX + (panelWidth - levelSize.x) / 2, levelY + 40 };
        
        DrawTextEx(game.gameFont, levelText, levelPos, 30, 2, 
                  ColorAlpha(NEON_GREEN, levelPulse));
    } else {
        DrawText(levelText, 
                leftSidebarX + (panelWidth - MeasureText(levelText, 30)) / 2, 
                levelY + 40, 30, 
                ColorAlpha(NEON_GREEN, levelPulse));
    }
    
    // Lines panel (continue using panelSpacing)
    int linesY = levelY + panelHeight + panelSpacing;
    DrawRectangle(leftSidebarX, linesY, panelWidth, panelHeight, ColorAlpha(CYBER_BLACK, 0.8f));
    DrawRectangleLines(leftSidebarX, linesY, panelWidth, panelHeight, ColorAlpha(NEON_PINK, 0.7f));
    
    // Add diagonal corner effect
    DrawLineEx(
        (Vector2){leftSidebarX, linesY + cornerSize},
        (Vector2){leftSidebarX + cornerSize, linesY},
        1.0f, ColorAlpha(NEON_PINK, 0.9f)
    );
    DrawLineEx(
        (Vector2){leftSidebarX + panelWidth, linesY + cornerSize},
        (Vector2){leftSidebarX + panelWidth - cornerSize, linesY},
        1.0f, ColorAlpha(NEON_PINK, 0.9f)
    );
    
    // Lines label
    if (game.customFontLoaded) {
        DrawTextEx(game.gameFont, "LINES", 
                  (Vector2){ leftSidebarX + 15, linesY + 10 }, 
                  24, 2, NEON_PINK);
    } else {
        DrawText("LINES", leftSidebarX + 15, linesY + 10, 24, NEON_PINK);
    }
    
    // Lines value
    const char* linesText = TextFormat("%d", game.lines);
    
    if (game.customFontLoaded) {
        Vector2 linesSize = MeasureTextEx(game.gameFont, linesText, 28, 2);
        Vector2 linesPos = { leftSidebarX + (panelWidth - linesSize.x) / 2, linesY + 40 };
        
        DrawTextEx(game.gameFont, linesText, linesPos, 28, 2, NEON_PINK);
    } else {
        DrawText(linesText, 
                leftSidebarX + (panelWidth - MeasureText(linesText, 28)) / 2, 
                linesY + 40, 28, NEON_PINK);
    }
    
    // High Score panel (continue using panelSpacing)
    int highscoreY = linesY + panelHeight + panelSpacing;
    DrawRectangle(leftSidebarX, highscoreY, panelWidth, panelHeight, ColorAlpha(CYBER_BLACK, 0.8f));
    DrawRectangleLines(leftSidebarX, highscoreY, panelWidth, panelHeight, ColorAlpha(NEON_PURPLE, 0.7f));
    
    // Add diagonal corner effect
    DrawLineEx(
        (Vector2){leftSidebarX, highscoreY + cornerSize},
        (Vector2){leftSidebarX + cornerSize, highscoreY},
        1.0f, ColorAlpha(NEON_PURPLE, 0.9f)
    );
    DrawLineEx(
        (Vector2){leftSidebarX + panelWidth, highscoreY + cornerSize},
        (Vector2){leftSidebarX + panelWidth - cornerSize, highscoreY},
        1.0f, ColorAlpha(NEON_PURPLE, 0.9f)
    );
    
    // High Score label
    if (game.customFontLoaded) {
        DrawTextEx(game.gameFont, "HIGH SCORE", 
                  (Vector2){ leftSidebarX + 15, highscoreY + 10 }, 
                  20, 2, NEON_PURPLE);
    } else {
        DrawText("HIGH SCORE", leftSidebarX + 15, highscoreY + 10, 20, NEON_PURPLE);
    }
    
    // High Score value with special glow effect for new high score
    const char* highScoreText = TextFormat("%d", game.highScore);
    float highScoreGlow = 1.0f;
    Color highScoreColor = WHITE;
    
    // Make high score pulse when it equals current score (new high score)
    if (game.score == game.highScore && game.score > 0) {
        highScoreGlow = sinf(GetTime() * 4.0f) * 0.3f + 0.7f;
        highScoreColor = NEON_YELLOW;
    }
    
    if (game.customFontLoaded) {
        Vector2 highScoreSize = MeasureTextEx(game.gameFont, highScoreText, 28, 2);
        Vector2 highScorePos = { leftSidebarX + (panelWidth - highScoreSize.x) / 2, highscoreY + 40 };
        
        DrawTextEx(game.gameFont, highScoreText, 
                  (Vector2){ highScorePos.x + 1, highScorePos.y + 1 }, 
                  28, 2, ColorAlpha(DARKGRAY, 0.5f));
                  
        DrawTextEx(game.gameFont, highScoreText, 
                  highScorePos, 28, 2, 
                  ColorAlpha(highScoreColor, highScoreGlow));
    } else {
        DrawText(highScoreText, 
                leftSidebarX + (panelWidth - MeasureText(highScoreText, 28)) / 2, 
                highscoreY + 40, 28, 
                ColorAlpha(highScoreColor, highScoreGlow));
    }
    
    // ---- Right Sidebar Panels ----
    
    // Next piece panel
    int nextPieceY = 100;
    DrawRectangle(rightSidebarX, nextPieceY, panelWidth, PREVIEW_SIZE, ColorAlpha(CYBER_BLACK, 0.8f));
    DrawRectangleLines(rightSidebarX, nextPieceY, panelWidth, PREVIEW_SIZE, ColorAlpha(NEON_BLUE, 0.7f));
    
    // Add diagonal corner effect
    DrawLineEx(
        (Vector2){rightSidebarX, nextPieceY + cornerSize},
        (Vector2){rightSidebarX + cornerSize, nextPieceY},
        1.0f, ColorAlpha(NEON_BLUE, 0.9f)
    );
    DrawLineEx(
        (Vector2){rightSidebarX + panelWidth, nextPieceY + cornerSize},
        (Vector2){rightSidebarX + panelWidth - cornerSize, nextPieceY},
        1.0f, ColorAlpha(NEON_BLUE, 0.9f)
    );
    
    // Next piece label
    if (game.customFontLoaded) {
        DrawTextEx(game.gameFont, "NEXT", 
                  (Vector2){ rightSidebarX + 15, nextPieceY + 15 }, 
                  24, 2, NEON_BLUE);
    } else {
        DrawText("NEXT", rightSidebarX + 15, nextPieceY + 15, 24, NEON_BLUE);
    }
    
    // Draw next piece with correct positioning
    Tetromino nextPiecePreview = game.next;
    nextPiecePreview.x = 1;
    nextPiecePreview.y = 1;
    
    // Center smaller pieces (adjust based on actual piece)
    if (nextPiecePreview.type == O_PIECE) {
        nextPiecePreview.x = 1;
    } else if (nextPiecePreview.type == I_PIECE) {
        nextPiecePreview.x = 0;
        nextPiecePreview.y = 1;
    }
    
    DrawTetromino(nextPiecePreview, rightSidebarX + panelWidth/2 - BLOCK_SIZE*2, nextPieceY + 50, 1.0f);
    
    // Hold piece panel only if game allows holding
    int holdPieceY = nextPieceY + PREVIEW_SIZE + panelSpacing;
    DrawRectangle(rightSidebarX, holdPieceY, panelWidth, PREVIEW_SIZE, ColorAlpha(CYBER_BLACK, 0.8f));
    
    // Different color for hold panel based on whether hold is available
    Color holdBorderColor = game.canHold ? NEON_GREEN : ColorAlpha(NEON_GREEN, 0.3f);
    DrawRectangleLines(rightSidebarX, holdPieceY, panelWidth, PREVIEW_SIZE, holdBorderColor);
    
    // Add diagonal corner effect
    DrawLineEx(
        (Vector2){rightSidebarX, holdPieceY + cornerSize},
        (Vector2){rightSidebarX + cornerSize, holdPieceY},
        1.0f, ColorAlpha(holdBorderColor, 0.9f)
    );
    DrawLineEx(
        (Vector2){rightSidebarX + panelWidth, holdPieceY + cornerSize},
        (Vector2){rightSidebarX + panelWidth - cornerSize, holdPieceY},
        1.0f, ColorAlpha(holdBorderColor, 0.9f)
    );
    
    // Hold piece label with dimmed color if hold is not available
    Color holdTextColor = game.canHold ? NEON_GREEN : ColorAlpha(NEON_GREEN, 0.5f);
    if (game.customFontLoaded) {
        DrawTextEx(game.gameFont, "HOLD", 
                  (Vector2){ rightSidebarX + 15, holdPieceY + 15 }, 
                  24, 2, holdTextColor);
    } else {
        DrawText("HOLD", rightSidebarX + 15, holdPieceY + 15, 24, holdTextColor);
    }
    
    // Draw held piece if there is one
    if (game.hasHeld) {
        Tetromino heldPiecePreview = game.held;
        heldPiecePreview.x = 1;
        heldPiecePreview.y = 1;
        
        // Center smaller pieces (adjust based on actual piece)
        if (heldPiecePreview.type == O_PIECE) {
            heldPiecePreview.x = 1;
        } else if (heldPiecePreview.type == I_PIECE) {
            heldPiecePreview.x = 0;
            heldPiecePreview.y = 1;
        }
        
        // Draw with dimmed appearance if hold is not available
        float holdAlpha = game.canHold ? 1.0f : 0.5f;
        DrawTetromino(heldPiecePreview, rightSidebarX + panelWidth/2 - BLOCK_SIZE*2, holdPieceY + 50, holdAlpha);
    } else {
        // Draw "empty" indicator or text
        const char* emptyText = "EMPTY";
        Color emptyColor = ColorAlpha(GRAY, 0.5f);
        
        if (game.customFontLoaded) {
            Vector2 emptySize = MeasureTextEx(game.gameFont, emptyText, 20, 2);
            DrawTextEx(game.gameFont, emptyText, 
                      (Vector2){ rightSidebarX + (panelWidth - emptySize.x)/2, holdPieceY + 80 }, 
                      20, 2, emptyColor);
        } else {
            DrawText(emptyText, 
                    rightSidebarX + (panelWidth - MeasureText(emptyText, 20))/2, 
                    holdPieceY + 80, 20, emptyColor);
        }
    }
    
    // Controls hint panel
    int controlsY = holdPieceY + PREVIEW_SIZE + panelSpacing;
    DrawRectangle(rightSidebarX, controlsY, panelWidth, 140, ColorAlpha(CYBER_BLACK, 0.7f));
    DrawRectangleLines(rightSidebarX, controlsY, panelWidth, 140, ColorAlpha(LIGHTGRAY, 0.5f));
    
    // Controls label
    if (game.customFontLoaded) {
        DrawTextEx(game.gameFont, "CONTROLS", 
                  (Vector2){ rightSidebarX + 15, controlsY + 10 }, 
                  20, 2, LIGHTGRAY);
    } else {
        DrawText("CONTROLS", rightSidebarX + 15, controlsY + 10, 20, LIGHTGRAY);
    }
    
    // Draw controls hints
    const char* controlsText[] = {
        "← → : MOVE",
        "↑ : ROTATE",
        "↓ : SOFT DROP",
        "SPACE : HARD DROP",
        "C : HOLD",
        "P : PAUSE"
    };
    
    for (int i = 0; i < 6; i++) {
        DrawText(controlsText[i], 
                rightSidebarX + 20, 
                controlsY + 40 + i * 16, 
                14, ColorAlpha(WHITE, 0.8f));
    }
}

// Draw the game
void DrawGame() {
    BeginDrawing();
    
    if (game.state == STATE_SPLASH) {
        DrawSplashScreen();
    } else if (game.state == STATE_MAIN_MENU) {
        DrawMainMenu();
        
        // Draw audio settings if active
        if (game.showAudioSettings) {
            DrawAudioSettings();
        }

        if (game.showAutoPlaySettings) {
            DrawAutoPlaySettings();
        }
        
        DrawFloatingTexts(); // For "How to Play" messages
    } else {
        // Dynamic cyberpunk background with grid
        DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 
                            CYBER_BLACK, 
                            CYBER_DARKBLUE);

        // Add cyber grid
        float gridTime = GetTime() * 0.2f;
        int gridSize = 60;
        float gridAlpha = 0.15f;

        // Horizontal grid lines with movement effect
        for(int i = 0; i < SCREEN_HEIGHT / gridSize + 1; i++) {
            float y = (float)i * gridSize + fmodf(gridTime * 30, gridSize);
            float alpha = gridAlpha * (1.0f - (y / SCREEN_HEIGHT) * 0.8f);
            DrawLineEx(
                (Vector2){0, y},
                (Vector2){SCREEN_WIDTH, y},
                1.0f,
                ColorAlpha(NEON_BLUE, alpha)
            );
        }

        // Vertical grid lines
        for(int i = 0; i < SCREEN_WIDTH / gridSize + 1; i++) {
            float x = (float)i * gridSize;
            DrawLineEx(
                (Vector2){x, 0},
                (Vector2){x, SCREEN_HEIGHT},
                1.0f,
                ColorAlpha(NEON_BLUE, gridAlpha * 0.5f)
            );
        }

        // Add subtle scanline effect
        for(int i = 0; i < SCREEN_HEIGHT; i += 4) {
            DrawRectangle(0, i, SCREEN_WIDTH, 1, ColorAlpha(BLACK, 0.07f));
        }
        
        DrawTetrisGrid();
    
        if (game.state == STATE_PLAYING) {
            DrawGhostPiece(game.current);
            DrawTetromino(game.current, BOARD_OFFSET_X, BOARD_OFFSET_Y, 1.0f);
            // Draw auto-play indicator when active
            DrawAutoPlayIndicator();
        }
        
        DrawUI();
        DrawParticles();
        DrawFloatingTexts();
        
        // Draw pause menu when game is paused
        if (game.state == STATE_PAUSED) {
            DrawPauseMenu();
        }
        
        // Draw game over screen
        if (game.state == STATE_GAMEOVER) {
            DrawGameOverScreen();
        }
    
        // Draw audio settings if active
        if (game.showAudioSettings) {
            DrawAudioSettings();
        }

        // Draw auto-play settings if active
        if (game.showAutoPlaySettings) {
            DrawAutoPlaySettings();
        }

    }
    
    DrawMouseCursor();
    
    EndDrawing();
}

// Function to handle the pause menu
void DrawPauseMenu() {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha(BLACK, 0.7f));
    
    // Pause menu with gradient
    int menuWidth = 450;
    int menuHeight = 450;
    DrawRectangleGradientV(
        SCREEN_WIDTH/2 - menuWidth/2, SCREEN_HEIGHT/2 - menuHeight/2,
        menuWidth, menuHeight,
        ColorAlpha(DARKBLUE, 0.8f),
        ColorAlpha(DARKPURPLE, 0.8f)
    );
    DrawRectangleLines(
        SCREEN_WIDTH/2 - menuWidth/2, SCREEN_HEIGHT/2 - menuHeight/2,
        menuWidth, menuHeight,
        WHITE
    );
    
    // Add subtle pulsing glow around menu
    float pulseScale = sinf(GetTime() * 2.0f) * 0.5f + 0.5f;
    DrawRectangleLines(
        SCREEN_WIDTH/2 - menuWidth/2 - 5, SCREEN_HEIGHT/2 - menuHeight/2 - 5,
        menuWidth + 10, menuHeight + 10,
        ColorAlpha(SKYBLUE, pulseScale * 0.7f)
    );
    
    // Center text in pause menu
    DrawText("PAUSED", 
             SCREEN_WIDTH/2 - MeasureText("PAUSED", 50)/2, 
             SCREEN_HEIGHT/2 - 180, 50, WHITE);
    
    // Update menu items to include audio settings
    const char* menuItems[5] = {
        "RESUME",
        "RESTART",
        "AUDIO SETTINGS",
        "AUTO-PLAY SETTINGS",
        "EXIT GAME"
    };

    // Calculate proper vertical spacing for menu items
    int menuItemHeight = 50;
    int totalMenuHeight = menuItemHeight * 5;
    int startY = SCREEN_HEIGHT/2 - totalMenuHeight/2;
    
    // Draw menu items
    for (int i = 0; i < 5; i++) {
        // Calculate item position
        int itemY = startY + i * menuItemHeight;
        
        Color itemColor = (i == game.menuSelection) ? YELLOW : RAYWHITE;
        float scale = (i == game.menuSelection) ? 1.1f + sinf(GetTime() * 5.0f) * 0.05f : 1.0f;
        
        // Show selection indicator with animation
        if (i == game.menuSelection) {
            float arrowOffset = sinf(GetTime() * 5.0f) * 5.0f;
            DrawText(">", 
                     SCREEN_WIDTH/2 - MeasureText(menuItems[i], 30 * scale)/2 - 30 - arrowOffset, 
                     itemY, 
                     30 * scale, itemColor);
            
            DrawText("<", 
                     SCREEN_WIDTH/2 + MeasureText(menuItems[i], 30 * scale)/2 + 10 + arrowOffset, 
                     itemY, 
                     30 * scale, itemColor);
        }
        
        DrawText(menuItems[i], 
            SCREEN_WIDTH/2 - MeasureText(menuItems[i], 30 * scale)/2, 
            itemY, 
            30 * scale, itemColor);
    }
    
    // Instructions
    DrawText("Use UP/DOWN to navigate", 
             SCREEN_WIDTH/2 - MeasureText("Use UP/DOWN to navigate", 15)/2, 
             SCREEN_HEIGHT/2 + 170, 15, LIGHTGRAY);
    
    DrawText("ENTER to select", 
             SCREEN_WIDTH/2 - MeasureText("ENTER to select", 15)/2, 
             SCREEN_HEIGHT/2 + 190, 15, LIGHTGRAY);
}

// Function to draw audio settings menu
void DrawAudioSettings() {
    int menuWidth = 550;
    int menuHeight = 470;
    
    // Draw background overlay with blur effect simulation
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha(BLACK, 0.85f));
    
    // Draw menu panel with improved gradient and rounded corners effect
    DrawRectangleGradientV(
        SCREEN_WIDTH/2 - menuWidth/2, SCREEN_HEIGHT/2 - menuHeight/2,
        menuWidth, menuHeight,
        ColorAlpha((Color){40, 45, 80, 255}, 0.95f),
        ColorAlpha((Color){25, 25, 50, 255}, 0.95f)
    );
    
    // Draw an inner border for modern look
    DrawRectangleLines(
        SCREEN_WIDTH/2 - menuWidth/2 + 2, SCREEN_HEIGHT/2 - menuHeight/2 + 2,
        menuWidth - 4, menuHeight - 4,
        ColorAlpha(SKYBLUE, 0.6f)
    );
    
    // Draw outer glow border
    float glowPulse = sinf(GetTime() * 1.5f) * 0.5f + 0.5f;
    DrawRectangleLines(
        SCREEN_WIDTH/2 - menuWidth/2 - 1, SCREEN_HEIGHT/2 - menuHeight/2 - 1,
        menuWidth + 2, menuHeight + 2,
        ColorAlpha(BLUE, 0.3f + 0.2f * glowPulse)
    );
    
    // Title with underline
    int titleY = SCREEN_HEIGHT/2 - menuHeight/2 + 25;
    DrawText("AUDIO SETTINGS", 
             SCREEN_WIDTH/2 - MeasureText("AUDIO SETTINGS", 30)/2, 
             titleY, 30, WHITE);
             
    // Draw title underline with gradient
    DrawRectangleGradientH(
        SCREEN_WIDTH/2 - menuWidth/2 + 50,
        titleY + 40,
        menuWidth - 100,
        2,
        ColorAlpha(SKYBLUE, 0.7f),
        ColorAlpha(DARKBLUE, 0.1f)
    );
    
    // Menu items and values with better alignment
    const char* menuItems[] = {
        "Master Vol",
        "Music Vol",
        "SFX Vol",
        "Music",
        "Sound",
        "SFX Pitch",
        "SFX Pan",
        "Back"
    };
    
    int startY = SCREEN_HEIGHT/2 - menuHeight/2 + 90;
    int itemHeight = 42;
    int labelX = SCREEN_WIDTH/2 - menuWidth/2 + 50;
    int controlX = SCREEN_WIDTH/2 - 80;
    int controlWidth = 230;
    
    for (int i = 0; i < 8; i++) {
        Color itemColor = (i == game.audioMenuSelection) ? 
                          ColorAlpha(GOLD, 0.9f + sinf(GetTime() * 4.0f) * 0.1f) : 
                          ColorAlpha(WHITE, 0.85f);
        
        // Draw item background for selected item
        if (i == game.audioMenuSelection) {
            DrawRectangle(
                SCREEN_WIDTH/2 - menuWidth/2 + 10,
                startY + i * itemHeight - 8,
                menuWidth - 20,
                itemHeight,
                ColorAlpha(WHITE, 0.07f)
            );
        }
        
        // Draw menu item text with shadow effect for selected item
        if (i == game.audioMenuSelection) {
            DrawText(menuItems[i], 
                    labelX + 1, 
                    startY + i * itemHeight + 1, 
                    22, ColorAlpha(BLACK, 0.5f));
        }
        
        DrawText(menuItems[i], 
                labelX, 
                startY + i * itemHeight, 
                22, itemColor);
        
        // Draw control for each setting
        switch(i) {
            case 0: // Master Volume
            case 1: // Music Volume
            case 2: // SFX Volume
                {
                    float value = 0;
                    Color barColor = WHITE;
                    if (i == 0) { value = game.masterVolume; barColor = WHITE; }
                    else if (i == 1) { value = game.musicVolume; barColor = SKYBLUE; }
                    else if (i == 2) { value = game.sfxVolume; barColor = GREEN; }
                    
                    // Slider background with rounded edges effect
                    DrawRectangle(controlX, startY + i * itemHeight + 8, controlWidth, 8, ColorAlpha(DARKGRAY, 0.5f));
                    
                    // Slider fill
                    DrawRectangle(controlX, startY + i * itemHeight + 8, (int)(value * controlWidth), 8, barColor);
                    
                    // Slider handle
                    DrawCircle(controlX + (int)(value * controlWidth), startY + i * itemHeight + 12, 8, 
                              (i == game.audioMenuSelection) ? YELLOW : barColor);
                    
                    // Value text
                    DrawText(TextFormat("%.0f%%", value * 100), 
                            controlX + controlWidth + 15, 
                            startY + i * itemHeight, 22, itemColor);
                }
                break;
                
            case 3: // Music toggle
            case 4: // Sound effects toggle
                {
                    bool enabled = (i == 3) ? game.musicEnabled : game.soundEnabled;
                    Color toggleColor = enabled ? GREEN : MAROON;
                    
                    // Draw toggle background
                    DrawRectangle(controlX, startY + i * itemHeight + 4, 60, 24, ColorAlpha(DARKGRAY, 0.3f));
                    
                    // Draw toggle state
                    DrawRectangle(
                        controlX + (enabled ? 30 : 0), 
                        startY + i * itemHeight + 4, 
                        30, 24, 
                        toggleColor
                    );
                    
                    // Draw label
                    DrawText(enabled ? "ON" : "OFF", 
                            controlX + 70, 
                            startY + i * itemHeight, 22,
                            enabled ? GREEN : ColorAlpha(RED, 0.8f));
                }
                break;
                
            case 5: // SFX Pitch
                {
                    // Map pitch from 0.5-1.5 to 0-1 for drawing
                    float normalizedValue = (game.sfxPitch - 0.5f);
                    
                    // Slider background
                    DrawRectangle(controlX, startY + i * itemHeight + 8, controlWidth, 8, ColorAlpha(DARKGRAY, 0.5f));
                    
                    // Center mark
                    DrawRectangle(controlX + controlWidth/2 - 1, startY + i * itemHeight + 6, 2, 12, ColorAlpha(WHITE, 0.6f));
                    
                    // Slider handle
                    DrawCircle(controlX + (int)(normalizedValue * controlWidth), startY + i * itemHeight + 12, 8, 
                              (i == game.audioMenuSelection) ? YELLOW : ORANGE);
                    
                    // Value text
                    DrawText(TextFormat("%.1f", game.sfxPitch), 
                            controlX + controlWidth + 15, 
                            startY + i * itemHeight, 22, itemColor);
                }
                break;
                
            case 6: // SFX Pan
                {
                    // Map pan from -1.0-1.0 to 0-1 for drawing
                    float normalizedValue = (game.sfxPan + 1.0f) / 2.0f;
                    
                    // Slider background
                    DrawRectangle(controlX, startY + i * itemHeight + 8, controlWidth, 8, ColorAlpha(DARKGRAY, 0.5f));
                    
                    // Center mark
                    DrawRectangle(controlX + controlWidth/2 - 1, startY + i * itemHeight + 6, 2, 12, ColorAlpha(WHITE, 0.6f));
                    
                    // Slider handle
                    DrawCircle(controlX + (int)(normalizedValue * controlWidth), startY + i * itemHeight + 12, 8, 
                              (i == game.audioMenuSelection) ? YELLOW : PURPLE);
                    
                    // Value text with L/R indicators
                    DrawText(TextFormat("%.1f", game.sfxPan), 
                            controlX + controlWidth + 15, 
                            startY + i * itemHeight, 22, itemColor);
                    
                    // Show L/R indicators
                    DrawText("L", controlX - 20, startY + i * itemHeight, 20, ColorAlpha(WHITE, 0.7f));
                    DrawText("R", controlX + controlWidth + 5, startY + i * itemHeight, 20, ColorAlpha(WHITE, 0.7f));
                }
                break;
                
            case 7: // Back button
                {
                    if (i == game.audioMenuSelection) {
                        // Draw button with highlight effect
                        DrawRectangleGradientH(
                            controlX - 20,
                            startY + i * itemHeight - 2,
                            100,
                            30,
                            ColorAlpha(BLUE, 0.5f),
                            ColorAlpha(DARKBLUE, 0.3f)
                        );
                        
                        // Button text
                        DrawText("BACK", 
                                controlX + 10, 
                                startY + i * itemHeight, 22, RAYWHITE);
                    }
                }
                break;
        }
    }
    
    // Instructions with modern styling
    int instructionsY = SCREEN_HEIGHT/2 + menuHeight/2 - 60;
    
    // Instructions container
    DrawRectangle(
        SCREEN_WIDTH/2 - 200,
        instructionsY - 5,
        400,
        50,
        ColorAlpha(BLACK, 0.3f)
    );
    
    // In DrawAudioSettings function
    DrawText("", SCREEN_WIDTH/2 - 175, instructionsY, 20, ColorAlpha(LIGHTGRAY, 0.8f));
    DrawArrow("←", SCREEN_WIDTH/2 - 175, instructionsY, 20, ColorAlpha(LIGHTGRAY, 0.8f));
    DrawArrow("→", SCREEN_WIDTH/2 - 165, instructionsY, 20, ColorAlpha(LIGHTGRAY, 0.8f));
    DrawText(" to adjust", SCREEN_WIDTH/2 - 145, instructionsY, 20, ColorAlpha(LIGHTGRAY, 0.8f));
             
    // Draw keyboard indicators
    DrawRectangle(SCREEN_WIDTH/2 - 195, instructionsY, 15, 15, ColorAlpha(WHITE, 0.5f));
    DrawArrow("←", SCREEN_WIDTH/2 - 193, instructionsY - 2, 20, BLACK);
    
    DrawRectangle(SCREEN_WIDTH/2 - 175 + MeasureText("← → to adjust", 20) + 5, instructionsY, 15, 15, ColorAlpha(WHITE, 0.5f));
    DrawArrow("→", SCREEN_WIDTH/2 - 175 + MeasureText("← → to adjust", 20) + 5, instructionsY - 2, 20, BLACK);
    
    DrawRectangle(SCREEN_WIDTH/2 - 195, instructionsY + 25, 15, 15, ColorAlpha(WHITE, 0.5f));
    DrawArrow("↵", SCREEN_WIDTH/2 - 193, instructionsY + 23, 20, BLACK);
}

// Utility function
bool IsMouseOverRect(Rectangle rect) {
    Vector2 mousePoint = GetMousePosition();
    return CheckCollisionPointRec(mousePoint, rect);
}

// Function to handle mouse input
void HandleMouseInput() {
    Vector2 mousePoint = GetMousePosition();
    bool mouseClicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    // Cache frequently used rectangles
    static Rectangle cachedRects[25]; // Increased size to store more rectangles
    
    // Define rectangle indices for better readability
    enum CachedRectIndices {
        // Auto-play settings rectangles
        RECT_AP_BACK_BUTTON,
        RECT_AP_TOGGLE,
        RECT_AP_LEFT_ARROW,
        RECT_AP_RIGHT_ARROW,
        RECT_AP_DELAY_SLIDER,
        RECT_AP_ITEM_1,
        RECT_AP_ITEM_2,
        RECT_AP_ITEM_3,
        
        // Audio settings rectangles
        RECT_AUDIO_BACK_BUTTON,
        RECT_AUDIO_SLIDER_1,
        RECT_AUDIO_SLIDER_2,
        RECT_AUDIO_SLIDER_3,
        RECT_AUDIO_TOGGLE_MUSIC,
        RECT_AUDIO_TOGGLE_SOUND,
        RECT_AUDIO_SLIDER_PITCH,
        RECT_AUDIO_SLIDER_PAN,
        
        // Pause menu rectangles
        RECT_PAUSE_ITEM_1,
        RECT_PAUSE_ITEM_2,
        RECT_PAUSE_ITEM_3,
        RECT_PAUSE_ITEM_4,
        RECT_PAUSE_ITEM_5,
        
        // Game over screen
        RECT_PLAY_AGAIN
    };
    
    // Only calculate rectangles once per frame or when UI changes
    static bool rectsInitialized = false;
    static GameState lastState = STATE_SPLASH;
    static bool lastShowAudioSettings = false;
    static bool lastShowAutoPlaySettings = false;
    
    // Recalculate when state changes or settings visibility changes
    bool needsUpdate = !rectsInitialized 
                      || lastState != game.state
                      || lastShowAudioSettings != game.showAudioSettings
                      || lastShowAutoPlaySettings != game.showAutoPlaySettings;
    
    if (needsUpdate) {
        // Calculate and cache rectangles based on current state
        
        // Auto-play settings rectangles
        if (game.showAutoPlaySettings) {
            int menuWidth = ScaleX(550);
            int menuHeight = ScaleY(470);
            int startY = GetScreenHeight()/2 - menuHeight/2 + ScaleY(90);
            int itemHeight = ScaleY(42);
            int controlX = GetScreenWidth()/2 - ScaleX(80);
            int controlWidth = ScaleX(230);
            
            // Cache back button
            cachedRects[RECT_AP_BACK_BUTTON] = (Rectangle){
                (float)(controlX - ScaleX(20)), 
                (float)(startY + 7 * itemHeight - ScaleY(2)), 
                (float)ScaleX(100), (float)ScaleY(30)
            };
            
            // Cache toggle control
            cachedRects[RECT_AP_TOGGLE] = (Rectangle){
                (float)controlX, 
                (float)(startY + 0 * itemHeight + 4), 
                60.0f, 24.0f
            };
            
            // Cache left arrow
            cachedRects[RECT_AP_LEFT_ARROW] = (Rectangle){
                (float)(controlX - 25),
                (float)(startY + itemHeight),
                20.0f, 22.0f
            };
            
            // Cache right arrow (needs calculation of label width)
            const char* difficultyLabels[] = {"BEGINNER", "MEDIUM", "EXPERT"};
            cachedRects[RECT_AP_RIGHT_ARROW] = (Rectangle){
                (float)(controlX + MeasureText(difficultyLabels[game.autoPlayLevel], 22) + 10),
                (float)(startY + itemHeight),
                20.0f, 22.0f
            };
            
            // Cache delay slider
            cachedRects[RECT_AP_DELAY_SLIDER] = (Rectangle){
                (float)controlX,
                (float)(startY + 2 * itemHeight + 3),
                (float)controlWidth,
                20.0f
            };
            
            // Cache menu item selection areas
            for (int i = 0; i < 3; i++) {
                cachedRects[RECT_AP_ITEM_1 + i] = (Rectangle){
                    (float)(SCREEN_WIDTH/2 - menuWidth/2 + 10),
                    (float)(startY + i * itemHeight - 8),
                    (float)(menuWidth - 20),
                    (float)itemHeight
                };
            }
        }
        
        // Audio settings rectangles
        if (game.showAudioSettings) {
            int startY = SCREEN_HEIGHT/2 - 150;
            int itemHeight = 42;
            
            // Cache back button
            cachedRects[RECT_AUDIO_BACK_BUTTON] = (Rectangle){
                SCREEN_WIDTH/2 - 100, 
                startY + (7 * itemHeight), 
                100, 30
            };
            
            // Cache volume sliders
            for (int i = 0; i < 3; i++) {
                cachedRects[RECT_AUDIO_SLIDER_1 + i] = (Rectangle){
                    SCREEN_WIDTH/2 - 80,
                    startY + (i * itemHeight) + 8,
                    230, 20
                };
            }
            
            // Cache toggle buttons
            cachedRects[RECT_AUDIO_TOGGLE_MUSIC] = (Rectangle){
                SCREEN_WIDTH/2 - 80,
                startY + (3 * itemHeight) + 4,
                60, 24
            };
            
            cachedRects[RECT_AUDIO_TOGGLE_SOUND] = (Rectangle){
                SCREEN_WIDTH/2 - 80,
                startY + (4 * itemHeight) + 4,
                60, 24
            };
            
            // Cache pitch and pan sliders
            cachedRects[RECT_AUDIO_SLIDER_PITCH] = (Rectangle){
                SCREEN_WIDTH/2 - 80,
                startY + (5 * itemHeight) + 8,
                230, 20
            };
            
            cachedRects[RECT_AUDIO_SLIDER_PAN] = (Rectangle){
                SCREEN_WIDTH/2 - 80,
                startY + (6 * itemHeight) + 8,
                230, 20
            };
        }
        
        // Pause menu rectangles
        if (game.state == STATE_PAUSED) {
            int menuItemHeight = 50;
            int totalMenuHeight = menuItemHeight * 5;
            int startY = SCREEN_HEIGHT/2 - totalMenuHeight/2;
            
            for (int i = 0; i < 5; i++) {
                int itemY = startY + i * menuItemHeight;
                
                cachedRects[RECT_PAUSE_ITEM_1 + i] = (Rectangle){
                    SCREEN_WIDTH/2 - 100,
                    itemY - 10,  // Add padding above text
                    200, 40
                };
            }
        }
        
        // Game over screen
        if (game.state == STATE_GAMEOVER) {
            cachedRects[RECT_PLAY_AGAIN] = (Rectangle){
                SCREEN_WIDTH/2 - 150,
                SCREEN_HEIGHT/2 + 70,
                300, 40
            };
        }
        
        // Update tracking variables
        rectsInitialized = true;
        lastState = game.state;
        lastShowAudioSettings = game.showAudioSettings;
        lastShowAutoPlaySettings = game.showAutoPlaySettings;
    }

    // Now use the cached rectangles based on current state
    
    if (game.showAutoPlaySettings) {
        // Use cached back button rectangle
        if (IsMouseOverRect(cachedRects[RECT_AP_BACK_BUTTON])) {
            if (game.autoPlayMenuSelection != 3) {
                game.autoPlayMenuSelection = 3;
                PlayGameSound(game.menuSound);
            }
            
            if (mouseClicked) {
                game.showAutoPlaySettings = false;
                PlayGameSound(game.menuSound);
            }
        }
        
        // Check item selection using cached rectangles
        for (int i = 0; i < 3; i++) {
            if (IsMouseOverRect(cachedRects[RECT_AP_ITEM_1 + i])) {
                if (game.autoPlayMenuSelection != i) {
                    game.autoPlayMenuSelection = i;
                    PlayGameSound(game.menuSound);
                }
                
                if (mouseClicked && i == 0) {
                    game.autoPlayEnabled = !game.autoPlayEnabled;
                    PlayGameSound(game.menuSound);
                }
            }
        }
        
        // Use cached toggle rectangle
        if (IsMouseOverRect(cachedRects[RECT_AP_TOGGLE]) && mouseClicked) {
            game.autoPlayEnabled = !game.autoPlayEnabled;
            PlayGameSound(game.menuSound);
        }
        
        // Use cached difficulty arrows
        if (game.autoPlayLevel > 0) {
            if (IsMouseOverRect(cachedRects[RECT_AP_LEFT_ARROW]) && mouseClicked) {
                game.autoPlayLevel--;
                PlayGameSound(game.menuSound);
            }
        }
        
        if (game.autoPlayLevel < AI_LEVELS - 1) {
            if (IsMouseOverRect(cachedRects[RECT_AP_RIGHT_ARROW]) && mouseClicked) {
                game.autoPlayLevel++;
                PlayGameSound(game.menuSound);
            }
        }
        
        // Use cached delay slider
        if (IsMouseOverRect(cachedRects[RECT_AP_DELAY_SLIDER]) && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            float normalizedValue = (mousePoint.x - cachedRects[RECT_AP_DELAY_SLIDER].x) / cachedRects[RECT_AP_DELAY_SLIDER].width;
            normalizedValue = ClampValue(normalizedValue, 0.0f, 1.0f);
            game.autoPlayDelay = AI_DELAY_MIN + normalizedValue * (AI_DELAY_MAX - AI_DELAY_MIN);
        }
        
        return; // Return early so we don't handle other UI elements
    }

    // If in audio settings menu
    if (game.showAudioSettings) {
        // Use cached back button
        if (IsMouseOverRect(cachedRects[RECT_AUDIO_BACK_BUTTON])) {
            if (game.audioMenuSelection != 7) {
                game.audioMenuSelection = 7;
                PlayGameSound(game.menuSound);
            }
            if (mouseClicked) {
                game.showAudioSettings = false;
                PlayGameSound(game.menuSound);
            }
        }
        
        // Check other audio menu items
        for (int i = 0; i < 7; i++) {
            Rectangle itemRect = {
                SCREEN_WIDTH/2 - 275,
                SCREEN_HEIGHT/2 - 150 + (i * 42),
                550, 42
            };
            
            if (IsMouseOverRect(itemRect)) {
                if (game.audioMenuSelection != i) {
                    game.audioMenuSelection = i;
                    PlayGameSound(game.menuSound);
                }
                
                if (mouseClicked) {
                    // Toggle actions for toggleable options
                    if (i == 3) { // Music toggle
                        game.musicEnabled = !game.musicEnabled;
                        if (game.musicEnabled) {
                            PlayMusicStream(game.backgroundMusic);
                        } else {
                            StopMusicStream(game.backgroundMusic);
                        }
                        PlayGameSound(game.menuSound);
                    } else if (i == 4) { // Sound effects toggle
                        game.soundEnabled = !game.soundEnabled;
                        PlayGameSound(game.menuSound);
                    }
                }
            }
        }
        
        // Use cached sliders for volume controls
        for (int i = 0; i < 3; i++) {
            if (IsMouseOverRect(cachedRects[RECT_AUDIO_SLIDER_1 + i]) && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                float normalizedValue = (mousePoint.x - cachedRects[RECT_AUDIO_SLIDER_1 + i].x) / cachedRects[RECT_AUDIO_SLIDER_1 + i].width;
                normalizedValue = ClampValue(normalizedValue, 0.0f, 1.0f);
                
                if (i == 0) game.masterVolume = normalizedValue;
                else if (i == 1) game.musicVolume = normalizedValue;
                else if (i == 2) game.sfxVolume = normalizedValue;
            }
        }
        
        // Use cached sliders for pitch and pan
        if (IsMouseOverRect(cachedRects[RECT_AUDIO_SLIDER_PITCH]) && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            float normalizedValue = (mousePoint.x - cachedRects[RECT_AUDIO_SLIDER_PITCH].x) / cachedRects[RECT_AUDIO_SLIDER_PITCH].width;
            normalizedValue = ClampValue(normalizedValue, 0.0f, 1.0f);
            game.sfxPitch = 0.5f + normalizedValue;
        }
        
        if (IsMouseOverRect(cachedRects[RECT_AUDIO_SLIDER_PAN]) && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            float normalizedValue = (mousePoint.x - cachedRects[RECT_AUDIO_SLIDER_PAN].x) / cachedRects[RECT_AUDIO_SLIDER_PAN].width;
            normalizedValue = ClampValue(normalizedValue, 0.0f, 1.0f);
            game.sfxPan = (normalizedValue * 2.0f) - 1.0f;
        }
        
        return;
    }
    
    // If in pause menu
    else if (game.state == STATE_PAUSED) {
        // Use cached menu item rectangles
        for (int i = 0; i < 5; i++) {
            if (IsMouseOverRect(cachedRects[RECT_PAUSE_ITEM_1 + i])) {
                if (game.menuSelection != i) {
                    game.menuSelection = i;
                    PlayGameSound(game.menuSound);
                }
                
                if (mouseClicked) {
                    PlayGameSound(game.menuSound);
                    
                    switch (i) {
                        case 0: // Resume
                            game.state = STATE_PLAYING;
                            break;
                        case 1: // Restart
                            InitGame();
                            break;
                        case 2: // Audio settings
                            game.showAudioSettings = true;
                            game.audioMenuSelection = 0;
                            break;
                        case 3: // Auto-play settings
                            game.showAutoPlaySettings = true;
                            game.autoPlayMenuSelection = 0;
                            break;
                        case 4: // Exit Game
                            shouldExitGame = true;
                            break;
                    }
                }
            }
        }
    }
    
    // If in game over screen
    else if (game.state == STATE_GAMEOVER) {
        // Use cached play again button
        if (IsMouseOverRect(cachedRects[RECT_PLAY_AGAIN]) && mouseClicked) {
            InitGame();
            game.state = STATE_PLAYING;
            if (game.musicEnabled && !IsMusicStreamPlaying(game.backgroundMusic)) {
                PlayMusicStream(game.backgroundMusic);
                SetMusicVolume(game.backgroundMusic, game.masterVolume * game.musicVolume);
            }
        }
    }
}


// To be added in DrawUI or DrawGame function
void DrawMouseCursor() {
    Vector2 mousePoint = GetMousePosition();
    bool isOverInteractive = false;
    
    // Check if mouse is over any interactive element
    if (game.showAudioSettings) {
        // Audio settings menu items
        for (int i = 0; i < 8; i++) {
            Rectangle itemRect = {
                SCREEN_WIDTH/2 - 275,
                SCREEN_HEIGHT/2 - 150 + (i * 42),
                550, 42
            };
            
            if (IsMouseOverRect(itemRect)) {
                isOverInteractive = true;
                break;
            }
        }
    } else if (game.state == STATE_PAUSED) {
        // Pause menu items
        for (int i = 0; i < 3; i++) {
            Rectangle menuItemRect = {
                SCREEN_WIDTH/2 - 100,
                SCREEN_HEIGHT/2 - 50 + (i * 60) - 10,
                200, 40
            };
            
            if (IsMouseOverRect(menuItemRect)) {
                isOverInteractive = true;
                break;
            }
        }
    } else if (game.state == STATE_GAMEOVER) {
        Rectangle playAgainRect = {
            SCREEN_WIDTH/2 - 150,
            SCREEN_HEIGHT/2 + 70,
            300, 40
        };
        
        if (IsMouseOverRect(playAgainRect)) {
            isOverInteractive = true;
        }
    }
    
    // Draw custom cursor with animation
    float pulseScale = sinf(GetTime() * 8.0f) * 0.2f + 1.0f;
    
    if (isOverInteractive) {
        // Draw interactive cursor (hand-like effect)
        DrawCircleV(mousePoint, 12 * pulseScale, ColorAlpha(WHITE, 0.3f));
        DrawCircleV(mousePoint, 8 * pulseScale, ColorAlpha(YELLOW, 0.5f));
        DrawCircleV(mousePoint, 4 * pulseScale, ColorAlpha(ORANGE, 0.8f));
    } else {
        // Draw standard cursor
        DrawCircleV(mousePoint, 10, ColorAlpha(WHITE, 0.3f));
        DrawCircleV(mousePoint, 6, ColorAlpha(LIGHTGRAY, 0.5f));
        DrawCircleV(mousePoint, 3, ColorAlpha(WHITE, 0.8f));
    }
}

// Auto-play toggle function
void ToggleAutoPlay() {
    game.autoPlayEnabled = !game.autoPlayEnabled;
    
    // Play feedback sound
    PlayGameSound(game.menuSound);
    
    // Show toggle message
    char toggleMsg[32];
    sprintf(toggleMsg, "AUTO-PLAY %s", game.autoPlayEnabled ? "ON" : "OFF");
    AddFloatingText(toggleMsg, 
                  (Vector2){SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 100},
                  game.autoPlayEnabled ? NEON_GREEN : NEON_RED,
                  1.5f);
                  
    // Reset auto-play timer
    game.timer = 0;
}

// Update auto-play logic
void UpdateAutoPlay() {
    // Only run if auto-play is enabled and we're actively playing
    if (!game.autoPlayEnabled || game.state != STATE_PLAYING) return;
    
    // Add a delay between AI moves based on difficulty
    static float aiMoveTimer = 0;
    aiMoveTimer += GetFrameTime();
    
    if (aiMoveTimer >= game.autoPlayDelay) {
        EvaluateAndExecuteBestMove();
        aiMoveTimer = 0;
    }
}

// Find and execute best move
void EvaluateAndExecuteBestMove() {
    // Tetromino bestMove = game.current;
    float bestScore = -INFINITY;
    int bestRotations = 0;
    int bestX = 0;
    bool shouldHold = false;
    
    // First, check if we should hold the piece
    if (game.canHold) {
        // Simulate holding the piece
        Tetromino heldPiece = game.hasHeld ? game.held : game.next;
        
        // Evaluate best position for held piece
        float heldScore = -INFINITY;
        
        // Try all rotations and positions for held piece
        for (int r = 0; r < AI_MAX_ROTATIONS; r++) {
            Tetromino testPiece = heldPiece;
            
            // Apply rotations
            for (int i = 0; i < r; i++) {
                RotateTetromino(&testPiece);
            }
            
            // Try all potential X positions
            for (int x = -3; x < COLS + 3; x++) {
                testPiece.x = x;
                testPiece.y = 0;
                
                // Skip invalid positions
                if (CheckCollision(testPiece)) continue;
                
                // Find drop position
                int dropPos = GetDropPosition(testPiece);
                testPiece.y = dropPos;
                
                // Evaluate this move
                float score = EvaluateMove(testPiece, dropPos, r);
                
                if (score > heldScore) {
                    heldScore = score;
                }
            }
        }
        
        // If holding would give us a better piece, do it
        if (heldScore > bestScore + 2.0f) {  // Add a threshold to avoid constant holding
            shouldHold = true;
            bestScore = heldScore;
        }
    }
    
    // If we don't decide to hold, evaluate current piece
    if (!shouldHold) {
        // Try all rotations and positions
        for (int r = 0; r < AI_MAX_ROTATIONS; r++) {
            Tetromino testPiece = game.current;
            
            // Apply rotations
            for (int i = 0; i < r; i++) {
                RotateTetromino(&testPiece);
            }
            
            // Try all potential X positions
            for (int x = -3; x < COLS + 3; x++) {
                testPiece.x = x;
                testPiece.y = 0;
                
                // Skip invalid positions
                if (CheckCollision(testPiece)) continue;
                
                // Find drop position
                int dropPos = GetDropPosition(testPiece);
                testPiece.y = dropPos;
                
                // Evaluate this move
                float score = EvaluateMove(testPiece, dropPos, r);
                
                if (score > bestScore) {
                    bestScore = score;
                    bestX = x;
                    bestRotations = r;
                }
            }
        }
    }
    
    // Execute the best move
    if (shouldHold) {
        // Hold the current piece
        HoldPiece();
    } else {
        // Apply rotations
        for (int i = 0; i < bestRotations; i++) {
            RotateTetromino(&game.current);
            PlayGameSound(game.rotateSound);
        }
        
        // Move to best X position
        while (game.current.x < bestX) {
            game.current.x++;
            if (CheckCollision(game.current)) {
                game.current.x--;
                break;
            }
        }
        
        while (game.current.x > bestX) {
            game.current.x--;
            if (CheckCollision(game.current)) {
                game.current.x++;
                break;
            }
        }
        
        // Hard drop
        game.current.y = GetDropPosition(game.current);
        MergeToGrid(game.current);
        
        // Play drop sound
        PlayGameSound(game.dropSound);
        
        // Process line clears and update game state
        int linesCleared = ClearLines();
        
        // Update score, etc. (similar logic to HandleInput's hard drop section)
        if (linesCleared > 0) {
            int points = 0;
            const char* clearText = NULL;
            float textScale = 1.0f;
            Color textColor = YELLOW;
            
            // Increment combo
            game.combo++;
            
            // Calculate points with combo multiplier
            switch (linesCleared) {
                case 1: 
                    points = 100 * game.level; 
                    clearText = "+100";
                    break;
                case 2: 
                    points = 300 * game.level;
                    clearText = "+300";
                    textScale = 1.2f;
                    textColor = GREEN;
                    break;
                case 3: 
                    points = 500 * game.level;
                    clearText = "+500";
                    textScale = 1.4f;
                    textColor = BLUE;
                    break;
                case 4: 
                    points = 800 * game.level;
                    clearText = "TETRIS!";
                    textScale = 1.8f;
                    textColor = PURPLE;
                    
                    // Play special sound for Tetris
                    PlayGameSound(game.tetrisSound);
                    break;
            }
            
            // Apply combo multiplier
            if (game.combo > 1) {
                float multiplier = 1.0f + (game.combo * 0.1f);
                points = (int)(points * multiplier);
                
                // Add combo text
                char comboText[32];
                sprintf(comboText, "COMBO x%d!", game.combo);
                
                Vector2 comboPos = {
                    BOARD_OFFSET_X + BOARD_WIDTH / 2,
                    BOARD_OFFSET_Y + BOARD_HEIGHT / 2 - 60
                };
                
                AddFloatingText(comboText, comboPos, ORANGE, 1.5f);
            }
            
            // Add floating score text
            Vector2 textPos = {
                BOARD_OFFSET_X + BOARD_WIDTH / 2,
                BOARD_OFFSET_Y + BOARD_HEIGHT / 2 - 30
            };
            
            AddFloatingText(clearText, textPos, textColor, textScale);
            
            game.score += points;
            game.lines += linesCleared;
            
            // Play sound effect for line clear
            PlayGameSound(game.clearSound);
            
            // Level up every 10 lines
            int oldLevel = game.level;
            game.level = 1 + (game.lines / 10);
            
            // Play level up sound if level changed
            if (game.level > oldLevel) {
                PlayGameSound(game.levelUpSound);
            }
            
            game.delay = 1.0f - (game.level - 1) * 0.1f;
            if (game.delay < 0.1f) game.delay = 0.1f;
            
            // Update high score if needed
            if (game.score > game.highScore) {
                game.highScore = game.score;
                // Save high score immediately
                SaveHighScore(game.highScore);
            }
        } else {
            // Reset combo if no lines cleared
            game.combo = 0;
        }
        
        game.current = game.next;
        game.next = GetRandomTetromino();
        game.softDropY = game.current.y;
        game.canHold = true;
        
        // Check for game over
        if (CheckCollision(game.current)) {
            game.state = STATE_GAMEOVER;
            game.autoPlayEnabled = false; // Disable auto-play on game over
            
            // Play game over sound
            PlayGameSound(game.gameoverSound);
            
            // Stop background music
            if (game.musicEnabled && IsMusicStreamPlaying(game.backgroundMusic)) {
                StopMusicStream(game.backgroundMusic);
            }
        }
    }
}

// Evaluate a potential move using weighted heuristics
float EvaluateMove(Tetromino t, int dropPosition, int rotations) {
    // Create a copy of the grid with this move applied
    int testGrid[ROWS][COLS];
    memcpy(testGrid, game.grid, sizeof(game.grid));
    
    // Apply the move to the test grid
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (t.shape[i][j]) {
                int y = t.y + i;
                int x = t.x + j;
                if (y >= 0 && y < ROWS && x >= 0 && x < COLS) {
                    testGrid[y][x] = 1;
                }
            }
        }
    }
    
    // Calculate heuristics
    
    // 1. Calculate aggregate height (sum of heights of each column)
    int aggregateHeight = 0;
    int columnHeights[COLS] = {0};
    
    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < ROWS; y++) {
            if (testGrid[y][x]) {
                columnHeights[x] = ROWS - y;
                break;
            }
        }
        aggregateHeight += columnHeights[x];
    }
    
    // 2. Calculate complete lines
    int completeLines = 0;
    for (int y = 0; y < ROWS; y++) {
        bool isComplete = true;
        for (int x = 0; x < COLS; x++) {
            if (!testGrid[y][x]) {
                isComplete = false;
                break;
            }
        }
        if (isComplete) completeLines++;
    }
    
    // 3. Calculate holes (empty cells with filled cells above)
    int holes = 0;
    for (int x = 0; x < COLS; x++) {
        bool blockFound = false;
        for (int y = 0; y < ROWS; y++) {
            if (testGrid[y][x]) {
                blockFound = true;
            } else if (blockFound) {
                holes++;
            }
        }
    }
    
    // 4. Calculate bumpiness (sum of differences between adjacent columns)
    int bumpiness = 0;
    for (int x = 0; x < COLS - 1; x++) {
        bumpiness += abs(columnHeights[x] - columnHeights[x + 1]);
    }
    
    // Adjust weights based on AI level
    float heightWeight = AI_HEIGHT_WEIGHT;
    float linesWeight = AI_LINES_WEIGHT;
    float holesWeight = AI_HOLES_WEIGHT;
    float bumpinessWeight = AI_BUMPINESS_WEIGHT;
    
    // Apply weights based on AI difficulty level
    switch (game.autoPlayLevel) {
        case 0: // Beginner
            heightWeight *= 0.7f;
            holesWeight *= 0.6f;
            break;
        case 1: // Intermediate
            // Use default weights
            break;
        case 2: // Expert
            linesWeight *= 1.2f;
            holesWeight *= 1.3f;
            break;
    }
    
    // Calculate final score using weighted sum
    float score = 
        heightWeight * aggregateHeight +
        linesWeight * completeLines +
        holesWeight * holes +
        bumpinessWeight * bumpiness;
        
    // Add bonus for using the hold feature (encourages strategic holds)
    if (t.type == game.held.type && game.hasHeld) {
        score += 0.5f;
    }
    
    return score;
}


// Draw auto-play indicator
void DrawAutoPlayIndicator() {
    if (!game.autoPlayEnabled) return;
    
    // Draw auto-play indicator in the top-left corner
    DrawRectangle(20, SCREEN_HEIGHT - 60, 200, 50, ColorAlpha(BLACK, 0.7f));
    DrawRectangleLines(20, SCREEN_HEIGHT - 60, 200, 50, NEON_GREEN);
    
    // Pulsing effect
    float pulse = sinf(GetTime() * 4.0f) * 0.3f + 0.7f;
    
    if (game.customFontLoaded) {
        DrawTextEx(
            game.gameFont, "AUTO-PLAY ACTIVE",
            (Vector2){ 30, SCREEN_HEIGHT - 55 },
            22, 2, ColorAlpha(NEON_GREEN, pulse)
        );
    } else {
        DrawText(
            "AUTO-PLAY ACTIVE",
            30, SCREEN_HEIGHT - 55, 22, ColorAlpha(NEON_GREEN, pulse)
        );
    }
    
    // Show AI level
    const char* levelText = "";
    switch(game.autoPlayLevel) {
        case 0: levelText = "BEGINNER"; break;
        case 1: levelText = "MEDIUM"; break;
        case 2: levelText = "EXPERT"; break;
    }
    
    DrawText(levelText, 120-(MeasureText(levelText, 16)/2), SCREEN_HEIGHT - 30, 16, ColorAlpha(YELLOW, pulse));
}

// Auto-play settings menu
void DrawAutoPlaySettings() {
    int menuWidth = 550;
    int menuHeight = 350;
    
    // Draw background overlay
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha(BLACK, 0.85f));
    
    // Draw menu panel with improved gradient and rounded corners effect
    DrawRectangleGradientV(
        SCREEN_WIDTH/2 - menuWidth/2, SCREEN_HEIGHT/2 - menuHeight/2,
        menuWidth, menuHeight,
        ColorAlpha((Color){40, 45, 80, 255}, 0.95f),
        ColorAlpha((Color){25, 25, 50, 255}, 0.95f)
    );
    
    // Draw an inner border for modern look
    DrawRectangleLines(
        SCREEN_WIDTH/2 - menuWidth/2 + 2, SCREEN_HEIGHT/2 - menuHeight/2 + 2,
        menuWidth - 4, menuHeight - 4,
        ColorAlpha(NEON_GREEN, 0.6f)
    );
    
    // Draw outer glow border
    float glowPulse = sinf(GetTime() * 1.5f) * 0.5f + 0.5f;
    DrawRectangleLines(
        SCREEN_WIDTH/2 - menuWidth/2 - 1, SCREEN_HEIGHT/2 - menuHeight/2 - 1,
        menuWidth + 2, menuHeight + 2,
        ColorAlpha(NEON_GREEN, 0.3f + 0.2f * glowPulse)
    );
    
    // Title with underline
    int titleY = SCREEN_HEIGHT/2 - menuHeight/2 + 25;
    DrawText("AUTO-PLAY SETTINGS", 
             SCREEN_WIDTH/2 - MeasureText("AUTO-PLAY SETTINGS", 30)/2, 
             titleY, 30, WHITE);
             
    // Draw title underline with gradient
    DrawRectangleGradientH(
        SCREEN_WIDTH/2 - menuWidth/2 + 50,
        titleY + 40,
        menuWidth - 100,
        2,
        ColorAlpha(NEON_GREEN, 0.7f),
        ColorAlpha(DARKGREEN, 0.1f)
    );
    
    // Menu items and values
    const char* menuItems[] = {
        "Auto-Play",
        "AI Difficulty",
        "Move Delay",
        "Back"
    };
    
    int startY = SCREEN_HEIGHT/2 - menuHeight/2 + 90;
    int itemHeight = 50;
    int labelX = SCREEN_WIDTH/2 - menuWidth/2 + 50;
    int controlX = SCREEN_WIDTH/2 - 60;
    int controlWidth = 230;
    
    for (int i = 0; i < 4; i++) {
        Color itemColor = (i == game.autoPlayMenuSelection) ? 
                          ColorAlpha(GOLD, 0.9f + sinf(GetTime() * 4.0f) * 0.1f) : 
                          ColorAlpha(WHITE, 0.85f);
        
        // Draw item background for selected item
        if (i == game.autoPlayMenuSelection) {
            DrawRectangle(
                SCREEN_WIDTH/2 - menuWidth/2 + 10,
                startY + i * itemHeight - 8,
                menuWidth - 20,
                itemHeight,
                ColorAlpha(WHITE, 0.07f)
            );
        }
        
        // Draw menu item text
        if (i == game.autoPlayMenuSelection) {
            DrawText(menuItems[i], 
                    labelX + 1, 
                    startY + i * itemHeight + 1, 
                    22, ColorAlpha(BLACK, 0.5f));
        }
        
        DrawText(menuItems[i], 
                labelX, 
                startY + i * itemHeight, 
                22, itemColor);
        
        // Draw control for each setting
        switch(i) {
            case 0: // Auto-Play toggle
                {
                    bool enabled = game.autoPlayEnabled;
                    Color toggleColor = enabled ? GREEN : MAROON;
                    
                    // Draw toggle background
                    DrawRectangle(controlX, startY + i * itemHeight + 4, 60, 24, ColorAlpha(DARKGRAY, 0.3f));
                    
                    // Draw toggle state
                    DrawRectangle(
                        controlX + (enabled ? 30 : 0), 
                        startY + i * itemHeight + 4, 
                        30, 24, 
                        toggleColor
                    );
                    
                    // Draw label
                    DrawText(enabled ? "ON" : "OFF", 
                            controlX + 70, 
                            startY + i * itemHeight, 22,
                            enabled ? GREEN : ColorAlpha(RED, 0.8f));
                }
                break;
                
            case 1: // AI Difficulty
                {
                    const char* difficultyLabels[] = {"BEGINNER", "MEDIUM", "EXPERT"};
                    Color difficultyColors[] = {GREEN, YELLOW, RED};
                    
                    // Draw difficulty label
                    DrawText(difficultyLabels[game.autoPlayLevel], 
                            controlX, 
                            startY + i * itemHeight, 22,
                            difficultyColors[game.autoPlayLevel]);
                    
                    // Draw selector arrows
                    if (i == game.autoPlayMenuSelection) {
                        float arrowPulse = sinf(GetTime() * 4.0f) * 0.3f + 0.7f;
                        if (game.autoPlayLevel > 0) {
                            DrawText("<", 
                                    controlX - 25, 
                                    startY + i * itemHeight, 22,
                                    ColorAlpha(WHITE, arrowPulse));
                        }
                        
                        if (game.autoPlayLevel < AI_LEVELS - 1) {
                            DrawText(">", 
                                    controlX + MeasureText(difficultyLabels[game.autoPlayLevel], 22) + 10, 
                                    startY + i * itemHeight, 22,
                                    ColorAlpha(WHITE, arrowPulse));
                        }
                    }
                }
                break;
                
            case 2: // Move Delay
                {
                    // Map delay from AI_DELAY_MIN-AI_DELAY_MAX to 0-1 for drawing
                    float normalizedValue = (game.autoPlayDelay - AI_DELAY_MIN) / (AI_DELAY_MAX - AI_DELAY_MIN);
                    
                    // Slider background
                    DrawRectangle(controlX, startY + i * itemHeight + 8, controlWidth, 8, ColorAlpha(DARKGRAY, 0.5f));
                    
                    // Slider fill
                    DrawRectangle(controlX, startY + i * itemHeight + 8, (int)(normalizedValue * controlWidth), 8, NEON_GREEN);
                    
                    // Slider handle
                    DrawCircle(controlX + (int)(normalizedValue * controlWidth), startY + i * itemHeight + 12, 8, 
                              (i == game.autoPlayMenuSelection) ? YELLOW : NEON_GREEN);
                    
                    // Value text
                    DrawText(TextFormat("%.2fs", game.autoPlayDelay), 
                            controlX + controlWidth + 15, 
                            startY + i * itemHeight, 22, itemColor);
                }
                break;
                
            case 3: // Back button
                {
                    if (i == game.autoPlayMenuSelection) {
                        // Draw button with highlight effect
                        DrawRectangleGradientH(
                            controlX - 20,
                            startY + i * itemHeight - 2,
                            100,
                            30,
                            ColorAlpha(GREEN, 0.5f),
                            ColorAlpha(DARKGREEN, 0.3f)
                        );
                        
                        // Button text
                        DrawText("BACK", 
                                controlX + 10, 
                                startY + i * itemHeight, 22, RAYWHITE);
                    }
                }
                break;
        }
    }
    
    // Instructions
    int instructionsY = SCREEN_HEIGHT/2 + menuHeight/2 - 60;
    
    // Instructions container
    DrawRectangle(
        SCREEN_WIDTH/2 - 200,
        instructionsY - 5,
        400,
        35,
        ColorAlpha(BLACK, 0.3f)
    );
    
    DrawText("Press ALT+A to quickly toggle Auto-Play", 
             SCREEN_WIDTH/2 - 195, 
             instructionsY, 20, ColorAlpha(LIGHTGRAY, 0.8f));
}

// Handle auto-play settings input
void HandleAutoPlaySettings() {
    // Handle keyboard input
    if (IsKeyPressed(KEY_UP)) {
        game.autoPlayMenuSelection--;
        if (game.autoPlayMenuSelection < 0) game.autoPlayMenuSelection = 3;
        PlayGameSound(game.menuSound);
    }
    
    if (IsKeyPressed(KEY_DOWN)) {
        game.autoPlayMenuSelection++;
        if (game.autoPlayMenuSelection > 3) game.autoPlayMenuSelection = 0;
        PlayGameSound(game.menuSound);
    }
    
    // Handle option adjustments
    switch(game.autoPlayMenuSelection) {
        case 0: // Auto-Play toggle
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT)) {
                game.autoPlayEnabled = !game.autoPlayEnabled;
                PlayGameSound(game.menuSound);
            }
            break;
            
        case 1: // AI Difficulty
            if (IsKeyPressed(KEY_LEFT) && game.autoPlayLevel > 0) {
                game.autoPlayLevel--;
                PlayGameSound(game.menuSound);
            }
            if (IsKeyPressed(KEY_RIGHT) && game.autoPlayLevel < AI_LEVELS - 1) {
                game.autoPlayLevel++;
                PlayGameSound(game.menuSound);
            }
            break;
            
        case 2: // Move Delay
            if (IsKeyDown(KEY_LEFT)) {
                game.autoPlayDelay = ClampValue(game.autoPlayDelay - 0.01f, AI_DELAY_MIN, AI_DELAY_MAX);
            }
            if (IsKeyDown(KEY_RIGHT)) {
                game.autoPlayDelay = ClampValue(game.autoPlayDelay + 0.01f, AI_DELAY_MIN, AI_DELAY_MAX);
            }
            break;
            
        case 3: // Back
            if (IsKeyPressed(KEY_ENTER)) {
                game.showAutoPlaySettings = false;
                PlayGameSound(game.menuSound);
            }
            break;
    }
    
    // Exit menu with ESC
    if (IsKeyPressed(KEY_ESCAPE)) {
        game.showAutoPlaySettings = false;
        PlayGameSound(game.menuSound);
    }
    
    // Also handle mouse input for settings
    Vector2 mousePoint = GetMousePosition();
    bool mouseClicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    
    // Menu dimensions from DrawAutoPlaySettings
    int menuWidth = 550;
    int menuHeight = 350;
    int startY = SCREEN_HEIGHT/2 - menuHeight/2 + 90;
    int itemHeight = 50;
    int controlX = SCREEN_WIDTH/2 - 60;
    int controlWidth = 230;
    
    // Check for Back button clicks
    Rectangle backButtonRect = {
        (float)(controlX - 20), 
        (float)(startY + 3 * itemHeight - 2), 
        100.0f, 30.0f
    };
    
    if (IsMouseOverRect(backButtonRect)) {
        if (game.autoPlayMenuSelection != 3) {
            game.autoPlayMenuSelection = 3;
            PlayGameSound(game.menuSound);
        }
        
        if (mouseClicked) {
            game.showAutoPlaySettings = false;
            PlayGameSound(game.menuSound);
        }
    }
    
    // Check item selection
    for (int i = 0; i < 3; i++) {
        Rectangle itemRect = {
            (float)(SCREEN_WIDTH/2 - menuWidth/2 + 10),
            (float)(startY + i * itemHeight - 8),
            (float)(menuWidth - 20),
            (float)itemHeight
        };
        
        if (IsMouseOverRect(itemRect)) {
            if (game.autoPlayMenuSelection != i) {
                game.autoPlayMenuSelection = i;
                PlayGameSound(game.menuSound);
            }
            
            if (mouseClicked) {
                switch(i) {
                    case 0: // Auto-Play toggle
                        game.autoPlayEnabled = !game.autoPlayEnabled;
                        PlayGameSound(game.menuSound);
                        break;
                }
            }
        }
    }


    // Handle toggle explicitly - add specific hit area for the toggle control
    Rectangle toggleRect = {
        (float)controlX, 
        (float)(startY + 0 * itemHeight + 4), 
        60.0f, 24.0f
    };
    
    if (IsMouseOverRect(toggleRect) && mouseClicked) {
        game.autoPlayEnabled = !game.autoPlayEnabled;
        PlayGameSound(game.menuSound);
    }
    
    
    // Handle difficulty selector arrows - improve hit areas
    if (game.autoPlayLevel > 0) {
        Rectangle leftArrowRect = {
            (float)(controlX - 25),
            (float)(startY + itemHeight),
            20.0f, 22.0f
        };
        
        if (IsMouseOverRect(leftArrowRect) && mouseClicked) {
            game.autoPlayLevel--;
            PlayGameSound(game.menuSound);
        }
    }
    
    if (game.autoPlayLevel < AI_LEVELS - 1) {
        const char* difficultyLabels[] = {"BEGINNER", "MEDIUM", "EXPERT"};
        Rectangle rightArrowRect = {
            (float)(controlX + MeasureText(difficultyLabels[game.autoPlayLevel], 22) + 10),
            (float)(startY + itemHeight),
            20.0f, 22.0f
        };
        
        if (IsMouseOverRect(rightArrowRect) && mouseClicked) {
            game.autoPlayLevel++;
            PlayGameSound(game.menuSound);
        }
    }
    
    // Handle slider for delay with improved hit area
    Rectangle delaySliderRect = {
        (float)controlX,
        (float)(startY + 2 * itemHeight + 3), // Expanded hit area
        (float)controlWidth,
        20.0f  // Taller hit area
    };

    if (IsMouseOverRect(delaySliderRect) && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        float normalizedValue = (mousePoint.x - delaySliderRect.x) / delaySliderRect.width;
        normalizedValue = ClampValue(normalizedValue, 0.0f, 1.0f);
        game.autoPlayDelay = AI_DELAY_MIN + normalizedValue * (AI_DELAY_MAX - AI_DELAY_MIN);
    }

}


// Main function with high score persistence
int main() {
    // Set configuration flags
    // SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);  // Make window resizable
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Retro Tetris - By Bismaya");
    SetExitKey(KEY_NULL);

    // Set custom window icon
    if (FileExists("resources/images/phantom_logo_org.png")) {
        Image icon = LoadImage("resources/images/phantom_logo_org.png");
        SetWindowIcon(icon);
        UnloadImage(icon); // Free the image data after setting the icon
        TraceLog(LOG_INFO, "Custom window icon loaded successfully");
    } else {
        TraceLog(LOG_WARNING, "Window icon not found, using default icon");
    }

    // Calculate initial scaling factors
    scaleX = (float)GetScreenWidth() / REFERENCE_WIDTH;
    scaleY = (float)GetScreenHeight() / REFERENCE_HEIGHT;

    BOARD_OFFSET_X = (GetScreenWidth() - (COLS * BLOCK_SIZE * scaleX)) / 2;
    BOARD_OFFSET_Y = ScaleY(60);
    
    // Initialize audio device with error checking
    InitAudioDevice();  // Call without checking return value
    
    // Check if audio is ready
    if (!IsAudioDeviceReady()) {
        TraceLog(LOG_WARNING, "Audio device could not be initialized, continuing without sound");
    }
    
    SetTargetFPS(60);
    srand((unsigned int)time(NULL));

    // Load phantom engine logo
    if (FileExists("resources/images/phantom_logo.png")) {
        engineLogo = LoadTexture("resources/images/phantom_logo.png");
        logoLoaded = true;
    } else {
        TraceLog(LOG_WARNING, "Engine logo not found, continuing without logo");
    }
    
    // Load shaders with error handling
    bool shadersLoaded = false;
    
    // Check if shader directory exists
    if (DirectoryExists("resources/shaders")) {
        LoadShaders();
        shadersLoaded = true;
    } else {
        TraceLog(LOG_WARNING, "Shader directory not found, continuing without effects");
    }
    
    // Hide system cursor since we're using a custom one
    HideCursor();
    
    // Load game resources
    LoadGameResources();
    
    // Load high score from file
    game.highScore = LoadHighScore();
    TraceLog(LOG_INFO, "Loaded high score: %d", game.highScore);

    // Start with splash screen
    game.state = STATE_SPLASH;
    splashTimer = 0.0f;
    shouldExitGame = false;
    
    InitGame();
    
    // Main game loop
    while (!WindowShouldClose() && !shouldExitGame) {
        // Check if window was resized
        if (IsWindowResized()) {
            scaleX = (float)GetScreenWidth() / REFERENCE_WIDTH;
            scaleY = (float)GetScreenHeight() / REFERENCE_HEIGHT;
            
            // Recalculate board offset to keep it centered
            BOARD_OFFSET_X = (GetScreenWidth() - (COLS * BLOCK_SIZE * scaleX)) / 2;
            BOARD_OFFSET_Y = ScaleY(60);
        }
        
        HandleInput();
        UpdateGame();
        DrawGame();
    }
    
    // Save high score before exiting
    SaveHighScore(game.highScore);
    TraceLog(LOG_INFO, "Saved high score: %d", game.highScore);
    
    // Clean up resources
    if (logoLoaded) {
        UnloadTexture(engineLogo);
    }

    // Clean up resources
    if (shadersLoaded) {
        UnloadShaders();
    }
    UnloadGameResources();
    CloseAudioDevice();
    CloseWindow();
    
    return 0;
}

// Utility function to clamp values within a range
float ClampValue(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// Function to save high score to file
void SaveHighScore(int score) {
    // Create data folder if it doesn't exist
    if (!DirectoryExists("data")) {
        if (mkdir("data") != 0) {
            TraceLog(LOG_ERROR, "Failed to create data directory");
            return;
        }
    }
    
    // Save to file with error handling
    FILE *file = fopen("data/highscore.dat", "wb");
    if (file != NULL) {
        fwrite(&score, sizeof(int), 1, file);
        fclose(file);
        TraceLog(LOG_INFO, "High score saved successfully");
    } else {
        TraceLog(LOG_ERROR, "Failed to open highscore.dat for writing");
    }
}

// Function to load high score from file
int LoadHighScore() {
    int score = 0;
    
    // Check if file exists before trying to open
    if (FileExists("data/highscore.dat")) {
        FILE *file = fopen("data/highscore.dat", "rb");
        if (file != NULL) {
            size_t readResult = fread(&score, sizeof(int), 1, file);
            if (readResult != 1) {
                TraceLog(LOG_WARNING, "Failed to read high score, using default value");
                score = 0;
            }
            fclose(file);
        } else {
            TraceLog(LOG_WARNING, "Failed to open highscore.dat for reading");
        }
    } else {
        TraceLog(LOG_INFO, "No existing high score found, starting with 0");
    }
    
    return score;
}

void DrawGameOverScreen() {
    // Semi-transparent overlay
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha(BLACK, 0.7f));
    
    // Game over panel with gradient
    int panelWidth = 500;
    int panelHeight = 380;
    DrawRectangleGradientV(
        SCREEN_WIDTH/2 - panelWidth/2, 
        SCREEN_HEIGHT/2 - panelHeight/2,
        panelWidth, panelHeight,
        ColorAlpha(DARKRED, 0.8f),
        ColorAlpha(BLACK, 0.8f)
    );
    
    // Panel border with glowing effect
    float glowIntensity = sinf(GetTime() * 2.0f) * 0.3f + 0.7f;
    DrawRectangleLines(
        SCREEN_WIDTH/2 - panelWidth/2, 
        SCREEN_HEIGHT/2 - panelHeight/2,
        panelWidth, panelHeight,
        ColorAlpha(NEON_RED, glowIntensity)
    );
    
    // Game Over text with glitch effect
    const char* gameOverText = "GAME OVER";
    int titleY = SCREEN_HEIGHT/2 - panelHeight/2 + 40;
    
    // Draw glitch copies
    for (int i = 0; i < 3; i++) {
        float offsetX = sinf(GetTime() * (10.0f + i * 3.0f)) * 5.0f;
        float offsetY = cosf(GetTime() * (8.0f + i * 2.0f)) * 3.0f;
        Color glitchColor = ColorAlpha(
            (i == 0) ? NEON_BLUE : ((i == 1) ? NEON_PINK : NEON_GREEN), 
            0.3f * glowIntensity
        );
        
        if (game.customFontLoaded) {
            DrawTextEx(
                game.gameFont, gameOverText,
                (Vector2){ 
                    SCREEN_WIDTH/2 - MeasureTextEx(game.gameFont, gameOverText, 60, 2).x/2 + offsetX, 
                    titleY + offsetY 
                },
                60, 2, glitchColor
            );
        } else {
            DrawText(
                gameOverText,
                SCREEN_WIDTH/2 - MeasureText(gameOverText, 60)/2 + offsetX,
                titleY + offsetY,
                60, glitchColor
            );
        }
    }
    
    // Main text
    if (game.customFontLoaded) {
        DrawTextEx(
            game.gameFont, gameOverText,
            (Vector2){ 
                SCREEN_WIDTH/2 - MeasureTextEx(game.gameFont, gameOverText, 60, 2).x/2, 
                titleY 
            },
            60, 2, WHITE
        );
    } else {
        DrawText(
            gameOverText,
            SCREEN_WIDTH/2 - MeasureText(gameOverText, 60)/2,
            titleY,
            60, WHITE
        );
    }
    
    // Score text
    char finalScoreText[64];
    sprintf(finalScoreText, "FINAL SCORE: %d", game.score);
    
    if (game.customFontLoaded) {
        DrawTextEx(
            game.gameFont, finalScoreText,
            (Vector2){ 
                SCREEN_WIDTH/2 - MeasureTextEx(game.gameFont, finalScoreText, 30, 2).x/2, 
                titleY + 100
            },
            30, 2, NEON_YELLOW
        );
    } else {
        DrawText(
            finalScoreText,
            SCREEN_WIDTH/2 - MeasureText(finalScoreText, 30)/2,
            titleY + 100,
            30, NEON_YELLOW
        );
    }
    
    // High score text
    if (game.score >= game.highScore && game.score > 0) {
        const char* newHighScoreText = "NEW HIGH SCORE!";
        float pulseScale = sinf(GetTime() * 4.0f) * 0.2f + 1.0f;
        
        if (game.customFontLoaded) {
            DrawTextEx(
                game.gameFont, newHighScoreText,
                (Vector2){ 
                    SCREEN_WIDTH/2 - MeasureTextEx(game.gameFont, newHighScoreText, 30 * pulseScale, 2).x/2, 
                    titleY + 150
                },
                30 * pulseScale, 2, NEON_GREEN
            );
        } else {
            DrawText(
                newHighScoreText,
                SCREEN_WIDTH/2 - MeasureText(newHighScoreText, 30 * pulseScale)/2,
                titleY + 150,
                30 * pulseScale, NEON_GREEN
            );
        }
    }
    
    // Play again button
    Rectangle playAgainButton = {
        SCREEN_WIDTH/2 - 150, 
        SCREEN_HEIGHT/2 + 70,
        300, 60
    };
    
    // Check if mouse is over button
    bool isMouseOver = IsMouseOverRect(playAgainButton);
    Color buttonColor = isMouseOver ? NEON_GREEN : ColorAlpha(NEON_GREEN, 0.6f);
    
    // Button with animation
    float btnGlow = sinf(GetTime() * 3.0f) * 0.3f + 0.7f;
    if (isMouseOver) btnGlow = 1.0f;
    
    DrawRectangleRec(playAgainButton, ColorAlpha(BLACK, 0.6f));
    DrawRectangleLinesEx(playAgainButton, 2, ColorAlpha(buttonColor, btnGlow));
    
    const char* playAgainText = "PLAY AGAIN";
    if (game.customFontLoaded) {
        DrawTextEx(
            game.gameFont, playAgainText,
            (Vector2){ 
                SCREEN_WIDTH/2 - MeasureTextEx(game.gameFont, playAgainText, 30, 2).x/2, 
                SCREEN_HEIGHT/2 + 85 
            },
            30, 2, buttonColor
        );
    } else {
        DrawText(
            playAgainText,
            SCREEN_WIDTH/2 - MeasureText(playAgainText, 30)/2,
            SCREEN_HEIGHT/2 + 85,
            30, buttonColor
        );
    }
    
    // Instructions
    DrawText("Press ENTER to play again", 
             SCREEN_WIDTH/2 - MeasureText("Press ENTER to play again", 20)/2, 
             SCREEN_HEIGHT/2 + 150, 20, 
             ColorAlpha(LIGHTGRAY, 0.8f));
}

void DrawSplashScreen() {
    // Background with dark cyberpunk gradient
    DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 
                        CYBER_BLACK, 
                        CYBER_DARKBLUE);
    
    // Add cyber grid for background effect
    float gridTime = GetTime() * 0.2f;
    int gridSize = 60;
    float gridAlpha = 0.1f;

    // Horizontal grid lines with movement effect
    for(int i = 0; i < SCREEN_HEIGHT / gridSize + 1; i++) {
        float y = (float)i * gridSize + fmodf(gridTime * 30, gridSize);
        float alpha = gridAlpha * (1.0f - (y / SCREEN_HEIGHT) * 0.8f);
        DrawLineEx(
            (Vector2){0, y},
            (Vector2){SCREEN_WIDTH, y},
            1.0f,
            ColorAlpha(NEON_BLUE, alpha)
        );
    }

    // Vertical grid lines
    for(int i = 0; i < SCREEN_WIDTH / gridSize + 1; i++) {
        float x = (float)i * gridSize;
        DrawLineEx(
            (Vector2){x, 0},
            (Vector2){x, SCREEN_HEIGHT},
            1.0f,
            ColorAlpha(NEON_BLUE, gridAlpha * 0.5f)
        );
    }
    
    // Add scanner effect
    for(int i = 0; i < SCREEN_HEIGHT; i += 3) {
        DrawRectangle(0, i, SCREEN_WIDTH, 1, ColorAlpha(BLACK, 0.05f));
    }
    
    // Calculate animation progress (0.0 to 1.0)
    float progress = splashTimer / 3.0f;  // 3 second splash screen
    progress = progress > 1.0f ? 1.0f : progress;
    
    // Ease in-out for smoother animation
    float easeProgress = progress < 0.5f ? 
                         2.0f * progress * progress : 
                         1.0f - pow(-2.0f * progress + 2.0f, 2.0f) / 2.0f;
    
    // Logo
    if (logoLoaded) {
        // Animate logo scale and alpha
        float scale = 0.8f + easeProgress * 0.2f;
        float alpha = easeProgress;
        
        // Draw the engine logo
        DrawTexturePro(
            engineLogo,
            (Rectangle){ 0, 0, engineLogo.width, engineLogo.height },
            (Rectangle){ 
                SCREEN_WIDTH/2, 
                SCREEN_HEIGHT/2 - 100, 
                engineLogo.width * scale, 
                engineLogo.height * scale 
            },
            (Vector2){ engineLogo.width * scale / 2, engineLogo.height * scale / 2 },
            0.0f,
            ColorAlpha(WHITE, alpha)
        );
    }
    
    // "RETRO TETRIS" text with animated reveal
    float textProgress = (progress - 0.3f) * 1.5f;  // Start text after logo appears
    if (textProgress > 0) {
        textProgress = textProgress > 1.0f ? 1.0f : textProgress;
        
        if (game.customFontLoaded) {
            const char* titleText = "RETRO TETRIS";
            Vector2 textSize = MeasureTextEx(game.gameFont, titleText, 80, 2);
            
            // Add glitch effect for cyber style
            for (int i = 0; i < 3; i++) {
                float glitchX = sinf(GetTime() * (10.0f + i)) * 5.0f * textProgress;
                float glitchY = cosf(GetTime() * (8.0f + i)) * 3.0f * textProgress;
                Color glitchColor = i == 0 ? NEON_BLUE : (i == 1 ? NEON_PINK : NEON_GREEN);
                
                DrawTextEx(
                    game.gameFont, titleText,
                    (Vector2){ 
                        SCREEN_WIDTH/2 - textSize.x/2 + glitchX, 
                        SCREEN_HEIGHT/2 + 50 + glitchY
                    },
                    80, 2, 
                    ColorAlpha(glitchColor, 0.3f * textProgress)
                );
            }
            
            // Main title text with cyberpunk color
            DrawTextEx(
                game.gameFont, titleText,
                (Vector2){ SCREEN_WIDTH/2 - textSize.x/2, SCREEN_HEIGHT/2 + 50 },
                80, 2, 
                ColorAlpha(NEON_PINK, textProgress)
            );
        } else {
            // Fallback with standard font
            const char* titleText = "RETRO TETRIS";
            int titleWidth = MeasureText(titleText, 80);
            DrawText(
                titleText,
                SCREEN_WIDTH/2 - titleWidth/2,
                SCREEN_HEIGHT/2 + 50,
                80,
                ColorAlpha(NEON_PINK, textProgress)
            );
        }
    }
    
    // "Powered by Phantom Engine" text
    float poweredByProgress = (progress - 0.5f) * 2.0f;
    if (poweredByProgress > 0) {
        poweredByProgress = poweredByProgress > 1.0f ? 1.0f : poweredByProgress;
        
        if (game.customFontLoaded) {
            const char* poweredByText = "POWERED BY PHANTOM ENGINE";
            Vector2 textSize = MeasureTextEx(game.gameFont, poweredByText, 30, 2);
            
            DrawTextEx(
                game.gameFont, poweredByText,
                (Vector2){ SCREEN_WIDTH/2 - textSize.x/2, SCREEN_HEIGHT/2 + 150 },
                30, 2, 
                ColorAlpha(NEON_BLUE, poweredByProgress)
            );
        } else {
            const char* poweredByText = "POWERED BY PHANTOM ENGINE";
            int textWidth = MeasureText(poweredByText, 30);
            DrawText(
                poweredByText,
                SCREEN_WIDTH/2 - textWidth/2,
                SCREEN_HEIGHT/2 + 150,
                30,
                ColorAlpha(NEON_BLUE, poweredByProgress)
            );
        }
    }
    
    // "Created by Bismaya" text
    float createdByProgress = (progress - 0.7f) * 3.0f;
    if (createdByProgress > 0) {
        createdByProgress = createdByProgress > 1.0f ? 1.0f : createdByProgress;
        
        if (game.customFontLoaded) {
            const char* createdByText = "CREATED BY BISMAYA";
            Vector2 textSize = MeasureTextEx(game.gameFont, createdByText, 24, 2);
            
            DrawTextEx(
                game.gameFont, createdByText,
                (Vector2){ SCREEN_WIDTH/2 - textSize.x/2, SCREEN_HEIGHT/2 + 200 },
                24, 2, 
                ColorAlpha(NEON_GREEN, createdByProgress)
            );
        } else {
            const char* createdByText = "CREATED BY BISMAYA";
            int textWidth = MeasureText(createdByText, 24);
            DrawText(
                createdByText,
                SCREEN_WIDTH/2 - textWidth/2,
                SCREEN_HEIGHT/2 + 200,
                24,
                ColorAlpha(NEON_GREEN, createdByProgress)
            );
        }
    }
    
    // Press any key text with pulse animation (appears near the end)
    float pressKeyProgress = (progress - 0.9f) * 10.0f;
    if (pressKeyProgress > 0) {
        pressKeyProgress = pressKeyProgress > 1.0f ? 1.0f : pressKeyProgress;
        float pulse = sinf(GetTime() * 4.0f) * 0.3f + 0.7f;
        
        const char* pressKeyText = "PRESS ANY KEY TO CONTINUE";
        int textWidth = MeasureText(pressKeyText, 20);
        DrawText(
            pressKeyText,
            SCREEN_WIDTH/2 - textWidth/2,
            SCREEN_HEIGHT - 100,
            20,
            ColorAlpha(WHITE, pressKeyProgress * pulse)
        );
    }
}

// Add this function above DrawMainMenu
void DrawMenuParticles() {
    static Vector2 particles[50] = {0};
    static Vector2 particleSpeeds[50] = {0};
    static Color particleColors[50] = {0};
    static float particleSizes[50] = {0};
    static bool initialized = false;
    
    // Initialize particles on first run
    if (!initialized) {
        for (int i = 0; i < 50; i++) {
            particles[i] = (Vector2){ GetRandomValue(0, SCREEN_WIDTH), GetRandomValue(0, SCREEN_HEIGHT) };
            particleSpeeds[i] = (Vector2){ GetRandomValue(-100, 100) / 100.0f, GetRandomValue(-100, 100) / 100.0f };
            
            // Select random neon color
            Color neonColors[] = { NEON_PINK, NEON_BLUE, NEON_GREEN, NEON_PURPLE, NEON_YELLOW };
            particleColors[i] = neonColors[GetRandomValue(0, 4)];
            
            particleSizes[i] = GetRandomValue(1, 4);
        }
        initialized = true;
    }
    
    // Update and draw particles
    for (int i = 0; i < 50; i++) {
        // Update position
        particles[i].x += particleSpeeds[i].x;
        particles[i].y += particleSpeeds[i].y;
        
        // Wrap around screen
        if (particles[i].x > SCREEN_WIDTH) particles[i].x = 0;
        if (particles[i].x < 0) particles[i].x = SCREEN_WIDTH;
        if (particles[i].y > SCREEN_HEIGHT) particles[i].y = 0;
        if (particles[i].y < 0) particles[i].y = SCREEN_HEIGHT;
        
        // Pulse opacity based on time
        float alpha = (sinf(GetTime() * 2.0f + i) * 0.5f + 0.5f) * 0.7f;
        
        // Draw particle with glow effect
        DrawCircleV(particles[i], particleSizes[i] * 2, ColorAlpha(particleColors[i], alpha * 0.3f));
        DrawCircleV(particles[i], particleSizes[i], ColorAlpha(particleColors[i], alpha));
    }
}
// Add this utility function
Color GetCyberpunkRainbow(float time) {
    const float frequency = 0.3f;
    
    // Cycle through RGB values using sine waves at different phases
    unsigned char r = (unsigned char)(sinf(frequency * time + 0) * 127 + 128);
    unsigned char g = (unsigned char)(sinf(frequency * time + 2) * 127 + 128);
    unsigned char b = (unsigned char)(sinf(frequency * time + 4) * 127 + 128);
    
    return (Color){ r, g, b, 255 };
}
// Add this function for background tetrominoes
void DrawBackgroundTetrominoes() {
    static struct {
        int shape[4][4];
        float x;
        float y;
        float rotation;
        float speed;
        Color color;
        float alpha;
    } bgPieces[8] = {0};
    static bool initialized = false;
    
    // Initialize on first run
    if (!initialized) {
        for (int i = 0; i < 8; i++) {
            // Copy a random tetromino shape
            TetrominoType type = GetRandomValue(0, 6);
            memcpy(bgPieces[i].shape, game.tetrominoes[type].shape, sizeof(bgPieces[i].shape));
            
            bgPieces[i].x = GetRandomValue(0, SCREEN_WIDTH);
            bgPieces[i].y = GetRandomValue(-200, -50);
            bgPieces[i].rotation = GetRandomValue(0, 360);
            bgPieces[i].speed = GetRandomValue(10, 30) / 10.0f;
            bgPieces[i].color = game.tetrominoes[type].color;
            bgPieces[i].alpha = GetRandomValue(5, 15) / 100.0f; // 0.05-0.15 alpha
        }
        initialized = true;
    }
    
    // Update and draw each piece
    for (int i = 0; i < 8; i++) {
        // Update position
        bgPieces[i].y += bgPieces[i].speed * GetFrameTime();
        bgPieces[i].rotation += 10.0f * GetFrameTime();
        
        // Reset if off screen
        if (bgPieces[i].y > SCREEN_HEIGHT + 100) {
            bgPieces[i].y = GetRandomValue(-200, -50);
            bgPieces[i].x = GetRandomValue(0, SCREEN_WIDTH);
        }
        
        // Draw the piece with rotation
        float blockSize = 20 * scaleX;
        
        // Draw rotated tetromino
        Vector2 center = {bgPieces[i].x, bgPieces[i].y};
        
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                if (bgPieces[i].shape[y][x]) {
                    Vector2 blockPos = {
                        (x - 1.5f) * blockSize,
                        (y - 1.5f) * blockSize
                    };
                    
                    // Rotate around center
                    float rot = bgPieces[i].rotation * DEG2RAD;
                    Vector2 rotatedPos = {
                        blockPos.x * cosf(rot) - blockPos.y * sinf(rot) + center.x,
                        blockPos.x * sinf(rot) + blockPos.y * cosf(rot) + center.y
                    };
                    
                    DrawRectangle(
                        rotatedPos.x, rotatedPos.y,
                        blockSize, blockSize,
                        ColorAlpha(bgPieces[i].color, bgPieces[i].alpha)
                    );
                }
            }
        }
    }
}

void DrawMainMenu() {
    // Background with gradients
    DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), 
                        CYBER_BLACK, 
                        CYBER_DARKBLUE);

    // Add falling tetromino silhouettes in background
    DrawBackgroundTetrominoes();

    // Add cyber grid for background effect
    float gridTime = GetTime() * 0.3f;
    int gridSize = ScaleY(70);
    float gridAlpha = 0.12f;
    
    // Horizontal grid lines with movement effect
    for(int i = 0; i < SCREEN_HEIGHT / gridSize + 1; i++) {
        float y = (float)i * gridSize + fmodf(gridTime * 30, gridSize);
        float alpha = gridAlpha * (1.0f - (y / SCREEN_HEIGHT) * 0.8f);
        DrawLineEx(
            (Vector2){0, y},
            (Vector2){SCREEN_WIDTH, y},
            1.0f,
            ColorAlpha(NEON_BLUE, alpha)
        );
    }

    // Vertical grid lines
    for(int i = 0; i < SCREEN_WIDTH / gridSize + 1; i++) {
        float x = (float)i * gridSize;
        DrawLineEx(
            (Vector2){x, 0},
            (Vector2){x, SCREEN_HEIGHT},
            1.0f,
            ColorAlpha(NEON_BLUE, gridAlpha * 0.5f)
        );
    }
    
    // Add scanlines effect
    for(int i = 0; i < SCREEN_HEIGHT; i += 4) {
        DrawRectangle(0, i, SCREEN_WIDTH, 1, ColorAlpha(BLACK, 0.07f));
    }
    
    // Add floating particles
    DrawMenuParticles();
    
    // Calculate title position and scale
    const char* titleText = "RETRO TETRIS";
    float titleY = ScaleY(60);
    float titleFontSize = ScaleFontSize(90);
    Vector2 titleSize;
    
    if (game.customFontLoaded) {
        titleSize = MeasureTextEx(game.gameFont, titleText, titleFontSize, 2);
    } else {
        titleSize = (Vector2){ MeasureText(titleText, 90), 90 };
    }
    
    // Calculate logo position and size
    float logoY = titleY + titleSize.y + ScaleY(30); 
    float logoScale = GetScreenWidth() * 0.0005f;
    logoScale = ClampValue(logoScale, 0.4f, 0.7f);
    float logoWidth = 0;
    float logoHeight = 0;
    
    if (logoLoaded) {
        logoWidth = engineLogo.width * logoScale;
        logoHeight = engineLogo.height * logoScale;
    }
    
    // Calculate menu dimensions
    int menuWidth = ScaleX(500);
    int menuHeight = ScaleY(400);
    int menuX = GetScreenWidth()/2 - menuWidth/2;
    
    // Position menu with proper spacing
    int menuY;
    if (logoLoaded) {
        menuY = logoY + logoHeight + ScaleY(50);
    } else {
        menuY = titleY + titleSize.y + ScaleY(70);
    }
    
    // Make sure menu doesn't go off-screen
    if (menuY + menuHeight > GetScreenHeight() - ScaleY(80)) {
        menuY = GetScreenHeight() - menuHeight - ScaleY(80);
    }
    
    // Glowing header backdrop
    float headerGlow = sinf(GetTime() * 1.5f) * 0.2f + 0.8f;
    float headerHeight = titleY + titleSize.y;
    
    // Header glow effect
    for (int i = 0; i < 3; i++) {
        float glowSize = ScaleY(20.0f - i * 6.0f);
        DrawRectangle(
            0, 
            headerHeight - glowSize,
            SCREEN_WIDTH, 
            glowSize * 2,
            ColorAlpha(NEON_PINK, 0.03f * headerGlow * (3-i)/3)
        );
    }
    
    // Title with dynamic rainbow color
    if (game.customFontLoaded) {
        // Draw glitch effects for cyberpunk feel
        for (int i = 0; i < 3; i++) {
            float glitchX = sinf(GetTime() * (8.0f + i)) * 4.0f;
            float glitchY = cosf(GetTime() * (6.0f + i)) * 2.0f;
            Color glitchColor = i == 0 ? NEON_BLUE : (i == 1 ? NEON_PINK : NEON_PURPLE);
            
            DrawTextEx(
                game.gameFont, titleText,
                (Vector2){ 
                    GetScreenWidth()/2 - titleSize.x/2 + glitchX, 
                    titleY + glitchY
                },
                titleFontSize, 2, 
                ColorAlpha(glitchColor, 0.3f)
            );
        }
        
        // Draw main title with rainbow color effect
        Color rainbowColor = GetCyberpunkRainbow(GetTime()); 
        DrawTextEx(
            game.gameFont, titleText,
            (Vector2){ GetScreenWidth()/2 - titleSize.x/2, titleY },
            titleFontSize, 2, 
            rainbowColor
        );
    } else {
        // Fallback to standard font with rainbow color
        Color rainbowColor = GetCyberpunkRainbow(GetTime());
        DrawText(
            titleText,
            GetScreenWidth()/2 - titleSize.x/2,
            titleY,
            90,
            rainbowColor
        );
    }
    
    // Draw Phantom logo with enhanced effect
    if (logoLoaded) {
        // Add glow effect
        float glowIntensity = sinf(GetTime() * 1.5f) * 0.3f + 0.7f;
        float logoX = GetScreenWidth() / 2 - logoWidth / 2;
        
        // Draw logo glow
        for (int i = 0; i < 3; i++) {
            float glowSize = (i + 1) * 5.0f;
            DrawTexturePro(
                engineLogo,
                (Rectangle){ 0, 0, engineLogo.width, engineLogo.height },
                (Rectangle){ 
                    logoX - glowSize, 
                    logoY - glowSize, 
                    logoWidth + glowSize * 2, 
                    logoHeight + glowSize * 2 
                },
                (Vector2){ 0, 0 },
                0.0f,
                ColorAlpha(WHITE, 0.05f * glowIntensity * (3-i))
            );
        }
        
        // Draw the logo with pulsing effect
        float pulseFactor = sinf(GetTime() * 1.5f) * 0.05f + 0.95f;
        
        DrawTexturePro(
            engineLogo,
            (Rectangle){ 0, 0, engineLogo.width, engineLogo.height },
            (Rectangle){ logoX, logoY, logoWidth * pulseFactor, logoHeight * pulseFactor },
            (Vector2){ 0, 0 },
            0.0f,
            WHITE
        );
    }
    
    // Menu panel background with dynamic gradient
    float gradientShift = sinf(GetTime() * 0.5f) * 0.2f + 0.8f;
    DrawRectangleGradientV(
        menuX, menuY,
        menuWidth, menuHeight,
        ColorAlpha(CYBER_DARKBLUE, 0.7f),
        ColorAlpha((Color){(int)(CYBER_BLACK.r * gradientShift), 
                           (int)(CYBER_BLACK.g * gradientShift), 
                           (int)(CYBER_BLACK.b * gradientShift), 
                           CYBER_BLACK.a}, 0.7f)
    );
    
    // Menu border with enhanced glow effect
    float borderGlow = sinf(GetTime() * 2.0f) * 0.3f + 0.7f;
    
    // Add multi-layer border for depth
    for (int i = 0; i < 3; i++) {
        int offset = i * 2;
        DrawRectangleLines(
            menuX - offset, menuY - offset,
            menuWidth + offset * 2, menuHeight + offset * 2,
            ColorAlpha(NEON_BLUE, borderGlow * 0.2f * (3-i)/3)
        );
    }
    
    // Main border
    DrawRectangleLines(
        menuX, menuY,
        menuWidth, menuHeight,
        ColorAlpha(NEON_BLUE, borderGlow * 0.7f)
    );
    
    // Add diagonal corners for cyberpunk effect
    int cornerSize = 20;
    DrawLineEx(
        (Vector2){menuX, menuY + cornerSize},
        (Vector2){menuX + cornerSize, menuY},
        2.0f, ColorAlpha(NEON_BLUE, 0.9f)
    );
    DrawLineEx(
        (Vector2){menuX + menuWidth, menuY + cornerSize},
        (Vector2){menuX + menuWidth - cornerSize, menuY},
        2.0f, ColorAlpha(NEON_BLUE, 0.9f)
    );
    DrawLineEx(
        (Vector2){menuX, menuY + menuHeight - cornerSize},
        (Vector2){menuX + cornerSize, menuY + menuHeight},
        2.0f, ColorAlpha(NEON_BLUE, 0.9f)
    );
    DrawLineEx(
        (Vector2){menuX + menuWidth, menuY + menuHeight - cornerSize},
        (Vector2){menuX + menuWidth - cornerSize, menuY + menuHeight},
        2.0f, ColorAlpha(NEON_BLUE, 0.9f)
    );
    
    // Menu items with enhanced effects
    const char* menuItems[] = {
        "PLAY GAME",
        "AUDIO SETTINGS",
        "AUTO-PLAY SETTINGS",
        "HOW TO PLAY",
        "EXIT GAME"
    };
    int numMenuItems = 5;
    
    // Calculate item spacing to fit menu height
    float itemSpacing = (menuHeight - ScaleY(100)) / (numMenuItems - 0.5f);
    float firstItemY = menuY + ScaleY(60);
    
    for (int i = 0; i < numMenuItems; i++) {
        bool isSelected = (i == mainMenuSelection);
        
        // Calculate item position with even spacing
        float itemY = firstItemY + (i * itemSpacing);
        
        // Enhanced selection effects
        if (isSelected) {
            // Variable glow effect
            float highlightPulse = sinf(GetTime() * 3.0f) * 0.3f + 0.7f;
            
            // Add multi-layer glow for selected item
            for (int j = 0; j < 3; j++) {
                float distortion = sinf(GetTime() * 5.0f + j) * 3.0f;
                DrawRectangleLines(
                    menuX + 30 - j * 2 + (j == 0 ? distortion : 0),
                    itemY - 15 - j * 2,
                    menuWidth - 60 + j * 4,
                    50 + j * 4,
                    ColorAlpha(NEON_GREEN, highlightPulse * 0.1f * (3-j))
                );
            }
            
            // Draw selection background with dynamic gradient
            DrawRectangleGradientH(
                menuX + 30,
                itemY - 15,
                menuWidth - 60,
                50,
                ColorAlpha(NEON_BLUE, highlightPulse * 0.2f),
                ColorAlpha(CYBER_BLACK, 0.1f)
            );
            
            // Enhanced arrow indicators with motion
            float arrowOffset = sinf(GetTime() * 5.0f) * 5.0f;
            float arrowPulse = sinf(GetTime() * 8.0f) * 0.2f + 0.8f;
            
            // Draw arrow with glow
            DrawText(">", 
                     menuX + 65 - arrowOffset, 
                     itemY, 
                     30, ColorAlpha(NEON_GREEN, highlightPulse));
            
            // Glow layer
            DrawText(">", 
                     menuX + 65 - arrowOffset, 
                     itemY, 
                     30 + 4, ColorAlpha(NEON_GREEN, highlightPulse * 0.3f * arrowPulse));
            
            // Right arrow
            DrawText("<", 
                     menuX + menuWidth - 65 + arrowOffset, 
                     itemY, 
                     30, ColorAlpha(NEON_GREEN, highlightPulse));
            
            // Right arrow glow
            DrawText("<", 
                     menuX + menuWidth - 65 + arrowOffset, 
                     itemY, 
                     30 + 4, ColorAlpha(NEON_GREEN, highlightPulse * 0.3f * arrowPulse));
        }
        
        // Draw menu text with enhanced effects
        if (game.customFontLoaded) {
            float scale = isSelected ? 1.1f + sinf(GetTime() * 5.0f) * 0.05f : 1.0f;
            Vector2 textSize = MeasureTextEx(game.gameFont, menuItems[i], 30 * scale, 2);
            
            // Choose dynamic color for selected item
            Color itemColor;
            if (isSelected) {
                // Subtle color cycling for selected item
                float colorShift = sinf(GetTime() * 2.0f) * 0.1f + 0.9f;
                itemColor = (Color){
                    (unsigned char)(NEON_GREEN.r * colorShift),
                    (unsigned char)(NEON_GREEN.g * colorShift),
                    (unsigned char)(NEON_GREEN.b * colorShift),
                    255
                };
                
                // Add text shadow/glow for selected item
                DrawTextEx(
                    game.gameFont, menuItems[i],
                    (Vector2){ menuX + menuWidth/2 - textSize.x/2 + 2, itemY + 2 },
                    30 * scale, 2, 
                    ColorAlpha(CYBER_BLACK, 0.7f)
                );
            } else {
                itemColor = WHITE;
            }
            
            DrawTextEx(
                game.gameFont, menuItems[i],
                (Vector2){ menuX + menuWidth/2 - textSize.x/2, itemY },
                30 * scale, 2, 
                itemColor
            );
        } else {
            float scale = isSelected ? 1.1f + sinf(GetTime() * 5.0f) * 0.05f : 1.0f;
            int textWidth = MeasureText(menuItems[i], 30 * scale);
            Color itemColor = isSelected ? NEON_GREEN : WHITE;
            
            // Add shadow for selected items
            if (isSelected) {
                DrawText(
                    menuItems[i],
                    menuX + menuWidth/2 - textWidth/2 + 2,
                    itemY + 2,
                    30 * scale,
                    ColorAlpha(CYBER_BLACK, 0.7f)
                );
            }
            
            DrawText(
                menuItems[i],
                menuX + menuWidth/2 - textWidth/2,
                itemY,
                30 * scale,
                itemColor
            );
        }
    }
    
    // Draw footer with enhanced effect
    const char* footerText = "POWERED BY BISMAYA PHANTOM ENGINE";
    float footerPulse = sinf(GetTime() * 1.5f) * 0.3f + 0.7f;
    
    if (game.customFontLoaded) {
        Vector2 textSize = MeasureTextEx(game.gameFont, footerText, 20, 2);
        DrawTextEx(
            game.gameFont, footerText,
            (Vector2){ SCREEN_WIDTH/2 - textSize.x/2, SCREEN_HEIGHT - 50 },
            20, 2, 
            ColorAlpha(NEON_BLUE, footerPulse * 0.7f)
        );
    } else {
        int textWidth = MeasureText(footerText, 20);
        DrawText(
            footerText,
            SCREEN_WIDTH/2 - textWidth/2,
            SCREEN_HEIGHT - 50,
            20,
            ColorAlpha(NEON_BLUE, footerPulse * 0.7f)
        );
    }
}




void HandleMainMenuInput() {
    // Handle mouse input
    bool mouseClicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    
    // Calculate the same positions as in DrawMainMenu for consistency
    const char* titleText = "RETRO TETRIS";
    float titleY = ScaleY(60);
    float titleFontSize = ScaleFontSize(90);
    Vector2 titleSize;
    
    if (game.customFontLoaded) {
        titleSize = MeasureTextEx(game.gameFont, titleText, titleFontSize, 2);
    } else {
        titleSize = (Vector2){ MeasureText(titleText, 90), 90 };
    }
    
    // Calculate logo position and size (if available)
    float logoY = titleY + titleSize.y + ScaleY(30);
    float logoScale = GetScreenWidth() * 0.0005f;
    logoScale = ClampValue(logoScale, 0.4f, 0.7f);
    float logoHeight = 0;
    
    if (logoLoaded) {
        logoHeight = engineLogo.height * logoScale;
    }
    
    // Calculate menu dimensions
    int menuWidth = ScaleX(500);
    int menuHeight = ScaleY(400);
    int menuX = GetScreenWidth()/2 - menuWidth/2;
    
    // Position menu with proper spacing below logo (or title if no logo)
    int menuY;
    if (logoLoaded) {
        menuY = logoY + logoHeight + ScaleY(50);
    } else {
        menuY = titleY + titleSize.y + ScaleY(70);
    }
    
    // Make sure menu doesn't go off-screen
    if (menuY + menuHeight > GetScreenHeight() - ScaleY(80)) {
        menuY = GetScreenHeight() - menuHeight - ScaleY(80);
    }
    
    // Calculate item spacing to match DrawMainMenu
    int numMenuItems = 5;
    float itemSpacing = (menuHeight - ScaleY(100)) / (numMenuItems - 0.5f);
    float firstItemY = menuY + ScaleY(60);
    
    // Check mouse hover over menu items
    for (int i = 0; i < numMenuItems; i++) {
        float itemY = firstItemY + (i * itemSpacing);
        
        Rectangle itemRect = {
            (float)menuX + 30,
            itemY - 15,
            (float)(menuWidth - 60),
            50.0f
        };
        
        if (IsMouseOverRect(itemRect)) {
            if (mainMenuSelection != i) {
                mainMenuSelection = i;
                PlayGameSound(game.menuSound);
            }
            
            if (mouseClicked) {
                ProcessMainMenuSelection();
                break;
            }
        }
    }
    
    // Keyboard controls
    if (IsKeyPressed(KEY_UP)) {
        mainMenuSelection--;
        if (mainMenuSelection < 0) mainMenuSelection = 4;
        PlayGameSound(game.menuSound);
    }
    
    if (IsKeyPressed(KEY_DOWN)) {
        mainMenuSelection++;
        if (mainMenuSelection > 4) mainMenuSelection = 0;
        PlayGameSound(game.menuSound);
    }
    
    // Make selection
    if (IsKeyPressed(KEY_ENTER)) {
        ProcessMainMenuSelection();
    }
}

void ProcessMainMenuSelection() {
    PlayGameSound(game.menuSound);
    
    switch (mainMenuSelection) {
        case 0: // Play Game
            {   
                // Remember if auto-play is enabled before initializing the game
                bool autoPlayWasEnabled = game.autoPlayEnabled;
                InitGame();
                game.state = STATE_PLAYING;
                // Re-enable auto-play if it was enabled before
                if (autoPlayWasEnabled) {
                    game.autoPlayEnabled = true;
                    
                    // Reset auto-play timer to prevent immediate move
                    game.timer = 0;
                    
                    // Show an indicator that auto-play is active
                    AddFloatingText("AUTO-PLAY ACTIVE", 
                                (Vector2){SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 50},
                                NEON_GREEN, 1.5f);
                }
            }
            break;
            
        case 1: // Audio Settings
            game.showAudioSettings = true;
            game.audioMenuSelection = 0;
            break;

        case 2: // Auto-Play Settings
            game.showAutoPlaySettings = true;
            game.autoPlayMenuSelection = 0;
            break;
            
        case 3: // How to Play - We'll just show a simple message
            // In a full implementation, you might have a separate help screen
            AddFloatingText("USE ARROW KEYS & SPACE TO PLAY", 
                          (Vector2){SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 150}, 
                          NEON_GREEN, 1.5f);
            break;
            
        case 4: // Exit Game
            // Set flag to close window - this is safer than directly closing
            shouldExitGame = true;
            break;
    }
}