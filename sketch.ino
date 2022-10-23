/* Pin configurations
 *  Encoder --> Arduino
 *  GND --> GND
 *  VCC --> 5V
 *  SW --> IO17
 *  DT --> IO27
 *  CLK --> IO26
 *  
 *  Joystick --> Arduino
 *  GND --> GND
 *  +5V --> 5V
 *  VRX --> IO25
 *  VRY --> IO33
 *  SW --> IO32
 *  
 *  Matrix --> Arduino (orient the matrix so that you are looking at the back of the panel with the writing on it)
 *  Top Left 5V --> 5V
 *  Top Left GND --> GND```````````
 *  Top Right DIN --> IO13
 *  
 *  Matrix --> Matrix
 *  Top Left DOUT --> Bottom Right DIN
 *  Top Left Power Connector (Red and black cable with yellow head) --> Bottom Right Power Connector
 */


//Import Libraries
#include <Adafruit_NeoPixel.h>
#include <Encoder.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Fonts/Picopixel.h>

//Inputs:
#define SW 32 //Joystick button
#define potX 25 //oystick X
#define potY 33 //Joystick Y

//#define twist A1 //Potentiometer

#define ASW 16
#define BSW 17
#define XSW 18
#define YSW 19

#define rotSW 17 //Rotary encoder button 14
//Initialize encoder on digital pins 2 (clock) and 3 (DT)
Encoder myEnc(26, 27);

#define PIN 13 //Neopixel matrix output
//Define Matrix
#define NUMROWS 16
#define NUMCOLS 32



Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(NUMROWS, NUMCOLS, PIN,
  NEO_MATRIX_TOP + NEO_MATRIX_LEFT +
  NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
  NEO_GRB + NEO_KHZ800);    
  
void setup() {
  Serial.begin(9600);
  matrix.begin();
  matrix.setBrightness(30);
  matrix.show();
  pinMode(SW, INPUT_PULLUP);
  pinMode(potX, INPUT_PULLUP);
  pinMode(potY, INPUT_PULLUP);
  //pinMode(twist, INPUT);
  delay(500);

  matrix.setRotation(2);
}

String game = "None";

int triX1 = 6;
int triY1 = 8;
int triX2 = 5;
int triY2 = 7;
int triX3 = 5;
int triY3 = 9;

class Pipe
{
  // Class Member Variables
  // These are initialized at startup
  public:
  int topX;
  float currX;
  int topY;
  int gap;

  float timeLastUpdated;

  int pipeWidth;
  int topHeight;
  int bottomHeight;
  
  uint32_t pipeColor;

  int bottomY;

  Pipe(int x, int y, int dist, int width, uint32_t color)
  {
  topX = x;
  currX = topX;
  topY = y;  
  gap = dist;   
  pipeWidth = width;
  pipeColor = color;
  }

  void drawPipe()
  {
    topHeight = 32-topY;
    bottomHeight = topY-gap;
        
    matrix.fillRect(topX, topY, pipeWidth, topHeight, pipeColor);
    matrix.fillRect(topX, 0, pipeWidth, bottomHeight, pipeColor);

    timeLastUpdated = millis()/1000.0;
  }

  bool checkCollision(int x, int y) 
  {
    if ((x-1 <= topX+pipeWidth-1 && (y+1 >= topY || y-1 <= bottomHeight-1)) || y == 1) {
      return true;
    } else {
      return false;
    }
  }
};

class Bird
{
  public:
  int dotX;
  int dotY;
  float currDotY;
  float upSpeed;
  float jumpSpeed;
  float baseFallSpeed;
  float currFallSpeed;
  int maxY;
  float birdTimeLastUpdated;

  Bird(int x, int y, float upVel, float jumpVel, float baseVel, int max) {
    dotX = x;
    dotY = y;
    currDotY = dotY;

    upSpeed = upVel;
    jumpSpeed = jumpVel;
    baseFallSpeed = baseVel;
    currFallSpeed = baseFallSpeed;
    maxY = max;
    
    birdTimeLastUpdated = millis()/1000.0;      
  }

  void updateBirdPos(int rotSWstate, int prevState) {
  //Ensure that you have to release button and tap again to flap
  if (rotSWstate == 0 && prevState == 1) {
    //Set upward velocity to positive value and reset current falling speed to base fall speed
    upSpeed = jumpSpeed;
    currFallSpeed = baseFallSpeed;
    maxY = round(currDotY + 4);
    birdTimeLastUpdated = (millis()-100.0)/1000.0;
  }

  //only change the y pos if bird is above the ground or if the upSpeed is positive (might not be needed once collision mechanism with ground is added, just lets you jump up from ground)
  if (dotY > 1 || upSpeed > 0) {
    currDotY += upSpeed * (millis()/1000.0 - birdTimeLastUpdated);

    if (upSpeed > 0 && currDotY >= maxY) {
      currDotY = maxY;
      upSpeed = -10;      
    }

    if (round(currDotY) == 1.0 && upSpeed == -14) {
      currDotY = 1.0;
    }

    dotY = round(currDotY);

    upSpeed -= currFallSpeed * (millis()/1000.0 - birdTimeLastUpdated);
    if (upSpeed <= -14) {
      upSpeed = -14;
    }

    // currFallSpeed += 5 * (millis()/1000.0 - birdTimeLastUpdated);   

    birdTimeLastUpdated = millis()/1000.0; 
  }

  if (dotY < 1) {
    dotY = 1;
    currDotY = 1.0;
  }

  if (dotY > 30) {
    dotY = 30;
   }
  }
  
  void drawBird() {
    matrix.drawPixel(dotX, dotY, matrix.Color(150, 150, 0));
    matrix.drawPixel(dotX+1, dotY, matrix.Color(150, 150, 0));
    matrix.drawPixel(dotX-1, dotY, matrix.Color(150, 150, 0));
    matrix.drawPixel(dotX, dotY+1, matrix.Color(150, 150, 0));
    matrix.drawPixel(dotX, dotY-1, matrix.Color(150, 150, 0));
  }
};

Bird gameBird(14, 16, 0, 20, 7, 4);

Pipe pipes[3] = {Pipe(-4, random(9,30), 7, 4, matrix.Color(0,150,0)),
              Pipe(-17, random(9,30), 7, 4, matrix.Color(0,150,0)),
              Pipe(-30, random(9,30), 7, 4, matrix.Color(0,150,0))
};
    
class FlappyBird
{
  public:
  
  int numPipes;
  float pipeSpeed;

  //previous state of the switch, used to prevent holding the button to make the bird go up
  int prevState;
  int rotSWstate;
  int xState;
  int yState;
  
  bool startGame;  
  bool isOver;

  FlappyBird(float pipeVel) {
    rotSWstate = digitalRead(rotSW);
    xState = map(analogRead(potX), 0, 4095, 0, 100);
    yState = map(analogRead(potY), 0, 4095, 0, 100);
    prevState = 1;

    triX1 = 6;
    triY1 = 8;
    triX2 = 5;
    triY2 = 7;
    triX3 = 5;
    triY3 = 9;

    pipeSpeed = pipeVel;
    numPipes = 3;

    startGame = false;
    isOver = false;
  }

  void initializeGame() {
    xState = map(analogRead(potX), 0, 4095, 0, 100);
    yState = map(analogRead(potY), 0, 4095, 0, 100);
    rotSWstate = digitalRead(rotSW);

    if (rotSWstate == 0) {
      startGame = true;
    }

    if (startGame && !isOver) {
      flappyBird();
    } else if (!startGame && !isOver) {
      gameBird.drawBird();
      gameBird.birdTimeLastUpdated = millis()/1000.0;

      for (int i = 0; i < numPipes; i++) {
        pipes[i].timeLastUpdated = millis()/1000.0;
      }
      matrix.show();
    } else {
      flapGameOver();
    }
  }

  void movePipes() {
    for (int i = 0; i < numPipes; i++) {
      pipes[i].currX += pipeSpeed * (millis()/1000.0 - pipes[i].timeLastUpdated);
      pipes[i].topX = round(pipes[i].currX);
      pipes[i].drawPipe();

      if (pipes[i].currX >= 16) {
        pipes[i].topY = random(9,30);
        pipes[i].topX = -22;
        pipes[i].currX = -22;
      }
    }
  }

  void resetGame() {
    delay(200);

    gameBird.dotX = 14;
    gameBird.dotY = 16;
    gameBird.currDotY = 16;
    gameBird.upSpeed = 0;
    gameBird.currFallSpeed = gameBird.baseFallSpeed;

    rotSWstate = digitalRead(rotSW);
    prevState = 1;

    matrix.setRotation(2);

    pipes[0].topX = -4;
    pipes[1].topX = -17;
    pipes[2].topX = -30;  

    pipes[0].currX = -4;
    pipes[1].currX = -17;
    pipes[2].currX = -30;  

    isOver = false;
    startGame = false;
  }

  void flapGameOver() {
    if (gameBird.dotY > 1) {
      for (int i = 0; i < numPipes; i++) {
        pipes[i].drawPipe();
      }

      gameBird.updateBirdPos(0, 0);
      gameBird.drawBird();

      matrix.show();
      matrix.fillScreen(0);

      delay(100);
    } else {
      matrix.setFont(&Picopixel);
      matrix.setRotation(0);

      matrix.setCursor(2, 4);
      matrix.setTextColor(matrix.Color(150,0,0));
      //void setTextWrap(boolean w);      
      matrix.print("End");

      matrix.setCursor(7,9);
      matrix.print("r");
      matrix.setCursor(5,14);
      matrix.print("m");    
      
      if (yState == 0 && triY1 == 8) {
        triX1 = 4;
        triY1 = 13;
        triX2 = 3;
        triY2 = 12;
        triX3 = 3;
        triY3 = 14;
      } else if (yState == 100 && triY1 == 13) {
        triX1 = 6;
        triY1 = 8;
        triX2 = 5;
        triY2 = 7;
        triX3 = 5;
        triY3 = 9;
      }

      if (rotSWstate == 0 && triY1 == 8) {
        resetGame();
      } else if (rotSWstate == 0 && triY1 == 13) {
        game = "None";
        delay(300);
      }
      
      matrix.fillTriangle(triX1,triY1,triX2,triY2,triX3,triY3,matrix.Color(0,0,150));
      
      matrix.show();
      matrix.fillScreen(0);
    }
    
  }
  
  void flappyBird() {
    gameBird.updateBirdPos(rotSWstate, prevState);
    gameBird.drawBird();

    movePipes();

    matrix.show();

    matrix.fillScreen(0);
    prevState = rotSWstate;

    for (int i = 0; i < numPipes; i++) {
      if (pipes[i].checkCollision(gameBird.dotX, gameBird.dotY)) {
        isOver=true;
        delay(100);
      }
    }
    
    delay(100);
  }
};

int rotSWstate;
int xState;
int yState;

FlappyBird flapGame(6.5);

int gamePointer = 0;

String gameList[2] = { "flappyBird", "tetris" };

int prevXState = 50;

class Tetris
{
  public:
  int grid[20][10] = { {0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0},
                     {0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0},
                     {0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0},
                     {0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0},
                     {0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0},
                     {0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0},
                     {0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0}};  

  int prevRow[10] = {0,0,0,0,0,0,0,0,0,0};
  int prevRow2[10] = {0,0,0,0,0,0,0,0,0,0};
  int prevRow3[10] = {0,0,0,0,0,0,0,0,0,0};
  int prevRow4[10] = {0,0,0,0,0,0,0,0,0,0};
  int prevRow5[10] = {0,0,0,0,0,0,0,0,0,0};

  int shapeOrder[7] = {0,1,2,3,4,5,6};
  String shapeList[7] = {"i","j","l","t","o","z","s"};

  float currLineX;
  int lineX;
  int prevLineX;

  float currLineY;
  int lineY;
  int prevLineY;
  float tetrisTimeLastUpdated;
  float controllerTimeLastUpdated;

  String currShape;
  String nextShape;
  int orientation;
  int prevOrientation;
  int width;
  int leftAdj;
  int height;

  float fallSpeed;
  int fastFallSpeed;
  float currFallSpeed;

  int xState;
  int prevXState;
  int yState;
  int prevYState;
  
  int encVal;
  int prevEncVal;

  int checked;

  bool landed;

  int rand1;
  int rand2;
  int temp;
  
  int tetrisRows[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  bool hasTetris;
  bool tetrisMode;
  int tetrisStart;
  int tetrisEnd;

  int shapePointer;

  int tetrisAnimPointer;

  bool isOver;

  int spawnX;

  Tetris() {
    lineX = random(0,9);
    spawnX = lineX;
    // lineX = 8;
    currLineX = lineX;
    prevLineX = lineX;

    checked = 0;

    currLineY = 19;
    lineY = 19;
    prevLineY = lineY;
    tetrisTimeLastUpdated = millis()/1000.0;   
    controllerTimeLastUpdated = millis()/1000.0;

    shapePointer = 1;

    shuffleShapes();

    currShape = shapeList[shapeOrder[0]];
    // currShape = "i";
    // orientation = random(1,5);
    orientation = 1;
    if (currShape == "i") {
      if ((orientation == 2 || orientation == 4)) {
        currLineY -= 1;
        lineY -= 1;
        prevLineY = lineY;
      }

      lineX = 4;
      spawnX = lineX;
      currLineX = lineX;
      prevLineX = lineX;
      
      // if (orientation == 1 && lineX == 8) {
      //   lineX -= 2;
      //   currLineX = lineX;
      //   prevLineX = lineX;
      // }
    } else if (currShape == "z" && orientation == 1 && lineX == 8) {
      lineX -=1;
      currLineX = lineX;
      prevLineX = lineX;
    }

    prevOrientation = orientation;
    width = 1;
    leftAdj = 0;
    height = 4;

    fallSpeed = 1;
    fastFallSpeed = 15;

    currFallSpeed = fallSpeed;

    landed = false;
    
    xState = map(analogRead(potX), 0, 4095, 0, 100);
    yState = map(analogRead(potY), 0, 4095, 0, 100);
    prevXState = xState;
    prevYState = xState; 

    encVal = getEncVal();
    prevEncVal = encVal;

    hasTetris = false;
    tetrisMode = false;
    tetrisStart = -1;
    tetrisEnd = -1;
    tetrisAnimPointer = 0;

    isOver = false;
  }

  void resetTetris() {
    for (int i = 0; i < 20; i++) {
      for (int j = 0; j < 10; j++) {
        grid[i][j] = 0;
      }
    }

    fallSpeed = 1;
    fastFallSpeed = 15;

    currFallSpeed = fallSpeed;
    
    currLineY = 19;
    lineY = 19;
    prevLineY = lineY;

    spawnPiece();
    isOver = false;
    matrix.setRotation(2);

    tetrisTimeLastUpdated = millis()/1000.0;
  }

  void shuffleShapes() {
    for (int i = 0; i < 100; i++) {
      rand1 = random(0,7);
      rand2 = random(0,7);
      
      while (rand1 == rand2) {
        rand2 = random(0,7);
      }
      
      temp = shapeOrder[rand1];
      shapeOrder[rand1] = shapeOrder[rand2];
      shapeOrder[rand2] = temp;
    }   
  }

  void drawNextShape() {
    if (nextShape == "i") {
      matrix.drawLine(0, 29, 3, 29, matrix.Color(110,236,238));
    } else if (nextShape == "j") {
      matrix.drawLine(0, 29, 2, 29, matrix.Color(0,0,230));
      matrix.drawPixel(2, 30, matrix.Color(0,0,230));      
    } else if (nextShape == "z") {
      matrix.drawLine(0, 29, 1, 29, matrix.Color(230,0,0));
      matrix.drawLine(1, 30, 2, 30, matrix.Color(230,0,0));
    } else if (nextShape == "l") {
      matrix.drawLine(0, 29, 2, 29, matrix.Color(238,167,53));
      matrix.drawPixel(0, 30, matrix.Color(238,167,53));         
    } else if (nextShape == "o") {
      matrix.drawLine(0, 29, 1, 29, matrix.Color(240,240,80));
      matrix.drawLine(0, 30, 1, 30, matrix.Color(240,240,80));      
    } else if (nextShape == "s") {
      matrix.drawLine(0, 30, 1, 30, matrix.Color(0,230,0));
      matrix.drawLine(1, 29, 2, 29, matrix.Color(0,230,0));      
    } else if (nextShape == "t") {
      matrix.drawLine(0, 29, 2, 29, matrix.Color(140,40,230));
      matrix.drawPixel(1, 30, matrix.Color(140,40,230));       
    }
  }

  void drawGrid() {
    for (int i = 0; i < 20; i++) {
      for (int j = 0; j < 10; j++) {
        int pixel = grid[i][j];
        if (pixel == 1) {
          matrix.drawPixel(j+3, i+6, matrix.Color(110,236,238));
        } else if (pixel == 2) {
          matrix.drawPixel(j+3, i+6, matrix.Color(0,0,230));        
        } else if (pixel == 3) {
          matrix.drawPixel(j+3, i+6, matrix.Color(230,0,0));
        } else if (pixel == 4) {
          matrix.drawPixel(j+3, i+6, matrix.Color(238,167,53));
        } else if (pixel == 5) {
          matrix.drawPixel(j+3, i+6, matrix.Color(240,240,80));
        } else if (pixel == 6) {
          matrix.drawPixel(j+3, i+6, matrix.Color(0,230,0));
        } else if (pixel == 7) {
          matrix.drawPixel(j+3, i+6, matrix.Color(140,40,230));
        }
      }
    }
  };

  void spawnPiece() {
    lineX = random(0,9);
    spawnX = lineX;
    currLineX = lineX;
    prevLineX = 0;

    currLineY = 19;
    lineY = 19;
    prevLineY = lineY;

    orientation = random(1,5);
    // orientation = 2;
    prevOrientation = orientation;

    currShape = shapeList[shapeOrder[shapePointer]];
    // currShape = "i";
    if (currShape == "i") {
      if (orientation == 4) {
        orientation = 2;
      }
      if (orientation == 2) {
        currLineY -= 1;
        lineY -= 1;
        prevLineY = lineY;
        Serial.println("adjustY");
        Serial.println("lineY");
      }

      lineX = 4;
      spawnX = lineX;
      currLineX = lineX;
      prevLineX = lineX;

      // if (orientation == 1 && lineX == 8) {
      //   lineX -= 2;
      //   currLineX = lineX;
      //   prevLineX = lineX;
      // }
      // if (orientation == 4 && lineX == 0) {
      //   lineX += 1;
      //   currLineX = lineX;
      //   prevLineX = lineX;
      // }
    } else if (currShape == "z" && orientation == 1 && lineX == 8) {
      lineX -=1;
      currLineX = lineX;
      prevLineX = lineX;
    }

    shapePointer += 1;
    // width = 1;
    // leftAdj = 0;
    // height = 4;

    if (shapePointer == 7) {
      shuffleShapes();
      shapePointer = 0;      
    }

    nextShape = shapeList[shapeOrder[shapePointer]];

    landed = false;

    detectCollision(true);
  }

  void checkTetris() {
    for (int i = 0; i < 20; i++) {
      hasTetris = true;
      for (int j = 0; j < 10; j++) { 
        if (grid[i][j] == 0) {
          hasTetris = false;
        }
      }
      if (hasTetris) {
        tetrisRows[i] = 1;   
        tetrisMode = true;

        if (tetrisStart == -1) {
          tetrisStart = i;
        }
      } else if (!hasTetris && tetrisStart != -1) {
        tetrisEnd = i;
        break;
      }
    }    
  }

  void tetrisAnimation() {
    for (int i = 0; i < 20; i++) {
      if (tetrisRows[i] == 1) {
        grid[i][tetrisAnimPointer] = 0;
      }
    }
    
    tetrisAnimPointer += 1;
    
    if (tetrisAnimPointer == 10) {
      tetrisMode = false;
      tetrisAnimPointer = 0;

      for (int i = 0; i < 20; i++) {  
        tetrisRows[i] = 0;
      }  
      
      for (int i = 0; i < 20-tetrisEnd; i++) {
        for (int j = 0; j < 10; j++) {
          grid[tetrisStart+i][j] = grid[tetrisEnd+i][j];          
        }      
      }

      tetrisStart = -1;
      tetrisEnd = -1;
    }
  }

  void gameOver() {
    rotSWstate = digitalRead(rotSW);  

    matrix.setFont(&Picopixel);
    matrix.setRotation(0);

    matrix.setCursor(2, 4);
    matrix.setTextColor(matrix.Color(150,0,0));
    //void setTextWrap(boolean w);      
    matrix.print("Game Over");

    matrix.setCursor(7,9);
    matrix.print("r");
    matrix.setCursor(5,14);
    matrix.print("m");    
    
    if (yState == 0 && triY1 == 8) {
      triX1 = 4;
      triY1 = 13;
      triX2 = 3;
      triY2 = 12;
      triX3 = 3;
      triY3 = 14;
    } else if (yState == 100 && triY1 == 13) {
      triX1 = 6;
      triY1 = 8;
      triX2 = 5;
      triY2 = 7;
      triX3 = 5;
      triY3 = 9;
    }

    if (rotSWstate == 0 && triY1 == 8) {
      resetTetris();
    } else if (rotSWstate == 0 && triY1 == 13) {
      game = "None";
      delay(300);
    }
    
    matrix.fillTriangle(triX1,triY1,triX2,triY2,triX3,triY3,matrix.Color(0,0,150));
    
    matrix.show();
    matrix.fillScreen(0);    
  }

  void mainGame() {
    xState = map(analogRead(potX), 0, 4095, 0, 100);
    yState = map(analogRead(potY), 0, 4095, 0, 100);
    encVal = getEncVal();

    if (!isOver) {
      if (!landed) {
        currLineY -= currFallSpeed * (millis()/1000.0 - tetrisTimeLastUpdated);
        lineY = round(currLineY);
      }

        if (lineY < prevLineY-1) {
          lineY += 1;
        }
      
      if ((xState == 0 || encVal-prevEncVal > 0) && lineX+width < 9 && !landed) {
        if (prevXState != 0 || encVal != prevEncVal) {  
          lineX += 1;        
        } else {
          currLineX += 5 * (millis()/1000.0 - controllerTimeLastUpdated);    
          lineX = round(currLineX);  

          if (lineX < prevLineX) {
            lineX = prevLineX;
          }
        }     
      } else if ((xState == 100 || encVal-prevEncVal < 0) && lineX-leftAdj > 0 && !landed) {
        if (prevXState != 100 || encVal != prevEncVal) {  
          lineX -= 1;        
        } else {
          currLineX -= 5 * (millis()/1000.0 - controllerTimeLastUpdated);
          lineX = round(currLineX);    
          
          if (lineX > prevLineX) {
            lineX = prevLineX;
          }    
        }  
      }
      controllerTimeLastUpdated = millis()/1000.0;

      if (yState == 100 && prevYState != 100 && !landed && lineY < 19) {
        orientation += 1;

        if (currShape == "i" && lineY >= 13 && (orientation == 3 || orientation == 1)) {
          orientation -=1;
        }
        if (orientation-2 > prevOrientation) {
          orientation -= 1;
        }   
        
        if (orientation == 5) {
          orientation = 1;
        }
      } else if (yState == 0) {
        currFallSpeed = fastFallSpeed;
      } else {
        currFallSpeed = fallSpeed;
      }

      detectCollision(false);

      if (!landed) {
        savePrevLines();
      }
      
      if (currShape == "j") {
        drawJ();      
      } else if (currShape == "i") {
        drawI();
      } else if (currShape == "z") {
        drawZ();
      } else if (currShape == "l") {
        drawL();
      } else if (currShape == "o") {
        drawO();
      } else if (currShape == "s") {
        drawS();
      } else if (currShape == "t") {
        drawT();
      }

      if (lineX+width > 9) {
        lineX = 9-width;
      } else if (lineX-leftAdj < 0) {
        lineX = leftAdj;
      }

      if (lineX-prevLineX > 1) {
        lineX -= 1;
      } else if (lineX-prevLineX < -1) {
        lineX += 1;
      }

      // if (lineY == 15) {
      //   lineX = spawnX;
      // }

      // if (lineY-height < 0) {
      //   landed = true;
      // }

      // if (landed) {
      //   lineY = height;
      //   currLineY = height;
      // }

      clearTop(false,true);

      drawGrid();
      drawNextShape();
      tetrisTimeLastUpdated = millis()/1000.0;

      matrix.drawLine(2, 5, 2, 26, matrix.Color(100,100,100));
      matrix.drawLine(13, 5, 13, 26, matrix.Color(100,100,100));

      matrix.drawLine(2, 5, 13, 5, matrix.Color(100,100,100));
      matrix.drawLine(2, 26, 13, 26, matrix.Color(100,100,100));
      
      matrix.show();  

      if (!landed) {
        for (int i = 0; i < 10; i++) {
          grid[lineY][i] = prevRow[i];
        }

        for (int i = 0; i < 10; i++) {
          grid[lineY-1][i] = prevRow2[i];
        }

        for (int i = 0; i < 10; i++) {
          grid[lineY-2][i] = prevRow3[i];
        }

        for (int i = 0; i < 10; i++) {
          grid[lineY-3][i] = prevRow4[i];
        }

        for (int i = 0; i < 10; i++) {
          grid[lineY+1][i] = prevRow5[i];
        }
        
      } else {
        if (!tetrisMode) {
          checkTetris();
          spawnPiece();
        }
      }

      if (tetrisMode) {
        tetrisAnimation();
      }
      
      matrix.fillScreen(0);
      delay(65);
      
      fallSpeed += 0.0015;

      prevXState = xState;  
      prevYState = yState; 
      prevLineX = lineX;
      prevLineY = lineY;
      prevOrientation = orientation; 
      prevEncVal = encVal;     
    } else {
      gameOver();
    }
  };

  void savePrevLines() {
      for (int i = 0; i < 10; i++) {
        prevRow[i] = grid[lineY][i];
      }

      for (int i = 0; i < 10; i++) {
        prevRow2[i] = grid[lineY-1][i];
      }

      for (int i = 0; i < 10; i++) {
        prevRow3[i] = grid[lineY-2][i];
      }

      for (int i = 0; i < 10; i++) {
        prevRow4[i] = grid[lineY-3][i];
      }

      for (int i = 0; i < 10; i++) {
        prevRow5[i] = grid[lineY+1][i];
      }
  }

  void clearTop(bool has_dot, bool is_empty) {
    for (int i = 0; i < 10; i++) {
      if (grid[19][i] != 0) {
        has_dot = true;
        break;        
      }
    }

    if (has_dot) {
      for (int i = 0; i < 10; i++) {
        if (grid[18][i] != 0) {
          is_empty = false;
        }
      }

      if (is_empty) {
        for (int i = 0; i < 10; i++) {
          grid[19][i] = 0;
        }        
      }
    }
  }

  void detectCollision(bool isSpawn) {
    if (currShape == "j") {
      if (orientation == 1) {
        width = 2;
        leftAdj = 0;
        height = 2;          
         
        if (lineY != prevLineY) {
          if (grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY-1][lineX+2] != 0 || lineY-1 < 0) {
            lineY += 1;
            landed = true;          
          }
        } 
        
        if (grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY-1][lineX+2] != 0 || grid[lineY][lineX+2] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
            lineX = prevLineX;
            currLineX = prevLineX; 
          } else if (!landed) {
            checked=0;
            while (grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY-1][lineX+2] != 0 || grid[lineY][lineX+2] != 0 || lineX-leftAdj < 0 || lineX+width > 9) {
              if (checked == 0) {
                lineY += 1;           
              } else if (checked == 1) {
                lineY -= 1;
                lineX += 1;
              } else if (checked == 2) {
                lineX -= 2;       
              } else if (checked == 3) {
                lineX -= 1;
              } else if (checked == 4) {
                lineX += 4;
              } else if (checked == 5) {
                lineX -= 2;
                orientation = prevOrientation;
                break;
              }
              checked += 1;             
            }
          }
        }    
      } else if (orientation == 2) {
        width = 1;
        leftAdj = 0;
        height = 3;

        if (lineY != prevLineY) {
          if (grid[lineY][lineX] != 0 || grid[lineY-2][lineX+1] != 0 || lineY-2 < 0) {
            lineY += 1;
            landed = true;          
          }
       } 
      
      if (grid[lineY][lineX] != 0 || grid[lineY][lineX+1] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY-2][lineX+1] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
            lineX = prevLineX;
            currLineX = prevLineX; 
          }  else if (!landed) {
            checked=0;
            while (grid[lineY][lineX] == 0 || grid[lineY][lineX+1] == 0 || grid[lineY-1][lineX+1] == 0 || grid[lineY-2][lineX+1] == 0 || lineX-leftAdj < 0 || lineX+width > 9) {
              if (checked == 0) {
                lineY += 1;           
              } else if (checked == 1) {
                lineY -= 1;
                lineX += 1;
              } else if (checked == 2) {
                lineX -= 2;       
              } else if (checked == 3) {
                lineX -= 1;
              } else if (checked == 4) {
                lineX += 4;
              } else if (checked == 5) {
                lineX -= 2;
                orientation = prevOrientation;
                break;
              }
              checked += 1;              
            }
          }
        }        
      } else if (orientation == 3) {
        width = 2;
        leftAdj = 0;
        height = 2;   
        if (lineY != prevLineY) {
          if (grid[lineY-2][lineX] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY-1][lineX+2] != 0 || lineY-2 < 0) {
            lineY += 1;
            landed = true;          
          }
        } 
        
        if (grid[lineY-2][lineX] != 0 || grid[lineY-1][lineX+2] != 0 || grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
            lineX = prevLineX;
            currLineX = prevLineX; 
          } else if (!landed) {
            checked=0;
            while (grid[lineY-2][lineX] != 0 || grid[lineY-1][lineX+2] != 0 || grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0 || lineX-leftAdj < 0 || lineX+width > 9) {
              if (checked == 0) {
                lineY += 1;           
              } else if (checked == 1) {
                lineY -= 1;
                lineX += 1;
              } else if (checked == 2) {
                lineX -= 2;       
              } else if (checked == 3) {
                lineX -= 1;
              } else if (checked == 4) {
                lineX += 4;
              } else if (checked == 5) {
                lineX -= 2;
                orientation = prevOrientation;
                break;
              }
              checked += 1;             
            }
          }
        }    
      } else {      
      width = 1;
      leftAdj = 0;
      height = 3;

      if (lineY != prevLineY) {
        if (grid[lineY-2][lineX+1] != 0 || grid[lineY-2][lineX] != 0 || lineY-2 < 0) {
          lineY += 1;
          landed = true;          
        }
       } 
      
      if (grid[lineY-2][lineX+1] != 0 || grid[lineY-2][lineX] != 0 || grid[lineY-1][lineX] != 0 || grid[lineY][lineX] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
            lineX = prevLineX;
            currLineX = prevLineX; 
          }  else if (!landed) {
            checked=0;
            while (grid[lineY-2][lineX+1] != 0 || grid[lineY-2][lineX] != 0 || grid[lineY-1][lineX] != 0 || grid[lineY][lineX] != 0 || lineX-leftAdj < 0 || lineX+width > 9) {
              if (checked == 0) {
                lineY += 1;           
              } else if (checked == 1) {
                lineY -= 1;
                lineX += 1;
              } else if (checked == 2) {
                lineX -= 2;       
              } else if (checked == 3) {
                lineX -= 1;
              } else if (checked == 4) {
                lineX += 4;
              } else if (checked == 5) {
                lineX -= 2;
                orientation = prevOrientation;
                break;
              }
              checked += 1;              
            }
          }
        }          
      }
    } else if (currShape == "i") {
      if (orientation == 1) {
        width = 3;
        leftAdj = 0;
        height = 1;

        if (lineY != prevLineY && !isSpawn) {
          if (grid[lineY][lineX] != 0 || grid[lineY][lineX+1] != 0 || grid[lineY][lineX+2] != 0 || grid[lineY][lineX+3] != 0 || lineY < 0) {
            lineY += 1;
            landed = true;          
          }
        } 
        
        if (grid[lineY][lineX] != 0 || grid[lineY][lineX+1] != 0 || grid[lineY][lineX+2] != 0 || grid[lineY][lineX+3] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
            lineX = prevLineX;
            currLineX = prevLineX; 
          }  else if (!landed) {
            checked=0;
            while (grid[lineY][lineX] != 0 || grid[lineY][lineX+1] != 0 || grid[lineY][lineX+2] != 0 || grid[lineY][lineX+3] != 0 || lineX-leftAdj < 0 || lineX+width > 9) {
              if (checked == 0) {
                lineY += 1;           
              } else if (checked == 1) {
                lineY -= 1;
                lineX += 1;
              } else if (checked == 2) {
                lineX -= 2;       
              } else if (checked == 3) {
                lineX -= 1;
              } else if (checked == 4) {
                lineX += 4;
              } else if (checked == 5) {
                lineX -= 2;
                orientation = prevOrientation;
                break;
              }
              checked += 1;              
            }
          }
        }   
      } else if (orientation == 2) {
        width = 1;
        leftAdj = -1;
        height = 3;

        if (lineY != prevLineY) {
          if (grid[lineY-2][lineX+1] != 0 || lineY-2 < 0) {
            lineY += 1;
            landed = true;          
          }
        } 
        
        if (grid[lineY-2][lineX+1] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY][lineX+1] != 0 || grid[lineY+1][lineX+1] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
            lineX = prevLineX;
            currLineX = prevLineX; 
          }  else if (!landed) {
            checked=0;
            while (grid[lineY-2][lineX+1] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY][lineX+1] != 0 || grid[lineY+1][lineX+1] != 0 || lineX-leftAdj < 0 || lineX+width > 9) {
              if (checked == 0) {
                lineY += 1;           
              } else if (checked == 1) {
                lineY -= 1;
                lineX += 1;
              } else if (checked == 2) {
                lineX -= 2;       
              } else if (checked == 3) {
                lineX -= 1;
              } else if (checked == 4) {
                lineX += 4;
              } else if (checked == 5) {
                lineX -= 2;
                orientation = prevOrientation;
                break;
              }
              checked += 1;              
            }
          }
        }  
      } else if (orientation == 3) {
        width = 3;
        leftAdj = 0;
        height = 1;

        if (lineY != prevLineY && !isSpawn) {
          if (grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY-1][lineX+2] != 0 || grid[lineY-1][lineX+3] != 0 || lineY-1 < 0) {
            lineY += 1;
            landed = true;          
          }
        } 
        
        if (grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY-1][lineX+2] != 0 || grid[lineY-1][lineX+3] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
            lineX = prevLineX;
            currLineX = prevLineX; 
          }  else if (!landed) {
            checked=0;
            while (grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY-1][lineX+2] != 0 || grid[lineY-1][lineX+3] != 0 || lineX-leftAdj < 0 || lineX+width > 9) {
              if (checked == 0) {
                lineY += 1;           
              } else if (checked == 1) {
                lineY -= 1;
                lineX += 1;
              } else if (checked == 2) {
                lineX -= 2;       
              } else if (checked == 3) {
                lineX -= 1;
              } else if (checked == 4) {
                lineX += 4;
              } else if (checked == 5) {
                lineX -= 2;
                orientation = prevOrientation;
                break;
              }
              checked += 1;              
            }
          }
        }          
      } else {
        width = 2;
        leftAdj = -2;
        height = 4;

        if (lineY != prevLineY) {
          if (grid[lineY-2][lineX+2] != 0 || lineY-2 < 0) {
            lineY += 1;
            landed = true;          
          }
        } 
        
        if (grid[lineY-2][lineX+2] != 0 || grid[lineY-1][lineX+2] != 0 || grid[lineY][lineX+2] != 0 || grid[lineY+1][lineX+2] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
            lineX = prevLineX;
            currLineX = prevLineX; 
          }  else if (!landed) {
            checked=0;
            while (grid[lineY-2][lineX+2] != 0 || grid[lineY-1][lineX+2] != 0 || grid[lineY][lineX+2] != 0 || grid[lineY+1][lineX+2] != 0 || lineX-leftAdj < 0 || lineX+width > 9) {
              if (checked == 0) {
                lineY += 1;           
              } else if (checked == 1) {
                lineY -= 1;
                lineX += 1;
              } else if (checked == 2) {
                lineX -= 2;       
              } else if (checked == 3) {
                lineX -= 1;
              } else if (checked == 4) {
                lineX += 4;
              } else if (checked == 5) {
                lineX -= 2;
                orientation = prevOrientation;
                break;
              }
              checked += 1;              
            }
          }
        }           
      }
    } else if (currShape == "z") {
      if (orientation == 1) {
        width = 2;
        leftAdj = 0;
        height = 2;

        if (lineY != prevLineY) {
          if (grid[lineY][lineX+2] != 0 || grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0 || lineY-1 < 0) {
            lineY += 1;
            landed = true;          
          }
        } 
        
        if (grid[lineY][lineX+2] != 0 || grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY][lineX+1] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
            lineX = prevLineX;
            currLineX = prevLineX; 
          }  else if (!landed) {
            checked=0;
            while (grid[lineY][lineX+2] != 0 || grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY][lineX+1] != 0 || lineX-leftAdj < 0 || lineX+width > 9) {
              if (checked == 0) {
                lineY += 1;           
              } else if (checked == 1) {
                lineY -= 1;
                lineX += 1;
              } else if (checked == 2) {
                lineX -= 2;       
              } else if (checked == 3) {
                lineX -= 1;
              } else if (checked == 4) {
                lineX += 4;
              } else if (checked == 5) {
                lineX -= 2;
                orientation = prevOrientation;
                break;
              }
              checked += 1;              
            }
          }
        }           
      } else if (orientation == 2) {
        width = 1;
        leftAdj = 0;
        height = 3;

        if (lineY != prevLineY) {
          if (grid[lineY-1][lineX] != 0 || grid[lineY-2][lineX+1] != 0 || lineY-2 < 0) {
            lineY += 1;
            landed = true;          
          }
        } 
        
        if (grid[lineY-1][lineX] != 0 || grid[lineY-2][lineX+1] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY][lineX] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
            lineX = prevLineX;
            currLineX = prevLineX; 
          }  else if (!landed) {
            checked=0;
            while (grid[lineY-1][lineX] != 0 || grid[lineY-2][lineX+1] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY][lineX] != 0 || lineX-leftAdj < 0 || lineX+width > 9) {
              if (checked == 0) {
                lineY += 1;           
              } else if (checked == 1) {
                lineY -= 1;
                lineX += 1;
              } else if (checked == 2) {
                lineX -= 2;       
              } else if (checked == 3) {
                lineX -= 1;
              } else if (checked == 4) {
                lineX += 4;
              } else if (checked == 5) {
                lineX -= 2;
                orientation = prevOrientation;
                break;
              }
              checked += 1;              
            }
          }
        }   
      } else if (orientation == 3) {
        width = 2;
        leftAdj = 0;
        height = 2;

        if (lineY != prevLineY) {
          if (grid[lineY-1][lineX+2] != 0 || grid[lineY-2][lineX] != 0 || grid[lineY-2][lineX+1] != 0 || lineY-2 < 0) {
            lineY += 1;
            landed = true;          
          }
        } 
        
        if (grid[lineY-1][lineX+2] != 0 || grid[lineY-2][lineX] != 0 || grid[lineY-2][lineX+1] != 0 || grid[lineY-1][lineX+1] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
            lineX = prevLineX;
            currLineX = prevLineX; 
          }  else if (!landed) {
            checked=0;
            while (grid[lineY-1][lineX+2] != 0 || grid[lineY-2][lineX] != 0 || grid[lineY-2][lineX+1] != 0 || grid[lineY-1][lineX+1] != 0 || lineX-leftAdj < 0 || lineX+width > 9) {
              if (checked == 0) {
                lineY += 1;           
              } else if (checked == 1) {
                lineY -= 1;
                lineX += 1;
              } else if (checked == 2) {
                lineX -= 2;       
              } else if (checked == 3) {
                lineX -= 1;
              } else if (checked == 4) {
                lineX += 4;
              } else if (checked == 5) {
                lineX -= 2;
                orientation = prevOrientation;
                break;
              }
              checked += 1;              
            }
          }
        }           
      } else {
        width = 2;
        leftAdj = -1;
        height = 2;

        if (lineY != prevLineY) {
          if (grid[lineY-1][lineX+1] != 0 || grid[lineY-2][lineX+2] != 0 || lineY-2 < 0) {
            lineY += 1;
            landed = true;          
          }
        } 
        
        if (grid[lineY-1][lineX+1] != 0 || grid[lineY-2][lineX+2] != 0 || grid[lineY][lineX+1] != 0 || grid[lineY-1][lineX+2] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
            lineX = prevLineX;
            currLineX = prevLineX; 
          }  else if (!landed) {
            checked=0;
            while (grid[lineY-1][lineX+2] != 0 || grid[lineY-2][lineX] != 0 || grid[lineY-2][lineX+1] != 0 || grid[lineY-1][lineX+1] != 0 || lineX-leftAdj < 0 || lineX+width > 9) {
              if (checked == 0) {
                lineY += 1;           
              } else if (checked == 1) {
                lineY -= 1;
                lineX += 1;
              } else if (checked == 2) {
                lineX -= 2;       
              } else if (checked == 3) {
                lineX -= 1;
              } else if (checked == 4) {
                lineX += 4;
              } else if (checked == 5) {
                lineX -= 2;
                orientation = prevOrientation;
                break;
              }
              checked += 1;              
            }
          }
        }             
      }
    } else if (currShape == "l") {
      if (orientation == 1) {
        width = 2;
        leftAdj = 0;
        height = 2;

        if (lineY != prevLineY) {
          if (grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY-1][lineX+2] != 0 || lineY-1 < 0) {
            lineY += 1;
            landed = true;          
          }
        } 
        
        if (grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY-1][lineX+2] != 0 || grid[lineY][lineX] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
            lineX = prevLineX;
            currLineX = prevLineX; 
          }  else if (!landed) {
            checked=0;
            while (grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY-1][lineX+2] != 0 || grid[lineY][lineX] != 0 || lineX-leftAdj < 0 || lineX+width > 9) {
              if (checked == 0) {
                lineY += 1;           
              } else if (checked == 1) {
                lineY -= 1;
                lineX += 1;
              } else if (checked == 2) {
                lineX -= 2;       
              } else if (checked == 3) {
                lineX -= 1;
              } else if (checked == 4) {
                lineX += 4;
              } else if (checked == 5) {
                lineX -= 2;
                orientation = prevOrientation;
                break;
              }
              checked += 1;              
            }
          }
        }            
      } else if (orientation == 2) {
        width = 1;
        leftAdj = 0;
        height = 3;

        if (lineY != prevLineY) {
          if (grid[lineY-2][lineX] != 0 || grid[lineY-2][lineX+1] != 0 || lineY-2 < 0) {
            lineY += 1;
            landed = true;          
          }
        } 
        
        if (grid[lineY-2][lineX] != 0 || grid[lineY-2][lineX+1] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY][lineX+1] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
            lineX = prevLineX;
            currLineX = prevLineX; 
          }  else if (!landed) {
            checked=0;
            while (grid[lineY-2][lineX] != 0 || grid[lineY-2][lineX+1] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY][lineX+1] != 0 || lineX-leftAdj < 0 || lineX+width > 9) {
              if (checked == 0) {
                Serial.println(0);
                lineY += 1;           
              } else if (checked == 1) {
                Serial.println(1);
                lineY -= 1;
                lineX += 1;
              } else if (checked == 2) {
                Serial.println(2);
                lineX -= 2;       
              } else if (checked == 3) {
                Serial.println(3);
                lineX -= 1;
              } else if (checked == 4) {
                Serial.println(4);
                lineX += 4;
              } else if (checked == 5) {
                lineX -= 2;
                orientation = prevOrientation;
                break;
              }
              checked += 1;              
            }
          }
        }         
      } else if (orientation == 3) {
        width = 2;
        leftAdj = 0;
        height = 3;

        if (lineY != prevLineY && orientation == prevOrientation) {
          if (grid[lineY-2][lineX+2] != 0 || grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0 || lineY-2 < 0) {
            lineY += 1;
            landed = true;                
          }
        } 
        
        if (grid[lineY-2][lineX+2] != 0 || grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY-1][lineX+2] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
            lineX = prevLineX;
            currLineX = prevLineX; 
          }  else if (!landed) {
            checked=0;
            while (grid[lineY-2][lineX+2] != 0 || grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY-1][lineX+2] != 0 || lineX-leftAdj < 0 || lineX+width > 9) {
              if (checked == 0) {
                lineY += 1;           
              } else if (checked == 1) {
                lineY -= 1;
                lineX += 1;
              } else if (checked == 2) {
                lineX -= 2;       
              } else if (checked == 3) {
                lineX -= 1;
              } else if (checked == 4) {
                lineX += 4;
              } else if (checked == 5) {
                lineX -= 2;
                orientation = prevOrientation;
                break;
              }
              checked += 1;              
            }
          }
        }          
      } else {
        width = 1;
        leftAdj = 0;
        height = 3;

        if (lineY != prevLineY) {
          if (grid[lineY][lineX+1] != 0 || grid[lineY-2][lineX] != 0 || lineY-2 < 0) {
            lineY += 1;
            landed = true;          
          }
        } 
        
        if (grid[lineY][lineX+1] != 0 || grid[lineY-2][lineX] != 0 || grid[lineY-1][lineX] != 0 || grid[lineY][lineX] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
            lineX = prevLineX;
            currLineX = prevLineX; 
          }  else if (!landed) {
            checked=0;
            while (grid[lineY][lineX+1] != 0 || grid[lineY-2][lineX] != 0 || grid[lineY-1][lineX] != 0 || grid[lineY][lineX] != 0 || lineX-leftAdj < 0 || lineX+width > 9) {
              if (checked == 0) {
                lineY += 1;           
              } else if (checked == 1) {
                lineY -= 1;
                lineX += 1;
              } else if (checked == 2) {
                lineX -= 2;       
              } else if (checked == 3) {
                lineX -= 1;
              } else if (checked == 4) {
                lineX += 4;
              } else if (checked == 5) {
                lineX -= 2;
                orientation = prevOrientation;
                break;
              }
              checked += 1;              
            }
          }
        }           
      }
    } else if (currShape == "o") {
      width = 1;
      leftAdj = 0;
      height = 2;

      if (lineY != prevLineY) {
        if (grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0 || lineY-1 < 0) {
          lineY += 1;
          landed = true;          
        }
      } 
      
      if (grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY][lineX] != 0 || grid[lineY][lineX+1] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
          lineX = prevLineX;
          currLineX = prevLineX; 
        }  else if (!landed) {
          checked=0;
          while (grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY][lineX] != 0 || grid[lineY][lineX+1] != 0 || lineX-leftAdj < 0 || lineX+width > 9) {
            if (checked == 0) {
              lineY += 1;           
            } else if (checked == 1) {
              lineY -= 1;
              lineX += 1;
            } else if (checked == 2) {
              lineX -= 2;       
            } else if (checked == 3) {
              lineX -= 1;
            } else if (checked == 4) {
              lineX += 4;
            } else if (checked == 5) {
              lineX -= 2;
              orientation = prevOrientation;
              break;
            }
            checked += 1;              
          }
        }
      }         
    } else if (currShape == "s") {
      if (orientation == 1) {
        width = 2;
        leftAdj = 0;
        height = 2;

        if (lineY != prevLineY) {
          if (grid[lineY-1][lineX+1] != 0 || grid[lineY-1][lineX+2] != 0 || grid[lineY][lineX] != 0 || lineY-1 < 0) {
            lineY += 1;
            landed = true;          
          }
        } 
        
        if (grid[lineY-1][lineX+1] != 0 || grid[lineY-1][lineX+2] != 0 || grid[lineY][lineX] != 0 || grid[lineY][lineX+1] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
            lineX = prevLineX;
            currLineX = prevLineX; 
          }  else if (!landed) {
            checked=0;
            while (grid[lineY-1][lineX+1] != 0 || grid[lineY-1][lineX+2] != 0 || grid[lineY][lineX] != 0 || grid[lineY][lineX+1] != 0 || lineX-leftAdj < 0 || lineX+width > 9) {
              if (checked == 0) {
                lineY += 1;           
              } else if (checked == 1) {
                lineY -= 1;
                lineX += 1;
              } else if (checked == 2) {
                lineX -= 2;       
              } else if (checked == 3) {
                lineX -= 1;
              } else if (checked == 4) {
                lineX += 4;
              } else if (checked == 5) {
                lineX -= 2;
                orientation = prevOrientation;
                break;
              }
              checked += 1;              
            }
          }
        }    
      } else if (orientation == 2) {
        width = 1;
        leftAdj = 0;
        height = 2;

        if (lineY != prevLineY) {
          if (grid[lineY-1][lineX+1] != 0 || grid[lineY-2][lineX] != 0 || lineY-2 < 0) {
            lineY += 1;
            landed = true;          
          }
        } 
        
        if (grid[lineY-1][lineX+1] != 0 || grid[lineY-2][lineX] != 0 || grid[lineY][lineX+1] != 0 || grid[lineY-1][lineX] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
            lineX = prevLineX;
            currLineX = prevLineX; 
          }  else if (!landed) {
            checked=0;
            while (grid[lineY-1][lineX+1] != 0 || grid[lineY-2][lineX] != 0 || grid[lineY][lineX+1] != 0 || grid[lineY-1][lineX] != 0 || lineX-leftAdj < 0 || lineX+width > 9) {
              if (checked == 0) {
                lineY += 1;           
              } else if (checked == 1) {
                lineY -= 1;
                lineX += 1;
              } else if (checked == 2) {
                lineX -= 2;       
              } else if (checked == 3) {
                lineX -= 1;
              } else if (checked == 4) {
                lineX += 4;
              } else if (checked == 5) {
                lineX -= 2;
                orientation = prevOrientation;
                break;
              }
              checked += 1;              
            }
          }
        }          
      } else if (orientation == 3) {
        width = 2;
        leftAdj = 0;
        height = 2;

        if (lineY != prevLineY) {
          if (grid[lineY-2][lineX+1] != 0 || grid[lineY-2][lineX+2] != 0 || grid[lineY-1][lineX] != 0 || lineY-2 < 0) {
            lineY += 1;
            landed = true;          
          }
        } 
        
        if (grid[lineY-2][lineX+1] != 0 || grid[lineY-2][lineX+2] != 0 || grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
            lineX = prevLineX;
            currLineX = prevLineX; 
          }  else if (!landed) {
            checked=0;
            while (grid[lineY-2][lineX+1] != 0 || grid[lineY-2][lineX+2] != 0 || grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0 || lineX-leftAdj < 0 || lineX+width > 9) {
              if (checked == 0) {
                lineY += 1;           
              } else if (checked == 1) {
                lineY -= 1;
                lineX += 1;
              } else if (checked == 2) {
                lineX -= 2;       
              } else if (checked == 3) {
                lineX -= 1;
              } else if (checked == 4) {
                lineX += 4;
              } else if (checked == 5) {
                lineX -= 2;
                orientation = prevOrientation;
                break;
              }
              checked += 1;              
            }
          }
        }          
      } else {
        width = 2;
        leftAdj = -1;
        height = 2;

        if (lineY != prevLineY) {
          if (grid[lineY-1][lineX+2] != 0 || grid[lineY-2][lineX+1] != 0 || lineY-2 < 0) {
            lineY += 1;
            landed = true;          
          }
        } 
        
        if (grid[lineY-1][lineX+2] != 0 || grid[lineY-2][lineX+1] != 0 || grid[lineY][lineX+2] != 0 || grid[lineY-1][lineX+1] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
            lineX = prevLineX;
            currLineX = prevLineX; 
          }  else if (!landed) {
            checked=0;
            while (grid[lineY-1][lineX+2] != 0 || grid[lineY-2][lineX+1] != 0 || grid[lineY][lineX+2] != 0 || grid[lineY-1][lineX+1] != 0 || lineX-leftAdj < 0 || lineX+width > 9) {
              if (checked == 0) {
                lineY += 1;           
              } else if (checked == 1) {
                lineY -= 1;
                lineX += 1;
              } else if (checked == 2) {
                lineX -= 2;       
              } else if (checked == 3) {
                lineX -= 1;
              } else if (checked == 4) {
                lineX += 4;
              } else if (checked == 5) {
                lineX -= 2;
                orientation = prevOrientation;
                break;
              }
              checked += 1;              
            }
          }
        }          
      }
    } else if (currShape == "t") {
      if (orientation == 1) {
        width = 2;
        leftAdj = 0;
        height = 1;

        if (lineY != prevLineY) {
          if (grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY-1][lineX+2] != 0 || lineY-1 < 0) {
            lineY += 1;
            landed = true;          
          }
        } 
        
        if (grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY-1][lineX+2] != 0 || grid[lineY][lineX+1] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
            lineX = prevLineX;
            currLineX = prevLineX; 
          }  else if (!landed) {
            checked=0;
              while (grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY-1][lineX+2] != 0 || grid[lineY][lineX+1] != 0 || lineX-leftAdj < 0 || lineX+width > 9) {
                if (checked == 0) {
                  lineY += 1;           
                } else if (checked == 1) {
                  lineY -= 1;
                  lineX += 1;
                } else if (checked == 2) {
                  lineX -= 2;       
                } else if (checked == 3) {
                  lineX -= 1;
                } else if (checked == 4) {
                  lineX += 4;
                } else if (checked == 5) {
                  lineX -= 2;
                  orientation = prevOrientation;
                  break;
                }
                checked += 1;              
              }
            }
          }            
      } else if (orientation == 2) {
        width = 1;
        leftAdj = 0;
        height = 3;

        if (lineY != prevLineY) {
          if (grid[lineY-1][lineX] != 0 || grid[lineY-2][lineX+1] != 0 || lineY-2 < 0) {
            lineY += 1;
            landed = true;          
          }
        } 
        
        if (grid[lineY-1][lineX] != 0 || grid[lineY-2][lineX+1] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY][lineX+1] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
            lineX = prevLineX;
            currLineX = prevLineX; 
          }  else if (!landed) {
            checked=0;
              while (grid[lineY-1][lineX] != 0 || grid[lineY-2][lineX+1] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY][lineX+1] != 0 || lineX-leftAdj < 0 || lineX+width > 9) {
                if (checked == 0) {
                  lineY += 1;           
                } else if (checked == 1) {
                  lineY -= 1;
                  lineX += 1;
                } else if (checked == 2) {
                  lineX -= 2;       
                } else if (checked == 3) {
                  lineX -= 1;
                } else if (checked == 4) {
                  lineX += 4;
                } else if (checked == 5) {
                  lineX -= 2;
                  orientation = prevOrientation;
                  break;
                }
                checked += 1;              
              }
            }
          }             
      } else if (orientation == 3) {
        width = 2;
        leftAdj = 0;
        height = 2;

        if (lineY != prevLineY) {
          if (grid[lineY-2][lineX+1] != 0 || grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+2] != 0 || lineY-2 < 0) {
            lineY += 1;
            landed = true;          
          }
        } 
        
        if (grid[lineY-2][lineX+1] != 0 || grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+2] != 0 || grid[lineY-1][lineX+1] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
            lineX = prevLineX;
            currLineX = prevLineX; 
            Serial.println("lineX not equal");
            Serial.println(lineX);
          }  else if (!landed) {
            checked=0;
              while (grid[lineY-2][lineX+1] != 0 || grid[lineY-1][lineX] != 0 || grid[lineY-1][lineX+2] != 0 || grid[lineY-1][lineX+1] != 0 || lineX-leftAdj < 0 || lineX+width > 9) {
                if (checked == 0) {
                  Serial.println(0);
                  lineY += 1;           
                } else if (checked == 1) {
                  Serial.println(1);
                  lineY -= 1;
                  lineX += 1;
                } else if (checked == 2) {
                  Serial.println(2);
                  lineX -= 2;       
                } else if (checked == 3) {
                  Serial.println(3);
                  lineX -= 1;
                } else if (checked == 4) {
                  Serial.println(4);
                  lineX += 4;
                } else if (checked == 5) {
                  Serial.println(5);
                  lineX -= 2;
                  orientation = prevOrientation;
                  break;
                }
                checked += 1;              
              }
            }
          }            
      } else {
        width = 2;
        leftAdj = -1;
        height = 3;

        if (lineY != prevLineY) {
          if (grid[lineY-1][lineX+2] != 0 || grid[lineY-2][lineX+1] != 0 || lineY-1 < 0) {
            lineY += 1;
            landed = true;          
          }
        } 
        
        if (grid[lineY-1][lineX+2] != 0 || grid[lineY-2][lineX+1] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY][lineX+1] != 0) {
          if (isSpawn) {
            isOver = true;
          } else if (lineX != prevLineX) {
            lineX = prevLineX;
            currLineX = prevLineX; 
          }  else if (!landed) {
            checked=0;
              while (grid[lineY-1][lineX+2] != 0 || grid[lineY-2][lineX+1] != 0 || grid[lineY-1][lineX+1] != 0 || grid[lineY][lineX+1] != 0 || lineX-leftAdj < 0 || lineX+width > 9) {
                if (checked == 0) {
                  lineY += 1;           
                } else if (checked == 1) {
                  lineY -= 1;
                  lineX += 1;
                } else if (checked == 2) {
                  lineX -= 2;       
                } else if (checked == 3) {
                  lineX -= 1;
                } else if (checked == 4) {
                  lineX += 4;
                } else if (checked == 5) {
                  lineX -= 2;
                  orientation = prevOrientation;
                  break;
                }
                checked += 1;              
              }
            }
          }            
      }
    }
  };

  void drawI() {
    if (orientation == 1) {
      width = 3;
      leftAdj = 0;
      height = 1;

      for (int i = lineX; i < lineX+4; i++) {
        grid[lineY][i] = 1;
      }
    } else if (orientation == 2) {
      width = 1;
      leftAdj = -1;
      height = 3;

      for (int i = lineY+1; i > lineY-3; i--) {
        grid[i][lineX+1] = 1;
      }      
    } else if (orientation == 3) {
      width = 3;
      leftAdj = 0;
      height = 1;
      
      for (int i = lineX; i < lineX+4; i++) {
        grid[lineY-1][i] = 1;
      }
    } else {
      width = 2;
      leftAdj = -2;
      height = 3;

      for (int i = lineY+1; i > lineY-3; i--) {
        grid[i][lineX+2] = 1;
      }  
    }
  };

  void drawJ() {
    if (orientation == 1) {
      width = 2;
      leftAdj = 0;
      height = 2;  
      
      grid[lineY][lineX+2] = 2;
      
      for (int i = lineX; i < lineX+3; i++) {
        grid[lineY-1][i] = 2;
      }      
    } else if (orientation == 2) {
      width = 1;
      leftAdj = 0;
      height = 3;

      grid[lineY][lineX] = 2;
      
      for (int i = lineY; i > lineY-3; i--) {
        grid[i][lineX+1] = 2;
      }      
    } else if (orientation == 3) {
      width = 2;
      leftAdj = 0;
      height = 2;    

      grid[lineY-2][lineX] = 2;
      
      for (int i = lineX; i < lineX+3; i++) {
        grid[lineY-1][i] = 2;
      }        
    } else {
      width = 1;
      leftAdj = 0;
      height = 3;

      grid[lineY-2][lineX+1] = 2;
      
      for (int i = lineY; i > lineY-3; i--) {
        grid[i][lineX] = 2;
      }  
    }
  };

  void drawZ() {
    if (orientation == 1) {
      width = 2;
      leftAdj = 0;
      height = 2;

      for (int i = lineX+1; i < lineX+3; i++) {
        grid[lineY][i] = 3;
      }
      
      for (int i = lineX; i < lineX+2; i++) {
        grid[lineY-1][i] = 3;
      }
    } else if (orientation == 2) {
      width = 1;
      leftAdj = 0;
      height = 3;

      for (int i = lineY; i > lineY-2; i--) {
        grid[i][lineX] = 3;
      }  

      for (int i = lineY-1; i > lineY-3; i--) {
        grid[i][lineX+1] = 3;
      }
    } else if (orientation == 3) {
      width = 2;
      leftAdj = 0;
      height = 2;

      for (int i = lineX+1; i < lineX+3; i++) {
        grid[lineY-1][i] = 3;
      }
      
      for (int i = lineX; i < lineX+2; i++) {
        grid[lineY-2][i] = 3;
      }
    } else {
      width = 2;
      leftAdj = -1;
      height = 3;

      for (int i = lineY; i > lineY-2; i--) {
        grid[i][lineX+1] = 3;
      }  

      for (int i = lineY-1; i > lineY-3; i--) {
        grid[i][lineX+2] = 3;
      }
    }
  };

  void drawL() {
    if (orientation == 1) {
      width = 2;
      leftAdj = 0;
      height = 2;

      grid[lineY][lineX] = 4;
      
      for (int i = lineX; i < lineX+3; i++) {
        grid[lineY-1][i] = 4;
      }
    } else if (orientation == 2) {
      width = 1;
      leftAdj = 0;
      height = 3;

      grid[lineY-2][lineX] = 4;
      
      for (int i = lineY; i > lineY-3; i--) {
        grid[i][lineX+1] = 4;
      }      
    } else if (orientation == 3) {
      width = 2;
      leftAdj = 0;
      height = 3;

      grid[lineY-2][lineX+2] = 4;
      
      for (int i = lineX; i < lineX+3; i++) {
        grid[lineY-1][i] = 4;
      } 
    } else {
      width = 1;
      leftAdj = 0;
      height = 3;

      grid[lineY][lineX+1] = 4;
      
      for (int i = lineY; i > lineY-3; i--) {
        grid[i][lineX] = 4;
      }   
    }

  };   

  void drawO() {
    width = 1;
    leftAdj = 0;
    height = 2;
    
    for (int i = lineX; i < lineX+2; i++) {
      grid[lineY][i] = 5;
    }
    
    for (int i = lineX; i < lineX+2; i++) {
      grid[lineY-1][i] = 5;
    }
  };  
  
  void drawS() {
    if (orientation == 1) {
      width = 2;
      leftAdj = 0;
      height = 2;

      for (int i = lineX; i < lineX+2; i++) {
        grid[lineY][i] = 6;
      }
      
      for (int i = lineX+1; i < lineX+3; i++) {
        grid[lineY-1][i] = 6;
      }
    } else if (orientation == 2) {
      width = 1;
      leftAdj = 0;
      height = 2;

      for (int i = lineY; i > lineY-2; i--) {
        grid[i][lineX+1] = 6;
      }  

      for (int i = lineY-1; i > lineY-3; i--) {
        grid[i][lineX] = 6;
      }      
    } else if (orientation == 3) {
      width = 2;
      leftAdj = 0;
      height = 2;

      for (int i = lineX; i < lineX+2; i++) {
        grid[lineY-1][i] = 6;
      }
      
      for (int i = lineX+1; i < lineX+3; i++) {
        grid[lineY-2][i] = 6;
      }
    } else {
      width = 2;
      leftAdj = -1;
      height = 2;

      for (int i = lineY; i > lineY-2; i--) {
        grid[i][lineX+2] = 6;
      }  

      for (int i = lineY-1; i > lineY-3; i--) {
        grid[i][lineX+1] = 6;
      }   
    }

  };      
  
  void drawT() {
    if (orientation == 1) {
      grid[lineY][lineX+1] = 7;
    
      for (int i = lineX; i < lineX+3; i++) {
        grid[lineY-1][i] = 7;
      }
      width = 2;
      leftAdj = 0;
      height = 1;
    } else if (orientation == 2) {
      grid[lineY-1][lineX] = 7;

      for (int i = lineY; i > lineY-3; i--) {
        grid[i][lineX+1] = 7;
      }   
      width = 1;
      leftAdj = 0;
      height = 3;
    } else if (orientation == 3) {
      grid[lineY-2][lineX+1] = 7;
    
      for (int i = lineX; i < lineX+3; i++) {
        grid[lineY-1][i] = 7;
      }      
      width = 2;
      leftAdj = 0;
      height = 2;
    } else {
      grid[lineY-1][lineX+2] = 7;

      for (int i = lineY; i > lineY-3; i--) {
        grid[i][lineX+1] = 7;
      }  
      width = 2;
      leftAdj = -1;
      height = 3;
    }
  };    

};

Tetris tetris;

bool stop = false;

String menu() {
  game = "None";

  rotSWstate = digitalRead(rotSW);  
  xState = map(analogRead(potX), 0, 4095, 0, 100);

  if (xState == 100 && prevXState != 100) {
    gamePointer += 1;    
  } else if (xState == 0 && prevXState != 0) {
    gamePointer -= 1;
  }

  if (gamePointer > 1) {
    gamePointer = 0;
  } else if (gamePointer < 0) {
    gamePointer = 1;
  }

  if (gameList[gamePointer] == "flappyBird") {
    Pipe pipe(6, 11, 7, 4, matrix.Color(0,150,0));
    pipe.drawPipe();

    Bird menuBird(7, 7, 0, 20, 7, 4);
    menuBird.drawBird();
  } else if (gameList[gamePointer] == "tetris") {

    matrix.drawLine(2, 5, 2, 26, matrix.Color(100,100,100));
    matrix.drawLine(13, 5, 13, 26, matrix.Color(100,100,100));

    matrix.drawLine(2, 5, 13, 5, matrix.Color(100,100,100));
    matrix.drawLine(2, 26, 13, 26, matrix.Color(100,100,100));
    // matrix.drawLine(3, 0, 3, 15, matrix.Color(100,100,100));
    // matrix.drawLine(12, 0, 12, 15, matrix.Color(100,100,100));
  }

  if (rotSWstate == 0) {
    game = gameList[gamePointer];
  }

  if (game == "flappyBird") {
    flapGame.resetGame();
  } else if (game == "tetris") {
    tetris.resetTetris();
  }

  matrix.show();

  prevXState = xState;

  return game;
}

int getEncVal() {
  int encVal;

  encVal = myEnc.read();

  encVal /= 4;

  return encVal;
}

class SpaceInvaders
{
  // Class Member Variables
  // These are initialized at startup
  public:

  int shipX;
  int shipY;   

  int xState;
  int prevXState; 

  int rotSWstate;
  int prevState;

  int blastX;
  int blastY;
  float currBlastY;
  float blastVel;
  float blastTimeLastUpdated;
  float gameStartTime;
  bool isBlast;
  int blastCount;

  float invaders[5][4][2] = {{{1,0},{1,0},{1,0},{1,0}},{{1,0},{1,0},{1,0},{1,0}},{{1,0},{1,0},{1,0},{1,0}},{{1,0},{1,0},{1,0},{1,0}},{{1,0},{1,0},{1,0},{1,0}}};
  float invaderVel;
  float invaderYVel; 

  int xPositions[5] = {0,0,0,0,0};
  float currXPositions[5] = {0.0,0.0,0.0,0.0,0.0};

  int yPositions[5] = {16,16,16,16,16};
  int prevYPositions[5] = {16,16,16,16,16};
  float currYPositions[5] = {16.0,16.0,16.0,16.0,16.0};

  int barriers[3] = {5,5,5};

  SpaceInvaders()
  {
    shipX = 7;
    shipY = 1;

    blastX = -1;
    blastY = shipY+2;
    currBlastY = blastY;
    blastVel = 32;
    blastTimeLastUpdated = millis()/1000.0;
    gameStartTime = millis()/1000.0;
    isBlast = false;

    xState = map(analogRead(potX), 0, 4095, 0, 100);
    rotSWstate = digitalRead(rotSW);   
    prevState = 1;

    blastCount = 0; 

    // invaders[4][2] = {{14,17},{11,17},{8,17},{5,17}}
    // invaders[4][2] = {{9,17},{6,17},{3,17},{0,17}}

    invaderVel = 1;
    invaderYVel = -invaderVel;
  }

  void drawShip() {
    matrix.drawLine(shipX,shipY,shipX+2,shipY,matrix.Color(0,150,0));    
    matrix.drawPixel(shipX+1,shipY+1,matrix.Color(0,150,0));
  }

  void drawInvaders() {
    for (int i = 0; i < 5; i++) {
      for (int j = 0; j < 4; j++) {
        if (invaders[i][j][0] == 1) {
          matrix.fillRect(j*3+xPositions[i],i+yPositions[i],2,2,matrix.Color(150,150,150));
        } else if (invaders[i][j][0] == 2) {
          if (millis()/1000.0 - invaders[i][j][1] < 500) {
            matrix.fillRect(j*3+xPositions[i],i+yPositions[i],2,2,matrix.Color(236,0,0));            
          } else if (millis()/1000.0 - invaders[i][j][1] < 1000) {
            matrix.drawRect(j*3+xPositions[i]-1,i+yPositions[i]-1,4,4,matrix.Color(236,0,0));                        
          } else if (millis()/1000.0 - invaders[i][j][1] < 1500) {
            matrix.drawRect(j*3+xPositions[i]-1,i+yPositions[i]-1,4,4,matrix.Color(236,105,44)); 
            invaders[i][j][0] = 0;                              
          }
        }
      }
    }
  }

  void drawBarriers() {
    for (int i = 0; i < 3; i++) {
      if (barriers[i] > 0) {
        if (i == 0) {
          matrix.fillRect(1, shipY+4, 3, 2, matrix.Color(0,150,0));
        } else if (i == 1) {
          matrix.fillRect(7, shipY+4, 2, 2, matrix.Color(0,150,0));          
        } else {
          matrix.fillRect(12, shipY+4, 3, 2, matrix.Color(0,150,0));          
        }
      }
    }
  }

  void moveInvaders() {
    for (int i = 0; i < 5; i++) {
      if ((xPositions[i] == 5 && invaderVel > 0) || (xPositions[i] == 0 && invaderVel < 0)) {
        currYPositions[i] += invaderYVel * (millis()/1000.0 - blastTimeLastUpdated);
        yPositions[i] = round(currYPositions[i]);

        if (prevYPositions[i] == yPositions[i]+1) {
          currYPositions[i] = yPositions[i];
          prevYPositions[i] = yPositions[i];
          invaderVel *= -1;
        }
      } else {
        currXPositions[0] += invaderVel * (millis()/1000.0 - blastTimeLastUpdated);
        xPositions[0] = round(currXPositions[0]);
        
        if (millis()/1000.0-gameStartTime >= 100) {
          currXPositions[1] += invaderVel * (millis()/1000.0 - blastTimeLastUpdated);
          xPositions[1] = round(currXPositions[1]);
        }    

        if (millis()/1000.0-gameStartTime >= 200) {
          currXPositions[2] += invaderVel * (millis()/1000.0 - blastTimeLastUpdated);
          xPositions[2] = round(currXPositions[2]);
        }

        if (millis()/1000.0-gameStartTime >= 300) {
          currXPositions[3] += invaderVel * (millis()/1000.0 - blastTimeLastUpdated);
          xPositions[3] = round(currXPositions[3]);
        }

        if (millis()/1000.0-gameStartTime >= 400) {
          currXPositions[4] += invaderVel * (millis()/1000.0 - blastTimeLastUpdated);
          xPositions[4] = round(currXPositions[4]);
        }        
      }
    }
  }

  void makeBlast() {
    blastX = shipX+1;
    blastY = shipY+2;  
    isBlast = true;
  }

  void drawBlast() {
    matrix.drawPixel(blastX,blastY,matrix.Color(230,250,230));
    matrix.drawPixel(blastX,blastY-1,matrix.Color(130,170,170));
    matrix.drawPixel(blastX,blastY-2,matrix.Color(50,140,140));

  }

  void blastCollision() {
    if (blastY > 31) {
      blastX = -1;
      blastY = shipY+2;
      currBlastY = blastY;
      isBlast = false;
    } else {
      for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 4; j++) {
          if (invaders[i][j][0] == 1) {
            if (blastX >= j*3+xPositions[i] && blastX <= j*3+xPositions[i]+1 && blastY >= i+yPositions[i] && blastY <= i+yPositions[i]+1) {
              blastX = -1;
              blastY = shipY+2;
              currBlastY = blastY;
              isBlast = false;

              invaders[i][j][0] = 2;       
              invaders[i][j][1] = millis()/1000.0;             
            }
          }
        }
      }
    }
  }
    
  void moveBlast() { 
    currBlastY += blastVel * (millis()/1000.0 - blastTimeLastUpdated);
    blastY = round(currBlastY);
    blastCollision();
  }

  void invaderBlast() {

  }

  void mainGame() {
    xState = map(analogRead(potX), 0, 4095, 0, 100);
    rotSWstate = digitalRead(rotSW);

    if (xState == 100 && prevXState != 100) {
      shipX -= 1;    
    } else if (xState == 0 && prevXState != 0) {
      shipX += 1;
    }

    if (rotSWstate == 0 && prevState == 1 && !isBlast) {
      makeBlast();
    }

    moveInvaders();
    drawInvaders();

    drawBarriers();

    drawShip();

    if (isBlast) {
      moveBlast();
    }

    if (isBlast) {
      drawBlast();
    }

    blastTimeLastUpdated = millis()/1000.0;

    matrix.show();
    matrix.fillScreen(0);    

    prevXState = xState;
    prevState = rotSWstate;
  }
}

spaceGame = SpaceInvaders();

void loop() {
  // if (game == "None") {
  //   game = menu();
  //   matrix.fillScreen(0);
  // } else if (game == "flappyBird") {
  //   flapGame.initializeGame();
  // } else if (game == "tetris") {
  //   tetris.mainGame();
  // }

  spaceGame.mainGame();

  // if (rotSWstate == 0) {
  //   stop = true;
  // }
}

