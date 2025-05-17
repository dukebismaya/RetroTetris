#include "../include/bismayalib.h"

Game game;
Shader glowShader;
Shader blockShader;
int glowTimeLocation;
int blockTimeLocation;
FloatingText floatingTexts[MAX_FLOATING_TEXT];
float splashTimer;
int mainMenuSelection;
Texture2D engineLogo;
bool logoLoaded;
bool shouldExitGame;
float scaleX;
float scaleY;
float BOARD_OFFSET_X;
float BOARD_OFFSET_Y;