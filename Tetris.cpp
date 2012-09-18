#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"
#include "timer.h"
#include <vector>
#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>


// ADD TEXT TO THE PAUSE FUNCTION TO INDICATE WHEN PAUSED

// Make the game dimensions correct

// ADD FILE I/O AND KEEP TRACK OF SCORES

// ADD IN A MENU/TITLE SCREEN WHERE OPTIONS CAN BE CHANGED

// EITHER REMOVE TETROMINOE ARRAY COMPLETELY AND ALTERNATE BETWEEN
// TWO INDIVIDUAL TETROMINOES, OR MAKE IT RECYCLE WHEN MAX VALUE
// IS REACHED

// At most there can be 440 / 20 x 400 / 20 = 440 blocks on
// screen at once. Each tetrominoe is made up of 4 blocks.
// The can be 110 tetrominoes on the screen at once. This breaks
// down into 20 columns and 22 rows of 20x20 bricks, or,
// 11 rows and 10 columns

using namespace std;

const int SCREEN_WIDTH = 480;
const int SCREEN_HEIGHT = 640;
const int SCREEN_BPP = 32;
const int FPS = 60;
const int BRICK_SIZE = 20;
const int STARTING_ROWS = 5;

const int PLAY_AREA_X = 20;
const int PLAY_AREA_Y = 20;
const int PLAY_AREA_WIDTH = 410;
const int PLAY_AREA_HEIGHT = 450;
const int PLAY_AREA_BORDER = 5;


const int INFO_AREA_X = PLAY_AREA_X + PLAY_AREA_WIDTH + 15;
const int INFO_AREA_Y = PLAY_AREA_Y;
const int PREVIEW_X = INFO_AREA_X + 80;
const int PREVIEW_Y = INFO_AREA_Y + 40;
const int SCORE_X = INFO_AREA_X + 50;
const int SCORE_Y = 180;
const int LEVEL_X = INFO_AREA_X + 50;
const int LEVEL_Y = 280;
const int ROWS_X = INFO_AREA_X + 50;
const int ROWS_Y = 380;



const int FALL_SPEED = 50;
const int SIDE_SPEED = 5;

// The surfaces
SDL_Surface *screen = NULL;
SDL_Surface *background = NULL;
SDL_Surface *playArea = NULL;
SDL_Surface *infoArea = NULL;


SDL_Surface *yellowBrick = NULL;
SDL_Surface *blueBrick = NULL;
SDL_Surface *greenBrick = NULL;
SDL_Surface *greyBrick = NULL;
SDL_Surface *redBrick = NULL;

SDL_Surface *message = NULL;
SDL_Surface *score = NULL;
SDL_Surface *scoreValue = NULL;
SDL_Surface *level = NULL;
SDL_Surface *levelValue = NULL;
SDL_Surface *rows = NULL;
SDL_Surface *rowsValue = NULL;


TTF_Font *font = NULL;
TTF_Font *font2 = NULL;

SDL_Color fontColor = {0, 0, 0};

// Event handling
SDL_Event event;


class Tetrominoe{
    private:
        // Denotes what shape this instance takes
        int type;
        int colorType;
        int baseX;
        int baseY;
        int rotState;
        int x[4];
        int y[4];
        bool inPlay;
        // Each tetrominoe can be made with 4 blocks
        SDL_Surface *blocks[4];
    public:
        Tetrominoe(int, int);
        Tetrominoe(int, int, int, int);
        int getBaseX();
        int getBaseY();
        void rotate();
        void setBaseX(int);
        void setBaseY(int);
        void setInPlay(bool);
        void move();
        void show();
        void preview();
        bool isInPlay();
        void moveLeft();
        void moveRight();
        int handleInput();
        int getX(int);
        int getY(int);
        int getColorType();
};

class Grid{
    private:
    int x;
    int y;
    int type;
    bool full;
    SDL_Surface *block;
    public:
    Grid(int, int, int);
    int getType();
    int getX();
    int getY();
    void setX(int);
    void setY(int);
    void setType(int);
    bool isFull();
    void setFull(bool);
    void show();
};


class GameGrid{
    private:
    Grid *theGrid[440];
    public:
    GameGrid();
    void showAll();
    void setBlock(int, int, int);
    void setNewBlock(int, int, int, int);
    void setFull(int, bool);
    int getX(int);
    int getY(int);
    int getType(int);
    bool checkCollisionVert(int, int);
    bool getFull(int);
};


SDL_Surface* loadImage(std::string filename){
    SDL_Surface* loadedImage = NULL;
    SDL_Surface* optimizedImage = NULL;

    loadedImage = IMG_Load(filename.c_str());

    if (loadedImage != NULL){
        optimizedImage = SDL_DisplayFormat(loadedImage);
    }

    SDL_FreeSurface(loadedImage);

    return optimizedImage;
}

void applySurface(int x, int y, SDL_Surface* source, SDL_Surface* dest){
    SDL_Rect offset;

    offset.x = x;
    offset.y = y;
    SDL_BlitSurface(source, NULL, dest, &offset);
}

bool init(){
    if (SDL_Init(SDL_INIT_EVERYTHING)==-1){
        return false;
    }

    if (TTF_Init() == -1){
        return false;
    }

    screen = SDL_SetVideoMode(SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_BPP, SDL_SWSURFACE);

    if (screen == NULL){
        return false;
    }

    srand(SDL_GetTicks());

    SDL_WM_SetCaption("Tetris", NULL);
}

bool loadFiles(){
    background = loadImage("background.png");
    redBrick = loadImage("redbrick.png");
    blueBrick = loadImage("bluebrick.png");
    greyBrick = loadImage("greybrick.png");
    greenBrick = loadImage("greenbrick.png");
    yellowBrick = loadImage("yellowbrick.png");
    playArea = loadImage("playarea.png");
    infoArea = loadImage("infoarea.png");

    if (background == NULL || redBrick == NULL || blueBrick == NULL || greyBrick == NULL || greenBrick == NULL || yellowBrick == NULL || playArea == NULL || infoArea == NULL)
        return false;

    font = TTF_OpenFont("courbd.ttf", 18);
    font2 = TTF_OpenFont("courbd.ttf", 16);

    return true;
}

void cleanUp(){
    SDL_FreeSurface(background);
    SDL_FreeSurface(redBrick);

    SDL_Quit();
}

Tetrominoe::Tetrominoe(int bX, int bY){

    baseX = bX;
    baseY = bY;
    type =  rand() % 7; // generate random number from 0 to 6
    inPlay = false;
    rotState = 0;

    colorType = rand() % 5;

    for (int i = 0; i < 4; i++){
        switch (colorType){
        case 0:     blocks[i] = redBrick;
                    break;
        case 1:     blocks[i] = blueBrick;
                    break;
        case 2:     blocks[i] = greyBrick;
                    break;
        case 3:     blocks[i] = greenBrick;
                    break;
        case 4:     blocks[i] = yellowBrick;
                    break;
        default: ;
        }

    }
    // 0 - square
    if (type == 0){
        // top left
        x[0] = baseX;
        y[0] = baseY;
        // top right
        x[1] = baseX + BRICK_SIZE;
        y[1] = baseY;
        // bottom left
        x[2] = baseX;
        y[2] = baseY + BRICK_SIZE;
        // bottom right
        x[3] = baseX + BRICK_SIZE;
        y[3] = baseY + BRICK_SIZE;
    }
    // 1 - bar
    if (type == 1){
        x[0] = baseX;
        y[0] = baseY;
        x[1] = baseX;
        y[1] = baseY + BRICK_SIZE;
        x[2] = baseX;
        y[2] = baseY + BRICK_SIZE + BRICK_SIZE;
        x[3] = baseX;
        y[3] = baseY + BRICK_SIZE + BRICK_SIZE + BRICK_SIZE;
    }
    // 2 - s
    if (type == 2){
        x[0] = baseX;
        y[0] = baseY;
        x[1] = baseX - BRICK_SIZE;
        y[1] = baseY;
        x[2] = baseX - BRICK_SIZE;
        y[2] = baseY + BRICK_SIZE;
        x[3] = baseX - BRICK_SIZE - BRICK_SIZE;
        y[3] = baseY + BRICK_SIZE;
    }
    // 3 - z
    if (type == 3){
        x[0] = baseX;
        y[0] = baseY;
        x[1] = baseX + BRICK_SIZE;
        y[1] = baseY;
        x[2] = baseX + BRICK_SIZE;
        y[2] = baseY + BRICK_SIZE;
        x[3] = baseX + BRICK_SIZE + BRICK_SIZE;
        y[3] = baseY + BRICK_SIZE;
    }
    // 4 - l
    if (type == 4){
        x[0] = baseX;
        y[0] = baseY;
        x[1] = baseX;
        y[1] = baseY + BRICK_SIZE;
        x[2] = baseX;
        y[2] = baseY + BRICK_SIZE + BRICK_SIZE;
        x[3] = baseX + BRICK_SIZE;
        y[3] = baseY + BRICK_SIZE + BRICK_SIZE;
    }
    // 5 - j
    if (type == 5){
        x[0] = baseX;
        y[0] = baseY;
        x[1] = baseX;
        y[1] = baseY + BRICK_SIZE;
        x[2] = baseX;
        y[2] = baseY + BRICK_SIZE + BRICK_SIZE;
        x[3] = baseX - BRICK_SIZE;
        y[3] = baseY + BRICK_SIZE + BRICK_SIZE;
    }
    // 6 - t
    if (type == 6){
        x[0] = baseX;
        y[0] = baseY;
        x[1] = baseX;
        y[1] = baseY + BRICK_SIZE;
        x[2] = baseX + BRICK_SIZE;
        y[2] = baseY;
        x[3] = baseX - BRICK_SIZE;
        y[3] = baseY;
    }
}

// Alternate constructor for defining type and color explicitly
// instead of randomly
Tetrominoe::Tetrominoe(int bX, int bY, int t, int c){

    baseX = bX;
    baseY = bY;
    type = t;
    inPlay = false;
    rotState = 0;

    colorType = c;

    for (int i = 0; i < 4; i++){
        switch (colorType){
        case 0:     blocks[i] = redBrick;
                    break;
        case 1:     blocks[i] = blueBrick;
                    break;
        case 2:     blocks[i] = greyBrick;
                    break;
        case 3:     blocks[i] = greenBrick;
                    break;
        case 4:     blocks[i] = yellowBrick;
                    break;
        default: ;
        }

    }
    // 0 - square
    if (type == 0){
        // top left
        x[0] = baseX;
        y[0] = baseY;
        // top right
        x[1] = baseX + BRICK_SIZE;
        y[1] = baseY;
        // bottom left
        x[2] = baseX;
        y[2] = baseY + BRICK_SIZE;
        // bottom right
        x[3] = baseX + BRICK_SIZE;
        y[3] = baseY + BRICK_SIZE;
    }
    // 1 - bar
    if (type == 1){
        x[0] = baseX;
        y[0] = baseY;
        x[1] = baseX;
        y[1] = baseY + BRICK_SIZE;
        x[2] = baseX;
        y[2] = baseY + BRICK_SIZE + BRICK_SIZE;
        x[3] = baseX;
        y[3] = baseY + BRICK_SIZE + BRICK_SIZE + BRICK_SIZE;
    }
    // 2 - s
    if (type == 2){
        x[0] = baseX;
        y[0] = baseY;
        x[1] = baseX - BRICK_SIZE;
        y[1] = baseY;
        x[2] = baseX - BRICK_SIZE;
        y[2] = baseY + BRICK_SIZE;
        x[3] = baseX - BRICK_SIZE - BRICK_SIZE;
        y[3] = baseY + BRICK_SIZE;
    }
    // 3 - z
    if (type == 3){
        x[0] = baseX;
        y[0] = baseY;
        x[1] = baseX + BRICK_SIZE;
        y[1] = baseY;
        x[2] = baseX + BRICK_SIZE;
        y[2] = baseY + BRICK_SIZE;
        x[3] = baseX + BRICK_SIZE + BRICK_SIZE;
        y[3] = baseY + BRICK_SIZE;
    }
    // 4 - l
    if (type == 4){
        x[0] = baseX;
        y[0] = baseY;
        x[1] = baseX;
        y[1] = baseY + BRICK_SIZE;
        x[2] = baseX;
        y[2] = baseY + BRICK_SIZE + BRICK_SIZE;
        x[3] = baseX + BRICK_SIZE;
        y[3] = baseY + BRICK_SIZE + BRICK_SIZE;
    }
    // 5 - j
    if (type == 5){
        x[0] = baseX;
        y[0] = baseY;
        x[1] = baseX;
        y[1] = baseY + BRICK_SIZE;
        x[2] = baseX;
        y[2] = baseY + BRICK_SIZE + BRICK_SIZE;
        x[3] = baseX - BRICK_SIZE;
        y[3] = baseY + BRICK_SIZE + BRICK_SIZE;
    }
    // 6 - t
    if (type == 6){
        x[0] = baseX;
        y[0] = baseY;
        x[1] = baseX;
        y[1] = baseY + BRICK_SIZE;
        x[2] = baseX + BRICK_SIZE;
        y[2] = baseY;
        x[3] = baseX - BRICK_SIZE;
        y[3] = baseY;
    }
}

void Tetrominoe::show(){
    for (int i = 0; i < 4; i++){
        applySurface(x[i], y[i], blocks[i], screen);
    }
}

void Tetrominoe::preview(){

    int tempType = type;
    int tempColor = colorType;
    int tempX = PREVIEW_X;
    int tempY = PREVIEW_Y;
    switch (type){
        // square
        case 0: tempY += 15;
                tempX -= 8;
                break;
        // bar
        case 1: tempY += 0;
                tempX += 2;
                break;
        // s
        case 2: tempY += 15;
                tempX += 20;
                break;
        // z
        case 3: tempY += 15;
                tempX += -20;
                break;
        // l
        case 4: tempY += 8;
                tempX += -8;
                break;
        // j
        case 5: tempY += 8;
                tempX += 8;
                break;
        // t
        case 6: tempY += 20;
                tempX += 2;
                break;
        default: ;
    }

    Tetrominoe preview(tempX, tempY, type, colorType);
    preview.show();
}

int Tetrominoe::getBaseX(){
    return baseX;
}

int Tetrominoe::getBaseY(){
    return baseY;
}

void Tetrominoe::moveLeft(){
    // Determine if the move will be out of bounds
    baseX -= 20;

    x[0] -= 20;
    x[1] -= 20;
    x[2] -= 20;
    x[3] -= 20;
}

void Tetrominoe::moveRight(){

    baseX += 20;
    x[0] += 20;
    x[1] += 20;
    x[2] += 20;
    x[3] += 20;
}

void Tetrominoe::rotate(){
    // Advance to next rotation state
    if (rotState==3){
        rotState = 0;
    }
    else{
        rotState++;
    }

    // 0 - square
    if (type == 0){
        // Do nothing, square doesnt actually rotate
    }
    // 1 - bar
    else if (type == 1){
        if (rotState==1){
            x[0] = x[0] - BRICK_SIZE;
            y[0] = y[0] + BRICK_SIZE;
            x[1] = x[0] + BRICK_SIZE;
            y[1] = y[0];
            x[2] = x[1] + BRICK_SIZE;
            y[2] = y[0];
            x[3] = x[2] + BRICK_SIZE;
            y[3] = y[0];
        }
        else if (rotState==2){
            x[0] = x[0] + 2 * BRICK_SIZE;
            y[0] = y[0] - BRICK_SIZE;
            x[1] = x[0];
            y[1] = y[0] + BRICK_SIZE;
            x[2] = x[1];
            y[2] = y[1] + BRICK_SIZE;
            x[3] = x[2];
            y[3] = y[2] + BRICK_SIZE;
        }
        else if (rotState==3){
            x[0] = x[0] + BRICK_SIZE;
            y[0] = y[0] + 2 * BRICK_SIZE ;
            x[1] = x[0] - BRICK_SIZE;
            y[1] = y[0];
            x[2] = x[1] - BRICK_SIZE;
            y[2] = y[0];
            x[3] = x[2] - BRICK_SIZE;
            y[3] = y[0];
        }
        else if (rotState==0){
            x[0] = x[0] - 2 * BRICK_SIZE;
            y[0] = y[0] - 2 * BRICK_SIZE;
            x[1] = x[0];
            y[1] = y[0] + BRICK_SIZE;
            x[2] = x[0];
            y[2] = y[1] + BRICK_SIZE;
            x[3] = x[0];
            y[3] = y[2] + BRICK_SIZE;
        }
    }
    // 2 - s
    else if (type == 2){
        if (rotState==0){
            x[0] = x[0] + BRICK_SIZE;
            y[0] = y[0] - 2 * BRICK_SIZE;
            x[1] = x[0] - BRICK_SIZE;
            y[1] = y[0];
            x[2] = x[1];
            y[2] = y[1] + BRICK_SIZE;
            x[3] = x[2] - BRICK_SIZE;
            y[3] = y[2];
        }
        else if (rotState==1){
            x[0] = x[0] - BRICK_SIZE;
            y[0] = y[0];
            x[1] = x[0];
            y[1] = y[0] + BRICK_SIZE;
            x[2] = x[1] + BRICK_SIZE;
            y[2] = y[1];
            x[3] = x[2];
            y[3] = y[2]+ BRICK_SIZE;
        }
        else if (rotState==2){
            x[0] = x[0] + BRICK_SIZE;
            y[0] = y[0] + BRICK_SIZE;
            x[1] = x[0] - BRICK_SIZE;
            y[1] = y[0];
            x[2] = x[1];
            y[2] = y[1] + BRICK_SIZE;
            x[3] = x[2] - BRICK_SIZE;
            y[3] = y[2];
        }
        else if (rotState==3){
            x[0] = x[0] - BRICK_SIZE;
            y[0] = y[0] + BRICK_SIZE;
            x[1] = x[0];
            y[1] = y[0] - BRICK_SIZE;
            x[2] = x[1] - BRICK_SIZE;
            y[2] = y[1];
            x[3] = x[2];
            y[3] = y[2] - BRICK_SIZE;
        }
    }
    // 3 - z
    else if (type == 3){
        if (rotState==0){
            x[0] = x[0];
            y[0] = y[0] - 2 * BRICK_SIZE;
            x[1] = x[0] + BRICK_SIZE;
            y[1] = y[0];
            x[2] = x[1];
            y[2] = y[1] + BRICK_SIZE;
            x[3] = x[2] + BRICK_SIZE;
            y[3] = y[2];
        }
        else if (rotState==1){
            x[0] = x[0] + 2 * BRICK_SIZE;
            y[0] = y[0];
            x[1] = x[0];
            y[1] = y[0] + BRICK_SIZE;
            x[2] = x[1] - BRICK_SIZE;
            y[2] = y[1];
            x[3] = x[2];
            y[3] = y[2] + BRICK_SIZE;
        }
        else if (rotState==2){
            x[0] = x[0];
            y[0] = y[0] + 2 * BRICK_SIZE;
            x[1] = x[0] - BRICK_SIZE;
            y[1] = y[0];
            x[2] = x[1];
            y[2] = y[1] - BRICK_SIZE;
            x[3] = x[2] - BRICK_SIZE;
            y[3] = y[2];
        }
        else if (rotState==3){
            x[0] = x[0] - 2 * BRICK_SIZE;
            y[0] = y[0];
            x[1] = x[0];
            y[1] = y[0] - BRICK_SIZE;
            x[2] = x[1] + BRICK_SIZE;
            y[2] = y[1];
            x[3] = x[2];
            y[3] = y[2] - BRICK_SIZE;
        }
    }
    // 4 - l
    else if (type == 4){
        if (rotState==0){
            x[0] = x[0] + BRICK_SIZE;
            y[0] = y[0] - BRICK_SIZE;
            x[1] = x[0];
            y[1] = y[0] + BRICK_SIZE;
            x[2] = x[0];
            y[2] = y[1] + BRICK_SIZE;
            x[3] = x[2] + BRICK_SIZE;
            y[3] = y[2];
        }
        else if (rotState==1){
            x[0] = x[0] + BRICK_SIZE;
            y[0] = y[0] + BRICK_SIZE;
            x[1] = x[0] - BRICK_SIZE;
            y[1] = y[0];
            x[2] = x[1] - BRICK_SIZE;
            y[2] = y[0];
            x[3] = x[2];
            y[3] = y[2] + BRICK_SIZE;
        }
        else if (rotState==2){
            x[0] = x[0] - BRICK_SIZE;
            y[0] = y[0] + BRICK_SIZE;
            x[1] = x[0];
            y[1] = y[0] - BRICK_SIZE;
            x[2] = x[0];
            y[2] = y[1] - BRICK_SIZE;
            x[3] = x[0]-BRICK_SIZE;
            y[3] = y[2];
        }
        else if (rotState==3){
            x[0] = x[0] - BRICK_SIZE;
            y[0] = y[0] - BRICK_SIZE;
            x[1] = x[0] + BRICK_SIZE;
            y[1] = y[0];
            x[2] = x[1] + BRICK_SIZE;
            y[2] = y[0];
            x[3] = x[2];
            y[3] = y[2] - BRICK_SIZE;
        }
    }
    // 5 - j
    else if (type == 5){
        if (rotState==1){
            x[0] = x[0] + BRICK_SIZE;
            y[0] = y[0] + BRICK_SIZE;
            x[1] = x[0] - BRICK_SIZE;
            y[1] = y[0];
            x[2] = x[1] - BRICK_SIZE;
            y[2] = y[0];
            x[3] = x[2];
            y[3] = y[2] - BRICK_SIZE;
        }
        else if (rotState==2){
            x[0] = x[0] - BRICK_SIZE;
            y[0] = y[0] + BRICK_SIZE;
            x[1] = x[0];
            y[1] = y[0] - BRICK_SIZE;
            x[2] = x[0];
            y[2] = y[1] - BRICK_SIZE;
            x[3] = x[0] + BRICK_SIZE;
            y[3] = y[2];
        }
        else if (rotState==3){
            x[0] = x[0] - BRICK_SIZE;
            y[0] = y[0] - BRICK_SIZE;
            x[1] = x[0] + BRICK_SIZE;
            y[1] = y[0];
            x[2] = x[1] + BRICK_SIZE;
            y[2] = y[0];
            x[3] = x[2];
            y[3] = y[0] + BRICK_SIZE;
        }
        else if (rotState==0){
            x[0] = x[0] + BRICK_SIZE;
            y[0] = y[0] - BRICK_SIZE;
            x[1] = x[0];
            y[1] = y[0] + BRICK_SIZE;
            x[2] = x[0];
            y[2] = y[1] + BRICK_SIZE;
            x[3] = x[0] - BRICK_SIZE;
            y[3] = y[2];
        }
    }
    // 6 - t
    else if (type == 6){
        if (rotState==0){
            x[1] = x[0];
            y[1] = y[0] + BRICK_SIZE;
            x[2] = x[0] - BRICK_SIZE;
            y[2] = y[0];
            x[3] = x[0] + BRICK_SIZE;
            y[3] = y[0];
        }
        else if (rotState==1){
            x[1] = x[0] - BRICK_SIZE;
            y[1] = y[0];
            x[2] = x[0];
            y[2] = y[0] - BRICK_SIZE;
            x[3] = x[0];
            y[3] = y[0] + BRICK_SIZE;
        }
        else if (rotState==2){
            x[1] = x[0];
            y[1] = y[0] - BRICK_SIZE;
            x[2] = x[0] - BRICK_SIZE;
            y[2] = y[0];
            x[3] = x[0] + BRICK_SIZE;
            y[3] = y[0];
        }
        else if (rotState==3){
            x[1] = x[0] + BRICK_SIZE;
            y[1] = y[0];
            x[2] = x[0];
            y[2] = y[0] - BRICK_SIZE;
            x[3] = x[0];
            y[3] = y[0] + BRICK_SIZE;
        }
    }
}

void Tetrominoe::move(){
    baseY += 20;
    y[0] += 20;
    y[1] += 20;
    y[2] += 20;
    y[3] += 20;
}

void Tetrominoe::setBaseX(int x){
    baseX = x;
}

void Tetrominoe::setBaseY(int y){
    baseY = y;
}

void Tetrominoe::setInPlay(bool b){
    inPlay = b;
}

bool Tetrominoe::isInPlay(){
    return inPlay;
}

int Tetrominoe::handleInput(){
    if (event.type == SDL_KEYDOWN){
        switch(event.key.keysym.sym){
            case SDLK_w:        return 0;   // rotate block
                                break;
            case SDLK_s:        return 1;   // drop block, set drop true
                                break;
            case SDLK_a:        return 2;   // move block left, set moveLeft true
                                break;
            case SDLK_d:        return 3;   // move block right, set moveLeft true
                                break;
            case SDLK_q:        return 4;   // quit
                                break;
            case SDLK_p:        return 8;
                                break;
            default: ;
        }
    }
    else if (event.type == SDL_KEYUP){
        switch (event.key.keysym.sym){
            case SDLK_s:        return 5;   // set drop false
                                break;
            case SDLK_a:        return 6;   // set moveLeft false
                                break;
            case SDLK_d:        return 7;   // set moveRight false
                                break;
            default:    ;
        }
    }
}

int Tetrominoe::getX(int n){
    return x[n];
}

int Tetrominoe::getY(int n){
    return y[n];
}

int Tetrominoe::getColorType(){
    return colorType;
}

Grid::Grid(int n, int m, int t){
    x = n;
    y = m;
    type = t;
    setType(t);
    full = false;
}

int Grid::getType(){
    return type;
}

int Grid::getX(){
    return x;
}

int Grid::getY(){
    return y;
}

void Grid::setX(int n){
    x = n;
}

void Grid::setY(int n){
    y = n;
}

bool Grid::isFull(){
    return full;
}

void Grid::setFull(bool b){
    full = b;
}

void Grid::show(){
    applySurface(x,y,block,screen);
}

void Grid::setType(int n){
    type = n;
    switch (type){
        case 0:     block = redBrick;
                    break;
        case 1:     block = blueBrick;
                    break;
        case 2:     block = greyBrick;
                    break;
        case 3:     block = greenBrick;
                    break;
        case 4:     block = yellowBrick;
                    break;
        default: ;
    }
}

GameGrid::GameGrid(){
    int baseX = PLAY_AREA_X + PLAY_AREA_BORDER;
    int baseY = PLAY_AREA_Y + PLAY_AREA_BORDER;

    for (int i = 0 ; i < 440; i+=20){
        for (int j = 0; j < 20; j++){
            theGrid[i+j] = new Grid(baseX + (j * BRICK_SIZE), baseY + i, 0);
        }
    }
}

int GameGrid::getType(int i){
    return theGrid[i]->getType();
}

int GameGrid::getX(int i){
    return theGrid[i]->getX();
}

int GameGrid::getY(int i){
    return theGrid[i]->getY();
}

void GameGrid::showAll(){
    for (int i = 0; i < 440; i++){
        if (theGrid[i]->isFull()){
            theGrid[i]->show();
        }
    }
}

bool GameGrid::checkCollisionVert(int x, int y){
    int i = x + (y * 20);
    if (i < 0 || i > 439){
        // avoid index out of bounds errors
    }
    else if (theGrid[i]->isFull()){
        return true;
    }
    return false;
}

void GameGrid::setFull(int n, bool b){
    theGrid[n]->setFull(b);
}

void GameGrid::setBlock(int i, int j, int t){
    theGrid[i+j]->setFull(true);
    theGrid[i+j]->setType(t);
}

void GameGrid::setNewBlock(int i, int x, int y, int type){
    theGrid[i] = new Grid(x, y, type);
}

bool GameGrid::getFull(int i){
    return theGrid[i]->isFull();
}

int main(int argc, char* args[]){
    bool play = true;
    bool paused = false;

    if (!init()){
        return 1;
    }
    if (!loadFiles()){
        return 1;
    }

    freopen( "CON", "w", stdout );
    freopen( "CON", "w", stderr );

    // Change to increase speed at which piece falls
    int speedDelay = FALL_SPEED;
    // Regulate speed of falling
    int counter = 0;

    // Regulate speed of horizontal movement
    int sideCounter = 0;
    int sideDelay = SIDE_SPEED; // change to modify speed player moves horizontally

    bool lose = false;      // True if player has lost the game
    bool drop = false;      // True is player is holding drop key
    bool moveLeft = false;  // True if player is currently moving left
    bool moveRight = false; // True if layer is currently moving right
    bool checkRows = false; // True when a tetrominoe is committed to the gameGrid.
                            // Necessary to prevent checking the rows constantly.
    bool rotate = false;    // True if piece should rotate

    int dropPointX = ((PLAY_AREA_X + PLAY_AREA_WIDTH)/2) - 10;
    int dropPointY = PLAY_AREA_Y + PLAY_AREA_BORDER;

    std::string currentScore;
    std::string currentRows;
    std::string currentLevel;
    int scoreInt = 0;
    int rowsInt = STARTING_ROWS;
    int levelInt = 1;
    std::stringstream out;

    Timer timer;
    timer.start();

    // 440 + 10 for extra buffer
    Tetrominoe *tetrominoes[440];
    for (int i = 0; i < 440; i++){
        tetrominoes[i] = new Tetrominoe(dropPointX, dropPointY);
    }
    int tetCount = 0;
    bool newTet = true;

    int value = 99;

    GameGrid gameGrid;

    while (play && !lose){

        timer.start();
        if (newTet == true){
            tetrominoes[tetCount]->setBaseX(dropPointX);
            tetrominoes[tetCount]->setBaseY(dropPointY);
            tetrominoes[tetCount]->setInPlay(true);
            newTet = false;
        }
        if (counter == 0){
            counter = speedDelay;
        }
        counter--;

        if (sideCounter == 0){
            sideCounter = sideDelay;
        }
        sideCounter--;

        if (SDL_PollEvent(&event)){
            value = tetrominoes[tetCount]->handleInput();
            if (value == 4)
                play = false;
            // Rotate
            else if (value == 0){
                rotate = true;
            }
            // Drop
            else if (value == 1){
                drop = true;
            }
            // Move left
            else if (value == 2){
                moveLeft = true;
            }
            // Move right
            else if (value == 3){
                moveRight = true;
            }
            // Stop drop
            else if (value == 5){
                drop = false;
            }
            // Stop moving left
            else if (value == 6){
                moveLeft = false;
            }
            // Stop moving right
            else if (value == 7){
                moveRight = false;
            }
            else if (value == 8){
                if (paused)
                    paused = false;
                else
                    paused = true;
            }

        }

        applySurface(0,0,background,screen);
        applySurface(PLAY_AREA_X, PLAY_AREA_Y, playArea, screen);
        applySurface(INFO_AREA_X, INFO_AREA_Y, infoArea, screen);
        tetrominoes[tetCount+1]->preview();

        // Draw all filled squares to screen
        gameGrid.showAll();

        if (!paused){

        if (moveLeft && sideCounter == 0){
            // Check if pieces are in the way, otherwise
            // set moveLeft to true
            int x[4];
            int y[4];
            bool dontMove = false;
            for (int i = 0; i < 4; i++){
                x[i] = (tetrominoes[tetCount]->getX(i) - PLAY_AREA_X - PLAY_AREA_BORDER) / 20;
                y[i] = (tetrominoes[tetCount]->getY(i) - PLAY_AREA_Y - PLAY_AREA_BORDER) / 20;

                if (x[i] == 0){
                    dontMove = true;
                }
                else if (gameGrid.checkCollisionVert(x[i]-1, y[i])){
                    dontMove = true;
                }
            }
            if (!dontMove){
                tetrominoes[tetCount]->moveLeft();
            }
        }
        if (moveRight && sideCounter == 0){
            // Check if pieces are in the way, otherwise
            // set moveRight to true
            int x[4];
            int y[4];
            bool dontMove = false;
            for (int i = 0; i < 4; i++){
                x[i] = (tetrominoes[tetCount]->getX(i) - PLAY_AREA_X - PLAY_AREA_BORDER) / 20;
                y[i] = (tetrominoes[tetCount]->getY(i) - PLAY_AREA_Y - PLAY_AREA_BORDER) / 20;

                if (x[i] != 0 && x[i] % 19 == 0){
                   dontMove = true;
                }
                else if ((x[i]+1) % 10 == 0){
                    //dontMove=true;
                }
                else if (gameGrid.checkCollisionVert(x[i]+1, y[i])){
                    dontMove = true;
                }
            }

            if (!dontMove){
                tetrominoes[tetCount]->moveRight();
            }
        }

        if (rotate){
            bool dontRotate = false;
            // rotate the tetrominoe and check if it violates anything
            tetrominoes[tetCount]->rotate();
            int tempX = 0;
            int tempY = 0;
            for (int i = 0; i < 4; i++){
                tempX = tetrominoes[tetCount]->getX(i);
                tempY = tetrominoes[tetCount]->getY(i);
                // If it exceeds either horizontal boundary
                if (tempX < PLAY_AREA_BORDER + PLAY_AREA_X || tempX > PLAY_AREA_WIDTH + PLAY_AREA_BORDER){
                    dontRotate = true;
                }
                // If it exceeds either vertical boundary
                else if (tempY < PLAY_AREA_BORDER + PLAY_AREA_Y || tempY > PLAY_AREA_BORDER + PLAY_AREA_HEIGHT){
                    dontRotate = true;
                }

                // If it collides with another piece
                else if (gameGrid.checkCollisionVert(tempX, tempY)){
                    dontRotate = true;
                }
            }

            if (!dontRotate){
                // If there are no problems, leave the rotated tetrominoe
            }
            else{
                // Otherwise, rotate 3 times to get back to original position;
                tetrominoes[tetCount]->rotate();
                tetrominoes[tetCount]->rotate();
                tetrominoes[tetCount]->rotate();
            }
            rotate = false;
        }

        if (counter == 0 || drop == true){
            bool actionDone = false;
            int x[4];
            int y[4];

            for (int i = 0; i < 4; i++){
                x[i] = (tetrominoes[tetCount]->getX(i) - PLAY_AREA_X - PLAY_AREA_BORDER) / 20;
                y[i] = (tetrominoes[tetCount]->getY(i) - PLAY_AREA_Y - PLAY_AREA_BORDER) / 20;
                if ( x[i] + ((y[i] + 1) * 20) > 439){
                    // do nothing. This should stop the game from crashing
                }
                else if (gameGrid.checkCollisionVert(x[i], y[i]+1)){
                    actionDone=true;

                    // add all parts of the tetrominoe to the game grid, queue up a new piece
                    newTet = true;
                    tetrominoes[tetCount]->setInPlay(false);
                    checkRows = true;
                    int x[4];
                    int y[4];

                    for (int i = 0; i < 4; i++){
                        x[i] = (tetrominoes[tetCount]->getX(i) - PLAY_AREA_X - PLAY_AREA_BORDER) / 20;
                        y[i] = (tetrominoes[tetCount]->getY(i) - PLAY_AREA_Y - PLAY_AREA_BORDER) / 20;

                        if (y[i] == 0){
                            lose = true;
                        }

                        gameGrid.setBlock(x[i], y[i]*20, tetrominoes[tetCount]->getColorType());
                    }

                    tetCount++;
                    i = 4; // break out of the loop
                }
            }

            // Check if any portion of tetrominoe will hit bottom of
            // the screen
            bool dontFall = false;
            for (int i = 0; i < 4; i++){
                if (tetrominoes[tetCount]->getY(i) + 20 > PLAY_AREA_HEIGHT - PLAY_AREA_BORDER)
                    dontFall = true;
            }

            if (actionDone){
                // do nothing
            }
            else if (dontFall == false){
                tetrominoes[tetCount]->move();
            }
            // The tetromino has hit the bottom of the screen
            // Add its bricks to the gameGrid and remove it from play
            else{
                newTet = true;
                tetrominoes[tetCount]->setInPlay(false);
                checkRows = true;
                int x[4];
                int y[4];

                for (int i = 0; i < 4; i++){
                    x[i] = (tetrominoes[tetCount]->getX(i) - PLAY_AREA_X - PLAY_AREA_BORDER) / 20;
                    y[i] = (tetrominoes[tetCount]->getY(i) - PLAY_AREA_Y - PLAY_AREA_BORDER) / 20;

                    gameGrid.setBlock(x[i], y[i]*20, tetrominoes[tetCount]->getColorType());
                }

                tetCount++;
            }
        }

        if (checkRows){
            bool testHit = true;
            int index = 0;
            checkRows = false;
            for (int i = 0; i < 440; i+= 20){
                testHit = true;
                for (int j = 0; j < 20; j++){
                    if (!gameGrid.getFull(i+j)){
                        testHit=false;
                        break;
                    }
                }
                if (testHit){
                    rowsInt--;
                    scoreInt += levelInt*99;
                    if (rowsInt == 0){
                        levelInt++;
                        speedDelay -= 5;
                        if (speedDelay <= 0){
                            speedDelay = 0;
                        }
                        rowsInt = STARTING_ROWS + (5 * levelInt);
                    }
                    for (int j = i; j < i + 20; j++){
                        gameGrid.setFull(j, false);
                        tetrominoes[j]->setInPlay(false);

                    }
                    // Move all the rows above current row down
                    if (testHit){
                        GameGrid tempGrid;
                        int tempX = 0;
                        int tempY = 0;
                        int tempType = 0;
                        for (int j = 20 ; j < i+20; j++){
                            // Set tempGrid[i] to be same as gameGrid[i-20]
                            tempX = gameGrid.getX(j-20);
                            tempY = gameGrid.getY(j-20);
                            tempType = gameGrid.getType(j-20);
                            tempGrid.setNewBlock(j, tempX, tempY+BRICK_SIZE, tempType);
                            if (gameGrid.getFull(j-20)){
                                tempGrid.setFull(j, true);
                            }
                        }
                        // Fill remaining elements of tempGrid with same elements of
                        // gameGrid, as these blocks should remain unchanged
                        for (int j = i + 20; j < 440; j++){
                            tempX = gameGrid.getX(j);
                            tempY = gameGrid.getY(j);
                            tempType = gameGrid.getType(j);
                            tempGrid.setNewBlock(j, tempX, tempY, tempType);
                            if (gameGrid.getFull(j)){
                                tempGrid.setFull(j, true);
                            }
                        }
                        gameGrid = tempGrid;
                    }
                }
            }
            // Output current grid positions to the console
            cout << '\n';
            for (int i = 0; i < 440; i+=20){
                cout << "Row " << i / 20 << "\t:";
                for (int j = 0; j < 20; j++){
                    if (gameGrid.getFull(i+j)){
                        cout << " 1";
                    }
                    else{
                        cout << " 0";
                    }
                }
                cout << endl;
            }
        }

        }



        tetrominoes[tetCount]->show();

        gameGrid.showAll();

        // Update score, row and level strings
        out << scoreInt;
        currentScore = out.str();
        out.clear();
        out.str("");
        out << rowsInt;
        currentRows = out.str();
        out.clear();
        out.str("");
        out << levelInt;
        currentLevel = out.str();
        out.clear();
        out.str("");

        // Display all text
        score = TTF_RenderText_Solid(font, "SCORE:", fontColor);
        applySurface(SCORE_X, SCORE_Y, score, screen);
        scoreValue = TTF_RenderText_Solid(font, currentScore.c_str(), fontColor);
        applySurface(SCORE_X, SCORE_Y + 20, scoreValue, screen);

        level = TTF_RenderText_Solid(font, "LEVEL:", fontColor);
        applySurface(LEVEL_X, LEVEL_Y, level, screen);
        levelValue = TTF_RenderText_Solid(font, currentLevel.c_str(), fontColor);
        applySurface(LEVEL_X, LEVEL_Y + 20, levelValue, screen);

        rows = TTF_RenderText_Solid(font, "ROWS:", fontColor);
        applySurface(ROWS_X, ROWS_Y, rows, screen);
        rowsValue = TTF_RenderText_Solid(font, currentRows.c_str(), fontColor);
        applySurface(ROWS_X, ROWS_Y + 20, rowsValue, screen);


        if (SDL_Flip(screen) == -1){
            return 1;
        }
        // Regulate FPS
        if (timer.getTicks() < 1000 / FPS){
            SDL_Delay((1000 / FPS) - timer.getTicks());
        }

    }
    cleanUp();
    return 0;
}

