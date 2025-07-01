#include "iGraphics.h"
#include "iSound.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>   

// Game states using enum
typedef enum
{
    STATE_MAIN_MENU,
    STATE_GAME,
    STATE_INSTRUCTIONS,
    STATE_SETTINGS,
    STATE_PAUSE,
    STATE_GAMEOVER,
    STATE_VICTORY,
    STATE_EXIT,
    STATE_GAME_OVER,
} GameState;

GameState currentState = STATE_MAIN_MENU;
bool bgmplaying = false;

// Ball
float ballX = 200, ballY = 300;
float ballRadius = 15;
float ballDY = 0;  // Ball's vertical speed
float gravity = -1;
bool onGround = false;  // Check if the ball is on the ground

// Score variables
int score = 0;  // Variable to keep track of the player's score

// Map dimensions and 2D array for map layout
#define MAX_MAP_WIDTH 65
#define MAX_MAP_HEIGHT 8
char gameMap[MAX_MAP_HEIGHT][MAX_MAP_WIDTH] = {
    "################################################################",
    "##...######.....P......###.........##..........................#",
    "##...######............###....P...........P....................#",
    "##...######...####.....###.........c...........................#",
    "##...######...####.....###..####..####..####......c....P.......#",
    "##............####..........##     ## ....##...................#",
    "##o......c....####.|....P...##...|....|...##.....|..|.....|...G#",
    "################################################################"
};
void updateCameraPosition();
// Camera position
float cameraX = 0, cameraY = 0;
int blockSize = 75;
int visibleWidth = 1000;
int visibleHeight = 600;

// Button variables for Main Menu
int btnX = 100, btnY = 100, btnW = 200, btnH = 50, gap = 20;

void drawMap() {
    for (int y = 0; y < MAX_MAP_HEIGHT; y++) {
        for (int x = 0; x < MAX_MAP_WIDTH; x++) {
            char currentChar = gameMap[y][x];
            int posX = (x * blockSize) - cameraX;  // Apply cameraX for horizontal camera movement
            int posY = ((MAX_MAP_HEIGHT - y - 1) * blockSize) - cameraY;  // Apply cameraY for vertical camera movement

            if (currentChar == '#') {
                iShowImage(posX, posY, "assets/images/block.bmp");
            } else if (currentChar == 'P') {
                iShowImage(posX, posY, "assets/images/ring-main.bmp");
            } else if (currentChar == 'c') {
                iSetColor(255, 255, 0);
                iFilledCircle(posX + blockSize / 2, posY + blockSize / 2, blockSize / 4);
            } else if (currentChar == 'G') {
                iSetColor(0, 255, 0);
                iFilledCircle(posX + blockSize / 2, posY + blockSize / 2, blockSize / 2);
            }
            else if(currentChar == '|'){
                 iShowImage(posX, posY, "assets/images/spike-main.bmp");
            }
        }
    }
}

// Update camera position to ensure it stays within map bounds
void updateCameraPosition()
{
    float centerX = visibleWidth / 2.0f;
    float leftBound = centerX - 200;
    float rightBound = centerX + 200;

    if (ballX - cameraX < leftBound)
        cameraX = ballX - leftBound;
    else if (ballX - cameraX > rightBound)
        cameraX = ballX - rightBound;

    // Clamp camera within map bounds
    cameraX = fmax(0, fmin(cameraX, MAX_MAP_WIDTH * blockSize - visibleWidth));
    cameraY = fmax(0, fmin(cameraY, MAX_MAP_HEIGHT * blockSize - visibleHeight));
}

// Ensure the ball stays within the screen bounds and doesn't go out
void limitBallPosition() {
    if (ballX - ballRadius < cameraX) ballX = cameraX + ballRadius + 300;
    if (ballX + ballRadius > cameraX + visibleWidth) ballX = cameraX + visibleWidth - ballRadius - 300;
    if (ballY - ballRadius < cameraY) ballY = cameraY + ballRadius;
    if (ballY + ballRadius > cameraY + visibleHeight) ballY = cameraY + visibleHeight - ballRadius;
    if (ballY < ballRadius) {
        ballY = ballRadius;  // Place the ball on top of the ground
        ballDY = 0;          // Stop the ball from falling
        onGround = true;     // Set the ball as on the ground
    }
}
void checkSpikeCollision() {
    int tileX = (int)(ballX / blockSize); 
    int tileYBelow = (int)((MAX_MAP_HEIGHT * blockSize - (ballY - ballRadius)) / blockSize); // Check below the ball
    int tileYAbove = (int)((MAX_MAP_HEIGHT * blockSize - (ballY + ballRadius)) / blockSize); // Check above the ball
    int tileXLeft = (int)((ballX - ballRadius) / blockSize); // Check left of the ball
    int tileXRight = (int)((ballX + ballRadius) / blockSize); // Check right of the ball

    // Vertical collision with spikes (below the ball)
    if (tileYBelow >= 0 && tileYBelow < MAX_MAP_HEIGHT && tileX >= 0 && tileX < MAX_MAP_WIDTH) {
        if (gameMap[tileYBelow][tileX] == '|') { // Spike collision below
            currentState = STATE_GAME_OVER; // Trigger game over
        }
    }

    // Vertical collision with spikes (above the ball)
    if (tileYAbove >= 0 && tileYAbove < MAX_MAP_HEIGHT && tileX >= 0 && tileX < MAX_MAP_WIDTH) {
        if (gameMap[tileYAbove][tileX] == '|') { // Spike collision above
            currentState = STATE_GAME_OVER; // Trigger game over
        }
    }

    // Horizontal collision with spikes (left side of the ball)
    if (tileYBelow >= 0 && tileYBelow < MAX_MAP_HEIGHT && tileXLeft >= 0 && tileXLeft < MAX_MAP_WIDTH) {
        if (gameMap[tileYBelow][tileXLeft] == '|') { // Spike collision left
            currentState = STATE_GAME_OVER; // Trigger game over
        }
    }

    // Horizontal collision with spikes (right side of the ball)
    if (tileYBelow >= 0 && tileYBelow < MAX_MAP_HEIGHT && tileXRight >= 0 && tileXRight < MAX_MAP_WIDTH) {
        if (gameMap[tileYBelow][tileXRight] == '|') { // Spike collision right
            currentState = STATE_GAME_OVER; // Trigger game over
        }
    }
}
void checkVictory() {
    int tileX = (int)(ballX / blockSize);
    int tileY = (int)((MAX_MAP_HEIGHT * blockSize - ballY) / blockSize);

    // Check if the ball reaches the goal 'G' (green circle)
    if (tileY >= 0 && tileY < MAX_MAP_HEIGHT && tileX >= 0 && tileX < MAX_MAP_WIDTH) {
        if (gameMap[tileY][tileX] == 'G') { // Victory condition reached
            currentState = STATE_VICTORY; // Trigger victory state
        }
    }
}

void iDraw()
{
    iClear();
    //iShowImage(0, 0, "wallpaper/wallpaper.bmp");

    if (!bgmplaying)
    {
        iPlaySound("assets/sounds/game_audio.wav", true);
        bgmplaying = true;
    }

    if (currentState == STATE_MAIN_MENU)
    {
         iClear();
         iShowImage(0, 0, "assets/images/wallpaper.bmp");
        iSetColor(0, 255, 0);
        iFilledRectangle(btnX, btnY, btnW, btnH);
        iSetColor(0, 0, 0);
        iText(btnX + 70, btnY + 15, "Start", GLUT_BITMAP_HELVETICA_18);

        iSetColor(0, 200, 255);
        iFilledRectangle(btnX + 1 * (btnW + gap), btnY, btnW, btnH);
        iSetColor(0, 0, 0);
        iText(btnX + 1 * (btnW + gap) + 40, btnY + 15, "Instructions", GLUT_BITMAP_HELVETICA_18);

        iSetColor(255, 165, 0);
        iFilledRectangle(btnX + 2 * (btnW + gap), btnY, btnW, btnH);
        iSetColor(0, 0, 0);
        iText(btnX + 2 * (btnW + gap) + 60, btnY + 15, "Settings", GLUT_BITMAP_HELVETICA_18);

        iSetColor(255, 0, 0);
        iFilledRectangle(btnX + 3 * (btnW + gap), btnY, btnW, btnH);
        iSetColor(0, 0, 0);
        iText(btnX + 3 * (btnW + gap) + 70, btnY + 15, "Exit", GLUT_BITMAP_HELVETICA_18);
    }

    else if (currentState == STATE_GAME)
    {
        iClear();
        iShowImage(0, 0, "assets/images/level1.bmp");
        drawMap();

        ballDY += gravity;
        ballY += ballDY;

        // --- Vertical Collision Detection ---
        int tileX = (int)(ballX / blockSize);
        int tileYBelow = (int)((MAX_MAP_HEIGHT * blockSize - (ballY - ballRadius)) / blockSize);
        int tileYAbove = (int)((MAX_MAP_HEIGHT * blockSize - (ballY + ballRadius)) / blockSize);

        if (tileYBelow >= 0 && tileYBelow < MAX_MAP_HEIGHT && tileX >= 0 && tileX < MAX_MAP_WIDTH)
        {
            if (gameMap[tileYBelow][tileX] == '#')
            {
                ballY = (MAX_MAP_HEIGHT - tileYBelow) * blockSize + ballRadius;
                ballDY = 0;
                onGround = true;
            }
            else
            {
                onGround = false;
            }
        }

        if (tileYAbove >= 0 && tileYAbove < MAX_MAP_HEIGHT && tileX >= 0 && tileX < MAX_MAP_WIDTH)
        {
            if (gameMap[tileYAbove][tileX] == '#')
            {
                ballY = (MAX_MAP_HEIGHT - tileYAbove - 1) * blockSize - ballRadius;
                if (ballDY > 0)
                    ballDY = 0;
            }
        }

        // --- Horizontal Collision Detection ---
        int tileYCenter = (int)((MAX_MAP_HEIGHT * blockSize - ballY) / blockSize);
        int tileXLeft = (int)((ballX - ballRadius) / blockSize);
        int tileXRight = (int)((ballX + ballRadius) / blockSize);

        if (tileYCenter >= 0 && tileYCenter < MAX_MAP_HEIGHT && tileXLeft >= 0 && tileXLeft < MAX_MAP_WIDTH)
        {
            if (gameMap[tileYCenter][tileXLeft] == '#')
            {
                ballX = (tileXLeft + 1) * blockSize + ballRadius;
            }
        }

        if (tileYCenter >= 0 && tileYCenter < MAX_MAP_HEIGHT && tileXRight >= 0 && tileXRight < MAX_MAP_WIDTH)
        {
            if (gameMap[tileYCenter][tileXRight] == '#')
            {
                ballX = tileXRight * blockSize - ballRadius;
            }
        }

         checkSpikeCollision();
             checkVictory();
        if (ballY < ballRadius)
        {
            ballY = ballRadius;
            ballDY = 0;
            onGround = true;
        }

        limitBallPosition();
        updateCameraPosition();

        iSetColor(255, 0, 0);
        iFilledCircle(ballX - cameraX, ballY - cameraY, ballRadius);

        char scoreText[20];
        sprintf(scoreText, "Score: %d", score);
        iSetColor(255, 255, 255);
        iText(20, 560, scoreText, GLUT_BITMAP_HELVETICA_18);
    }

    else if (currentState == STATE_INSTRUCTIONS)
    {
         iClear();
         iShowImage(0, 0, "assets/images/instruction-main.bmp");
         iSetColor(123, 187, 23);
         iFilledRectangle(120, 300, 430, 120);
         iSetColor(0, 0, 0);
         iText(120, 400, "Instructions:", GLUT_BITMAP_TIMES_ROMAN_24);
         iText(120, 370, "- Use LEFT and RIGHT arrow keys to move the ball.", GLUT_BITMAP_HELVETICA_18);
         iText(120, 340, "- Prevent the ball from falling below.", GLUT_BITMAP_HELVETICA_18);
         iText(120, 310, "- Press 'b' to return to the Main Menu.", GLUT_BITMAP_HELVETICA_18);
    }
    else if(currentState == STATE_SETTINGS)
    {
        iClear();
        iShowImage(0, 0, "assets/images/settings-main.bmp");
    }

    else if (currentState == STATE_PAUSE)
    {
        iSetColor(255, 255, 0);
        iText(370, 260, "Game Paused. Press 'r' to Resume or 'b' to go to Menu.", GLUT_BITMAP_HELVETICA_18);
    }
    else if (currentState == STATE_GAME_OVER)
    {
        iShowImage(0, 0, "assets/images/game-over.bmp");
    }

    else if (currentState == STATE_VICTORY)
    {
        iShowImage(0, 0, "assets/images/victory.bmp");
    }
}

void iMouseMove(int mx, int my) {}
void iMouseDrag(int mx, int my) {}
void iMouseWheel(int dir, int mx, int my) {}

void iMouse(int button, int state, int mx, int my)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        if (currentState == STATE_MAIN_MENU)
        {
            if (mx >= btnX && mx <= btnX + btnW && my >= btnY && my <= btnY + btnH)
            {
                currentState = STATE_GAME;
            }
            else if (mx >= btnX + 1 * (btnW + gap) && mx <= btnX + 1 * (btnW + gap) + btnW &&
                     my >= btnY && my <= btnY + btnH)
            {
                currentState = STATE_INSTRUCTIONS;
            }
            else if (mx >= btnX + 2 * (btnW + gap) && mx <= btnX + 2 * (btnW + gap) + btnW &&
                     my >= btnY && my <= btnY + btnH)
            {
                currentState = STATE_SETTINGS;
            }
            else if (mx >= btnX + 3 * (btnW + gap) && mx <= btnX + 3 * (btnW + gap) + btnW &&
                     my >= btnY && my <= btnY + btnH)
            {
                exit(0);
            }
        }
    }
}

void iSpecialKeyboard(unsigned char key)
{
    int moveSpeed = 10;

    // Only move ball, do NOT move camera manually here
    if (key == GLUT_KEY_LEFT)
    {
        ballX -= moveSpeed;
    }
    if (key == GLUT_KEY_RIGHT)
    {
        ballX += moveSpeed;
    }
}

void iKeyboard(unsigned char key)
{
    if (key == 'b')
    {
        currentState = STATE_MAIN_MENU;
        ballX = 200;
        ballY = 300;
        ballDY = 0;
    }

    if (key == ' ' && onGround)
    {
        ballDY = 25;
        onGround = false;
    }
    if (currentState == STATE_GAME_OVER && key == 'b')  
    {
        currentState = STATE_MAIN_MENU;  
        ballX = 200;  
        ballY = 300;
        ballDY = 0;  
    }

    if (currentState == STATE_PAUSE && key == 'r')  
    {
        currentState = STATE_GAME;  
    }
}


int gameFPS = 60; 

void timer(int value) {
   
    iDraw();

    // Request next update
    glutTimerFunc(1000 / gameFPS, timer, 0);  // 1000 / gameFPS ms = ~16.67 ms

    
}

int main(int argc, char *argv[])
{
    printf("Starting game...\n");

    glutInit(&argc, argv);
    iInitializeSound();
    iInitialize(1000, 600, "Bounce Classic");

    
    glutTimerFunc(1000 / gameFPS, timer, 0);  

    glutMainLoop();
    return 0;
}
