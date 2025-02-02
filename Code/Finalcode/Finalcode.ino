#include <Wire.h>
#include <PulseSensorPlayground.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

/////////////////////////////
//----------Pin Definitions-------
/////////////////////////////
const int PulseWire = 0;
const int LED = LED_BUILTIN;
const int buttonPin = 2;
#define TFT_DC            8
#define TFT_RST           9
#define TFT_CS           10

/////////////////////////////
//----------TFT Display-------
/////////////////////////////
Adafruit_ST7735 TFT = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

/////////////////////////////
//----------Game Variables-------
/////////////////////////////
int bpmReadings[30];
int currentReadingIndex = 0;
int bpmSum = 0;
int averageBPM = 0;
bool inStage1 = false;
bool inStage2 = false;
bool inStage3 = false;
bool inStage4 = false;
bool inStage5 = false;
bool finished = false;
bool timeTakenBool = true;
PulseSensorPlayground pulseSensor;

unsigned long lastMeasurementTime = 0;
const long measurementInterval = 1000;
int Threshold = 650;

int targetNumber;
int buttonPressCount;
unsigned long stageStartTime;
unsigned long buttonStartTime;
bool lastButtonState = HIGH;
unsigned long timeLimit = 3500;
int stg1Points = 0;
int stg2Points = 0;
int stg3Points = 0;
int stg4Points = 0;
int stg5Points = 0;
int points = 0;
int stage = 1;
int timeTaken = 0;
int totalTime = 0;
int currentButtonState;
int screenreset=0;

/////////////////////////////
//----------Screen Dimensions-------
/////////////////////////////
#define TFTW            128
#define TFTH            160
#define TFTW2            64
#define TFTH2            80

/////////////////////////////
//----------Stage 2 Constants-------
/////////////////////////////
#define SPEED             1
#define GRAVITY         9.8
#define JUMP_FORCE     2.15
#define SKIP_TICKS     20.0
#define MAX_FRAMESKIP     5
#define BIRDW             8
#define BIRDH             8
#define BIRDW2            4
#define BIRDH2            4
#define PIPEW            12
#define GAPHEIGHT        36
#define FLOORH           20
#define GRASSH            4

/////////////////////////////
//----------Color Definitions-------
/////////////////////////////
const unsigned int BCKGRDCOL = TFT.color565(138, 235, 244);
const unsigned int BIRDCOL = TFT.color565(255, 254, 174);
const unsigned int PIPECOL  = TFT.color565(99, 255, 78);
const unsigned int PIPEHIGHCOL  = TFT.color565(250, 255, 250);
const unsigned int PIPESEAMCOL  = TFT.color565(0, 0, 0);
const unsigned int FLOORCOL = TFT.color565(246, 240, 163);
const unsigned int GRASSCOL  = TFT.color565(141, 225, 87);
const unsigned int GRASSCOL2 = TFT.color565(156, 239, 88);

/////////////////////////////
//----------Bird Sprite Colors-------
/////////////////////////////
#define C0 BCKGRDCOL
#define C1 TFT.color565(195,165,75)
#define C2 BIRDCOL
#define C3 ST7735_WHITE
#define C4 ST7735_RED
#define C5 TFT.color565(251,216,114)
static unsigned int birdcol[] =
{ C0, C0, C1, C1, C1, C1, C1, C0,
  C0, C1, C2, C2, C2, C1, C3, C1,
  C0, C2, C2, C2, C2, C1, C3, C1,
  C1, C1, C1, C2, C2, C3, C1, C1,
  C1, C2, C2, C2, C2, C2, C4, C4,
  C1, C2, C2, C2, C1, C5, C4, C0,
  C0, C1, C2, C1, C5, C5, C5, C0,
  C0, C0, C1, C5, C5, C5, C0, C0
};

/////////////////////////////
//----------Structures-------
/////////////////////////////
struct BIRD {
  unsigned char x, y, old_y;
  unsigned int col;
  float vel_y;
} bird;

struct PIPE {
  char x, gap_y;
  unsigned int col;
} pipe;

short scoreStage2;
short tmpx, tmpy;

/////////////////////////////
//----------Utility Functions-------
/////////////////////////////
#define drawPixel(a, b, c) TFT.setAddrWindow(a, b, a, b); TFT.pushColor(c)

/////////////////////////////
//----------Stage 4 Variables-------
/////////////////////////////
const int stage4_potPin = A2;
int stage4_potValue = 0;
int stage4_dotPosX = 0;
const int stage4_dotPosY = 80;

int stage4_lastDotPosX = -1, stage4_lastDotPosY = -1;

int stage4_ball1PosX = 0, stage4_ball1PosY = 0;
int stage4_ball2PosX = 0, stage4_ball2PosY = 0;
int stage4_ballSpeed = 2;
int stage4_ball2Speed = 2;
const int stage4_ballDiameter = 10;

int stage4_lastBall1PosX = -1, stage4_lastBall1PosY = -1;
int stage4_lastBall2PosX = -1, stage4_lastBall2PosY = -1;

int stage4_score = 0;

int stage4_currentLevel = 1;

bool stage4_gameStarted = false;

bool stage4_ball1Reset = false;
bool stage4_ball2Reset = false;

/////////////////////////////
//----------Stage 5 Variables-------
/////////////////////////////
#define POT_PIN A2
#define RESTART_BUTTON_PIN 2

int arrowX = 64;
int arrowY = 145;
const int arrowWidth = 10;
const int arrowHeight = 15;
int potValueStage5 = 0;
const int arrowSpeedStage5 = 2;

uint16_t fixedBlockColors[] = {ST77XX_RED, ST77XX_BLUE, ST77XX_YELLOW, ST77XX_GREEN};
uint16_t randomizedBlockColorsStage5[4];
String colorNames[] = {"Red", "Blue", "Yellow", "Green"};
int correctColorIndex = 0;
int displayedColorIndex = 0;

bool CgameOver = false;
bool CgameWon = false;
bool CgameOverScreenDisplayed = false;
int Cscore = 0;
int levelStage5 = 1;

int colorIndices[] = {0, 1, 2, 3};

int previousCscore = -1;

bool lastButtonStateStage5 = HIGH;

unsigned long lastDebounceTimeStage5 = 0;
const unsigned long debounceDelayStage5 = 50;

bool waitingToStartStage5 = true;

/////////////////////////////
//----------Setup-------
/////////////////////////////
void setup() {
  Serial.begin(115200);
  Wire.begin(8);
  Wire.onRequest(sendData);
  pulseSensor.analogInput(PulseWire);
  pulseSensor.blinkOnPulse(LED);
  pulseSensor.setThreshold(Threshold);
  pinMode(RESTART_BUTTON_PIN, INPUT_PULLUP);
  pinMode(buttonPin, INPUT_PULLUP);
  if (pulseSensor.begin()) {
    TFT.initR(INITR_BLACKTAB);
    TFT.fillScreen(ST7735_BLACK);
    TFT.setTextColor(ST7735_WHITE);
    TFT.setTextSize(1);
    showWelcomeScreen();
  }
  scoreStage2 = 0;
  bird.x = 20;
  bird.y = bird.old_y = TFTH2 - BIRDH;
  bird.vel_y = -JUMP_FORCE;
  tmpx = tmpy = 0;
  randomSeed(analogRead(0));
  pipe.x = TFTW;
  pipe.gap_y = random(20, TFTH - 60);
  arrowX = 64;
  arrowY = 145;
}

/////////////////////////////
//----------Main Loop-------
/////////////////////////////
void loop() {
  if (digitalRead(buttonPin) == LOW && !inStage1 && !inStage2 && !inStage3 && !inStage4 && !inStage5 ) {
    delay(200);
    inStage1 = true;
    TFT.fillScreen(ST7735_BLACK);
    startStage1();
  }
  if (inStage1) {
    if (timeTakenBool) {
      stageStartTime = millis();
      timeTakenBool = false;
    }
    checkHeartbeat();
  }
  if (inStage2) {
    startStage2();
  }
  if (inStage3) {
    TFT.fillScreen(ST7735_BLACK);
    TFT.setTextSize(2);
    TFT.setCursor(0, 10);  // Top-left corner
    TFT.setTextColor(ST7735_WHITE);
    TFT.println("Stage 3");
    TFT.setCursor(0, 30);
    TFT.println("Press");
    TFT.setCursor(0, 50);
    TFT.println("The Button");
    TFT.setCursor((TFT.width() - 90) / 2, 90);  
    TFT.setTextColor(ST7735_BLUE);
    TFT.println("BE FAST!");
    delay(5000);  
    TFT.setTextSize(2);
    TFT.fillScreen(ST7735_BLACK);
    TFT.setCursor(0, 10);  
    TFT.setTextColor(ST7735_WHITE);
    TFT.println("You Must");
    TFT.setCursor(0, 30);
    TFT.println("Get");
    TFT.setCursor(0, 50);
    TFT.println("400 points");
    TFT.setCursor(0, 90);  
    TFT.setTextColor(ST7735_BLUE);
    TFT.println("To Pass");
    delay(3000);
    TFT.fillScreen(ST7735_BLACK);
    TFT.setTextColor(ST7735_WHITE);
    TFT.setCursor(0, 10);
    TFT.println("4 Levels");
    TFT.setCursor(0, 30);
    TFT.println("Be");
    TFT.setCursor(0, 50);
    TFT.println("Fast");
    delay(3000);
    TFT.fillScreen(ST7735_BLACK);
    if (timeTakenBool) {
      stageStartTime = millis();
      timeTakenBool = false;
    }
    startStage3();
  }
  if (inStage4) {
    stage4_playGame();
  }
  if (inStage5) {
    if(screenreset ==0){
      TFT.fillScreen(ST7735_BLACK);
      screenreset =1;
    }
    if (timeTakenBool) {
      stageStartTime = millis();
      timeTakenBool = false;
    }
    if (waitingToStartStage5) {
      bool currentButtonState = digitalRead(RESTART_BUTTON_PIN);
      if (lastButtonStateStage5 == HIGH && currentButtonState == LOW && (millis() - lastDebounceTimeStage5) > debounceDelayStage5) {
        lastDebounceTimeStage5 = millis();
        waitingToStartStage5 = false;
        TFT.fillRect(0, 20, 128, 20, ST77XX_BLACK);
        resetGameStage5();
        drawArrowStage5(arrowX, arrowY);
      }
      lastButtonStateStage5 = currentButtonState;
      return;
    }
    if (CgameWon) {
      if (!CgameOverScreenDisplayed) {
        displayWinScreenStage5();
        CgameOverScreenDisplayed = true;
      }
      return;
    }
    if (CgameOver) {
      if (!CgameOverScreenDisplayed) {
        displayCgameOverStage5();
        CgameOverScreenDisplayed = true;
      }
      bool currentButtonState = digitalRead(RESTART_BUTTON_PIN);
      if (lastButtonStateStage5 == HIGH && currentButtonState == LOW && (millis() - lastDebounceTimeStage5) > debounceDelayStage5) {
        lastDebounceTimeStage5 = millis();
        restartGameStage5();
      }
      lastButtonStateStage5 = currentButtonState;
      return;
    }
    potValueStage5 = analogRead(POT_PIN);
    int newArrowX = map(potValueStage5, 0, 1023, 7, 121);
    newArrowX = constrain(newArrowX, 7, 121);
    int newArrowY = arrowY - arrowSpeedStage5;
    if (newArrowY < 0) {
      eraseArrowStage5(arrowX, arrowY);
      arrowY = 145;
      resetGameStage5();
      drawArrowStage5(newArrowX, arrowY);
      arrowX = newArrowX;
      return;
    }
    if (newArrowX != arrowX || newArrowY != arrowY) {
      eraseArrowStage5(arrowX, arrowY);
      arrowX = newArrowX;
      arrowY = newArrowY;
      drawArrowStage5(arrowX, arrowY);
    }
    checkCollisionStage5();
    if (Cscore != previousCscore) {
      updateCscoreDisplayStage5();
      previousCscore = Cscore;
    }
    delay(50);
  }
}

/////////////////////////////
//----------Stage 1 Methods-------
/////////////////////////////
void showWelcomeScreen() {
  TFT.fillScreen(ST7735_BLACK);
  TFT.setTextSize(2);
  TFT.setCursor(TFT.width(), 30);
  TFT.println("Welcome to");
  TFT.println("the Game!");
  TFT.setTextSize(1);
  TFT.setCursor((TFT.width() - 90) / 2, 90);
  TFT.println("Press the button");
  TFT.setCursor((TFT.width() - 90) / 2, 100);
  TFT.println("to start.");
  delay(2000);
}

void startStage1() {
  TFT.fillScreen(ST7735_BLACK);
  TFT.setTextSize(2);
  TFT.setCursor(0, 10);
  TFT.println("Stage 1");
  TFT.println("Get");
  TFT.println("your BPM!");
  TFT.setCursor((TFT.width() - 90) / 2, 90);
  TFT.setTextColor(ST7735_RED);
  TFT.println("above 140");
  TFT.setTextColor(ST7735_WHITE);
  TFT.setTextSize(2);
  delay(5000);
  TFT.fillScreen(ST7735_BLACK);
  TFT.fillRect(0, 40, TFT.width(), 40, ST7735_BLACK);
  TFT.setCursor(20, 60);
  TFT.print("Avg BPM: ");
  TFT.setTextSize(1);
  TFT.drawLine(0, 10, TFT.width(), 10, ST7735_RED);
  TFT.drawLine(0, 150, TFT.width(), 150, ST7735_RED);
}

void checkHeartbeat() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastMeasurementTime >= measurementInterval) {
    lastMeasurementTime = currentMillis;
    if (pulseSensor.sawStartOfBeat()) {
      int myBPM = pulseSensor.getBeatsPerMinute();
      bpmSum -= bpmReadings[currentReadingIndex];
      bpmReadings[currentReadingIndex] = myBPM;
      bpmSum += myBPM;
      currentReadingIndex = (currentReadingIndex + 1) % 30;
      averageBPM = bpmSum / 30;
      TFT.setTextSize(2);
      TFT.setCursor(20, 90);
      TFT.fillRect(20, 90, 100, 30, ST7735_BLACK);
      TFT.setTextColor(ST7735_RED);
      TFT.print(averageBPM);
      TFT.setTextColor(ST7735_BLACK);
      TFT.setTextSize(1);
      if (averageBPM > 140) {
        TFT.fillScreen(ST7735_BLACK);
        TFT.setTextSize(2);
        TFT.setTextColor(ST7735_GREEN);
        TFT.setCursor((TFT.width() - 60) / 2, 50);
        TFT.println("PASS");
        TFT.setTextColor(ST7735_WHITE);
        delay(2000);
        TFT.fillScreen(ST7735_BLACK);
        inStage1 = false;
        inStage2 = true;
        timeTaken = ((millis() - stageStartTime) / 1000);
        totalTime += timeTaken;
        timeTakenBool = true;
        Serial.println(timeTaken);
        stg1Points += 100;
        points +=stg1Points;
      }
    }
  }
}

/////////////////////////////
//----------Stage 2 Methods-------
/////////////////////////////
void startStage2() {
  TFT.fillScreen(ST7735_BLACK);
  TFT.setTextColor(ST7735_WHITE);
  TFT.setTextSize(2);
  TFT.setCursor(0, 10);
  TFT.println("Stage 2");
  TFT.println("Floppy");
  TFT.println("Bird");
  TFT.setCursor((TFT.width() - 90) / 2, 90);
  TFT.setTextColor(ST7735_YELLOW);
  TFT.println("Above 10");
  TFT.setTextColor(ST7735_WHITE);
  delay(5000);
  if (timeTakenBool) {
    stageStartTime = millis();
    timeTakenBool = false;
  }
  game_start();
  game_loop();
  game_over();
}

void game_start() {
  TFT.fillScreen(ST7735_BLACK);
  TFT.fillRect(10, TFTH2 - 20, TFTW - 20, 1, ST7735_WHITE);
  TFT.fillRect(10, TFTH2 + 32, TFTW - 20, 1, ST7735_WHITE);
  TFT.setTextColor(ST7735_WHITE);
  TFT.setTextSize(3);
  TFT.setCursor( TFTW2 - (6 * 9), TFTH2 - 16);
  TFT.println("FLAPPY");
  TFT.setTextSize(3);
  TFT.setCursor( TFTW2 - (6 * 9), TFTH2 + 8);
  TFT.println("-BIRD-");
  TFT.setTextSize(1);
  TFT.setCursor( 10, TFTH2 - 28);
  TFT.println("ATMEGA328");
  TFT.setCursor( TFTW2 - (12 * 3) - 1, TFTH2 + 34);
  TFT.println("press button");
  while (1) {
    if (digitalRead(buttonPin) == LOW) break;
  }
  game_init();
}

void game_init() {
  TFT.fillScreen(BCKGRDCOL);
  scoreStage2 = 0;
  bird.x = 20;
  bird.y = bird.old_y = TFTH2 - BIRDH;
  bird.vel_y = -JUMP_FORCE;
  tmpx = tmpy = 0;
  randomSeed(analogRead(0));
  pipe.x = TFTW;
  pipe.gap_y = random(20, TFTH - 60);
}

void game_loop() {
  unsigned char GAMEH = TFTH - FLOORH;
  TFT.drawFastHLine(0, GAMEH, TFTW, ST7735_BLACK);
  TFT.fillRect(0, GAMEH + 1, TFTW2, GRASSH, GRASSCOL);
  TFT.fillRect(TFTW2, GAMEH + 1, TFTW2, GRASSH, GRASSCOL2);
  TFT.drawFastHLine(0, GAMEH + GRASSH, TFTW, ST7735_BLACK);
  TFT.fillRect(0, GAMEH + GRASSH + 1, TFTW, FLOORH - GRASSH, FLOORCOL);
  char grassx = TFTW;
  double delta, old_time, next_game_tick, current_time;
  next_game_tick = current_time = millis();
  int loops;
  bool passed_pipe = false;
  unsigned char px;
  while (1) {
    loops = 0;
    while ( millis() > next_game_tick && loops < MAX_FRAMESKIP) {
      if (digitalRead(buttonPin) == LOW) {
        if (bird.y > BIRDH2 * 0.5) bird.vel_y = -JUMP_FORCE;
        else bird.vel_y = 0;
      }
      old_time = current_time;
      current_time = millis();
      delta = (current_time - old_time) / 1000.0;
      bird.vel_y += GRAVITY * delta;
      bird.y += bird.vel_y;
      pipe.x -= SPEED;
      if (pipe.x < -PIPEW) {
        pipe.x = TFTW;
        pipe.gap_y = random(20, GAMEH - (10 + GAPHEIGHT));
      }
      next_game_tick += SKIP_TICKS;
      loops++;
    }
    if (pipe.x >= 0 && pipe.x < TFTW) {
      TFT.drawFastVLine(pipe.x + 3, 0, pipe.gap_y, PIPECOL);
      TFT.drawFastVLine(pipe.x + 3, pipe.gap_y + GAPHEIGHT + 1, GAMEH - (pipe.gap_y + GAPHEIGHT + 1), PIPECOL);
      TFT.drawFastVLine(pipe.x, 0, pipe.gap_y, PIPEHIGHCOL);
      TFT.drawFastVLine(pipe.x, pipe.gap_y + GAPHEIGHT + 1, GAMEH - (pipe.gap_y + GAPHEIGHT + 1), PIPEHIGHCOL);
      drawPixel(pipe.x, pipe.gap_y, PIPESEAMCOL);
      drawPixel(pipe.x, pipe.gap_y + GAPHEIGHT, PIPESEAMCOL);
      drawPixel(pipe.x, pipe.gap_y - 6, PIPESEAMCOL);
      drawPixel(pipe.x, pipe.gap_y + GAPHEIGHT + 6, PIPESEAMCOL);
      drawPixel(pipe.x + 3, pipe.gap_y - 6, PIPESEAMCOL);
      drawPixel(pipe.x + 3, pipe.gap_y + GAPHEIGHT + 6, PIPESEAMCOL);
    }
    if (pipe.x <= TFTW) TFT.drawFastVLine(pipe.x + PIPEW, 0, GAMEH, BCKGRDCOL);
    tmpx = BIRDW - 1;
    do {
      px = bird.x + tmpx + BIRDW;
      tmpy = BIRDH - 1;
      do {
        drawPixel(px, bird.old_y + tmpy, BCKGRDCOL);
      } while (tmpy--);
      tmpy = BIRDH - 1;
      do {
        drawPixel(px, bird.y + tmpy, birdcol[tmpx + (tmpy * BIRDW)]);
      } while (tmpy--);
    } while (tmpx--);
    bird.old_y = bird.y;
    grassx -= SPEED;
    if (grassx < 0) grassx = TFTW;
    TFT.drawFastVLine( grassx    % TFTW, GAMEH + 1, GRASSH - 1, GRASSCOL);
    TFT.drawFastVLine((grassx + 64) % TFTW, GAMEH + 1, GRASSH - 1, GRASSCOL2);
    if (bird.y > GAMEH - BIRDH) break;
    if (bird.x + BIRDW >= pipe.x - BIRDW2 && bird.x <= pipe.x + PIPEW - BIRDW) {
      if (bird.y < pipe.gap_y || bird.y + BIRDH > pipe.gap_y + GAPHEIGHT) break;
      else passed_pipe = true;
    }
    else if (bird.x > pipe.x + PIPEW - BIRDW && passed_pipe) {
      passed_pipe = false;
      TFT.setTextColor(BCKGRDCOL);
      TFT.setCursor( TFTW2, 4);
      TFT.print(scoreStage2);
      TFT.setTextColor(ST7735_WHITE);
      scoreStage2++;
    }
    TFT.setCursor( TFTW2, 4);
    TFT.print(scoreStage2);
  }
  delay(1200);
}

void game_over() {
  if (scoreStage2 >= 10) {
    TFT.setTextSize(2);
    TFT.fillScreen(ST7735_BLACK);
    TFT.setTextColor(ST7735_GREEN);
    TFT.setCursor((TFT.width() - 60) / 2, 50);
    TFT.println("PASS");
    stg2Points += scoreStage2 * 30;
    points += stg2Points;
    TFT.setTextColor(ST7735_WHITE);
    delay(2000);
    TFT.fillScreen(ST7735_BLACK);
    inStage2 = false;
    inStage3 = true;
    timeTaken = ((millis() - stageStartTime) / 1000);
    totalTime += timeTaken;
    timeTakenBool = true;
  } else {
    TFT.setTextSize(2);
    TFT.fillScreen(ST7735_BLACK);
    TFT.setTextColor(ST7735_RED);
    TFT.setCursor((TFT.width() - 60) / 2, 50);
    TFT.println("FAIL");
    TFT.setTextColor(ST7735_WHITE);
    delay(2000);
    TFT.fillScreen(ST7735_BLACK);
    startStage2();
  }
}

/////////////////////////////
//----------Stage 3 Methods-------
/////////////////////////////
void startStage3() {
  targetNumber = random(1, 11);
  buttonPressCount = 0;
  buttonStartTime = millis();
  TFT.fillScreen(ST7735_BLACK);
  TFT.setTextSize(2);
  TFT.setCursor(0, 5);
  TFT.println("Press the button");
  TFT.setCursor((TFT.width() - 90) / 2, 60);
  TFT.setTextColor(ST7735_YELLOW);
  TFT.println(targetNumber);
  TFT.setTextColor(ST7735_WHITE);
  TFT.drawLine(0, 90, TFT.width(), 90, ST7735_BLUE);
  TFT.setCursor((TFT.width() - 90) / 2, 110);
  TFT.println("Points: ");
  TFT.setTextColor(ST7735_MAGENTA);
  TFT.print(stg3Points);
  TFT.setTextColor(ST7735_WHITE);
  while (millis() - buttonStartTime < timeLimit) {
    currentButtonState = digitalRead(buttonPin);
    delay(30);
    if (currentButtonState == LOW && lastButtonState == HIGH) {
      buttonPressCount++;
      TFT.setCursor(0, 10);
      TFT.fillScreen(ST7735_BLACK);
      TFT.setTextSize(2);
      TFT.print("Button pressed: ");
      TFT.println(buttonPressCount);
    }
    lastButtonState = currentButtonState;
  }
  Serial.print("Time's up! You pressed the button ");
  Serial.print(buttonPressCount);
  Serial.println(" times.");
  if (buttonPressCount == targetNumber) {
    stg3Points += 20;
    Serial.println("Success!");
  } else {
    Serial.println("Failed! Game Over.");
    timeLimit = 3500;
    stg3Points = 0;
    stage = 1;
    inStage3 = false;
    inStage4 = false;
    delay(1000);
    startStage3();
    return;
  }
  if (stg3Points == 100) {
    stage = 2;
    timeLimit = timeLimit - 500;
    TFT.fillScreen(ST7735_BLACK);
    TFT.setCursor((TFT.width() - 90) / 2, 60);
    TFT.setTextColor(ST7735_GREEN);
    TFT.println("Success!");
    TFT.setTextColor(ST7735_WHITE);
    TFT.println("to Level 2!");
    delay(2000);
  } else if (stg3Points == 200) {
    stage = 3;
    timeLimit = timeLimit - 500;
    TFT.fillScreen(ST7735_BLACK);
    TFT.setCursor((TFT.width() - 90) / 2, 60);
    TFT.setTextColor(ST7735_GREEN);
    TFT.println("Success!");
    TFT.setTextColor(ST7735_WHITE);
    TFT.println("to Level 3!");
    delay(2000);
  } else if (stg3Points == 300) {
    stage = 4;
    timeLimit = timeLimit - 500;
    TFT.fillScreen(ST7735_BLACK);
    TFT.setCursor((TFT.width() - 90) / 2, 60);
    TFT.setTextColor(ST7735_GREEN);
    TFT.println("Success!");
    TFT.setTextColor(ST7735_WHITE);
    TFT.println("to Level 4!");
    delay(2000);
  } else if (stg3Points == 400) {
    Serial.println("Congratulations! You completed all levels!");
    points += stg3Points;
    timeLimit = 3500;
    TFT.fillScreen(ST7735_BLACK);
    TFT.setTextSize(2);
    TFT.setTextColor(ST7735_GREEN);
    TFT.setCursor((TFT.width() - 60) / 2, 50);
    TFT.println("PASS");
    TFT.setTextColor(ST7735_WHITE);
    delay(2000);
    TFT.fillScreen(ST7735_BLACK);
    inStage3 = false;
    inStage4 = true;
    timeTaken = ((millis() - stageStartTime) / 1000);
    totalTime += timeTaken;
    timeTakenBool = true;
    startStage4();
    return;
  }
  delay(1000);
  startStage3();
}

/////////////////////////////
//----------Stage 4 Methods-------
/////////////////////////////
void startStage4() {
  TFT.fillScreen(ST7735_BLACK);
  TFT.setTextSize(2);
  TFT.setTextColor(ST7735_WHITE);
  TFT.fillScreen(ST7735_BLACK);
  TFT.setTextColor(ST7735_WHITE);
  TFT.setTextSize(2);
  TFT.setCursor(0, 10);
  TFT.println("Stage 4");
  TFT.println("Doge");
  TFT.println("Game");
  TFT.setCursor((TFT.width() - 90) / 2, 90);
  TFT.setTextColor(ST7735_MAGENTA);
  TFT.println("Above 40");
  TFT.setTextColor(ST7735_WHITE);
  delay(5000);
  TFT.fillScreen(ST7735_BLACK);
  TFT.setTextSize(1);
  TFT.setTextColor(ST7735_WHITE);
  stage4_resetGame();
}

void stage4_startGame() {
  stage4_score = 0;
  stage4_ballSpeed = 2;
  stage4_ball2Speed = 2;
  stage4_ball1PosX = random(0, 128);
  stage4_ball1PosY = 0;
  stage4_ball2PosX = random(0, 128);
  stage4_ball2PosY = 0;
  stage4_ball1Reset = false;
  stage4_ball2Reset = false;
  stage4_currentLevel = 1;
  stage4_gameStarted = true;
  TFT.fillScreen(ST7735_WHITE);
}

void stage4_resetGame() {
  stage4_gameStarted = false;
  stage4_ball1Reset = false;
  stage4_ball2Reset = false;
  TFT.fillScreen(ST7735_WHITE);
  TFT.setTextColor(ST7735_BLACK);
  TFT.setTextSize(1);
  TFT.setCursor(15, 70);
  TFT.print("Press Button");
  TFT.setCursor(25, 100);
  TFT.print("to Start");
}

void stage4_playGame() {
  stage4_potValue = analogRead(stage4_potPin);
  stage4_dotPosX = map(stage4_potValue, 0, 1023, 0, 128);
  stage4_dotPosX = constrain(stage4_dotPosX, 0, 128);
  if (digitalRead(buttonPin) == LOW) {
    delay(200);
    if (!stage4_gameStarted) {
      if (timeTakenBool) {
        stageStartTime = millis();
        timeTakenBool = false;
      }
      stage4_startGame();
    } else {
      stage4_resetGame();
    }
  }
  if (stage4_gameStarted) {
    TFT.fillRect(5, 5, 60, 10, ST7735_WHITE);
    TFT.setCursor(5, 5);
    TFT.setTextColor(ST7735_BLACK);
    TFT.setTextSize(1);
    TFT.print("Score: ");
    TFT.print(stage4_score);
    if (stage4_lastDotPosX != -1 && stage4_lastDotPosY != -1) {
      TFT.fillCircle(stage4_lastDotPosX, stage4_lastDotPosY, 5, ST7735_WHITE);
    }
    TFT.fillCircle(stage4_dotPosX, stage4_dotPosY, 5, ST7735_RED);
    stage4_lastDotPosX = stage4_dotPosX;
    stage4_lastDotPosY = stage4_dotPosY;
    if (stage4_lastBall1PosX != -1 && stage4_lastBall1PosY != -1) {
      TFT.fillCircle(stage4_lastBall1PosX, stage4_lastBall1PosY, stage4_ballDiameter / 2, ST7735_WHITE);
    }
    stage4_ball1PosY += stage4_ballSpeed;
    if (stage4_ball1PosY >= 160) {
      stage4_ball1PosY = 0;
      stage4_ball1PosX = random(0, 128);
      if (stage4_currentLevel == 1) {
        stage4_score++;
      } else if (stage4_currentLevel == 2) {
        stage4_ball1Reset = true;
      }
      if (stage4_currentLevel == 1) {
        stage4_ballSpeed++;
      } else if (stage4_currentLevel == 2) {
        stage4_ballSpeed++;
        stage4_ball2Speed++;
      }
    }
    TFT.fillCircle(stage4_ball1PosX, stage4_ball1PosY, stage4_ballDiameter / 2, ST7735_BLUE);
    stage4_lastBall1PosX = stage4_ball1PosX;
    stage4_lastBall1PosY = stage4_ball1PosY;
    if (stage4_currentLevel == 2 && stage4_lastBall2PosX != -1 && stage4_lastBall2PosY != -1) {
      TFT.fillCircle(stage4_lastBall2PosX, stage4_lastBall2PosY, stage4_ballDiameter / 2, ST7735_WHITE);
    }
    if (stage4_currentLevel == 2) {
      stage4_ball2PosY += stage4_ball2Speed;
      if (stage4_ball2PosY >= 160) {
        stage4_ball2PosY = 0;
        stage4_ball2PosX = random(0, 128);
        stage4_ball2Reset = true;
      }
      TFT.fillCircle(stage4_ball2PosX, stage4_ball2PosY, stage4_ballDiameter / 2, ST7735_GREEN);
      stage4_lastBall2PosX = stage4_ball2PosX;
      stage4_lastBall2PosY = stage4_ball2PosY;
    }
    if (stage4_currentLevel == 2) {
      if (stage4_ball1Reset && stage4_ball2Reset) {
        stage4_score++;
        stage4_ball1Reset = false;
        stage4_ball2Reset = false;
      }
    }
    if (stage4_checkCollision(stage4_dotPosX, stage4_dotPosY, stage4_ball1PosX, stage4_ball1PosY)) {
      stage4_gameOver();
      return;
    }
    if (stage4_currentLevel == 2 && stage4_checkCollision(stage4_dotPosX, stage4_dotPosY, stage4_ball2PosX, stage4_ball2PosY)) {
      stage4_gameOver();
      return;
    }
    if (stage4_score == 20 && stage4_currentLevel == 1) {
      stage4_levelUp();
      return;
    }
    if (stage4_score == 40 && stage4_currentLevel == 2) {
      stage4_winGame();
      inStage4 = false;
      inStage5 = true;
      return;
    }
    delay(50);
  }
}

bool stage4_checkCollision(int dotX, int dotY, int ballX, int ballY) {
  return (abs(dotX - ballX) < 5 + stage4_ballDiameter / 2) && (abs(dotY - ballY) < 5 + stage4_ballDiameter / 2);
}

void stage4_gameOver() {
  TFT.fillScreen(ST7735_WHITE);
  TFT.setCursor(30, 70);
  TFT.setTextColor(ST7735_BLACK);
  TFT.setTextSize(2);
  TFT.print("Game Over!");
  delay(2000);
  stage4_resetGame();
}

void stage4_levelUp() {
  stage4_currentLevel++;
  stage4_ballSpeed = 2;
  stage4_ball2Speed = 2;
  stage4_ball1PosY = 0;
  stage4_ball2PosY = 0;
  stage4_ball1Reset = false;
  stage4_ball2Reset = false;
  TFT.fillScreen(ST7735_WHITE);
  TFT.setCursor(10, 60);
  TFT.setTextColor(ST7735_BLACK);
  TFT.setTextSize(2);
  TFT.print("Level Up!");
  delay(2000);
  TFT.fillScreen(ST7735_WHITE);
}

void stage4_winGame() {
  TFT.fillScreen(ST7735_BLACK);
  TFT.setTextColor(ST7735_WHITE);
  TFT.fillScreen(ST7735_BLACK);
  TFT.setTextColor(ST7735_WHITE);
  TFT.fillScreen(ST7735_BLACK);
  TFT.setTextSize(2);
  TFT.setTextColor(ST7735_GREEN);
  TFT.setCursor((TFT.width() - 60) / 2, 50);
  TFT.println("PASS");
  TFT.setTextColor(ST7735_WHITE);
  delay(2000);
  TFT.fillScreen(ST7735_BLACK);
  TFT.setTextSize(2);
  TFT.setCursor(0, 10);
  TFT.println("Stage 5");
  TFT.println("Color");
  TFT.println("Match");
  TFT.setCursor((TFT.width() - 90) / 2, 90);
  TFT.setTextColor(ST7735_YELLOW);
  TFT.println("Above 40");
  TFT.setTextColor(ST7735_WHITE);
  delay(1000);
  stg4Points = stage4_score * 5;
  points += stg4Points;
  timeTaken = ((millis() - stageStartTime) / 1000);
  totalTime += timeTaken;
  timeTakenBool = true;
  inStage4 = false;
  inStage5 = true;
  return;
}

/////////////////////////////
//----------Stage 5 Methods-------
/////////////////////////////
void resetGameStage5() {
  correctColorIndex = random(0, 4);
  do {
    displayedColorIndex = random(0, 4);
  } while (displayedColorIndex == correctColorIndex);
  if (levelStage5 == 1) {
    for (int i = 0; i < 4; i++) {
      randomizedBlockColorsStage5[i] = fixedBlockColors[i];
    }
  } else if (levelStage5 == 2) {
    shuffleColorsStage5();
    for (int i = 0; i < 4; i++) {
      randomizedBlockColorsStage5[i] = fixedBlockColors[colorIndices[i]];
    }
  }
  TFT.fillRect(0, 0, 128, 20, ST77XX_BLACK);
  TFT.setCursor(30, 5);
  TFT.setTextColor(randomizedBlockColorsStage5[displayedColorIndex]);
  TFT.setTextSize(1);
  TFT.print(colorNames[correctColorIndex]);
  drawBlocksStage5();
  if (!CgameOver && !CgameWon) {
    drawArrowStage5(arrowX, arrowY);
  }
}

void drawBlocksStage5() {
  int blockWidth = 32;
  for (int i = 0; i < 4; i++) {
    TFT.fillRect(i * blockWidth, 50, blockWidth, 15, randomizedBlockColorsStage5[i]);
  }
}

void drawArrowStage5(int x, int y) {
  if (x - arrowWidth / 2 < 0 || x + arrowWidth / 2 >= 128 || y + arrowHeight > 160) {
    return;
  }
  TFT.fillTriangle(
    x, y,
    x - arrowWidth / 2, y + arrowHeight / 2,
    x + arrowWidth / 2, y + arrowHeight / 2,
    ST77XX_WHITE
  );
  TFT.fillRect(
    x - arrowWidth / 4,
    y + arrowHeight / 2,
    arrowWidth / 2,
    arrowHeight / 2,
    ST77XX_WHITE
  );
}

void eraseArrowStage5(int x, int y) {
  TFT.fillTriangle(
    x, y,
    x - arrowWidth / 2, y + arrowHeight / 2,
    x + arrowWidth / 2, y + arrowHeight / 2,
    ST77XX_BLACK
  );
  TFT.fillRect(
    x - arrowWidth / 4,
    y + arrowHeight / 2,
    arrowWidth / 2,
    arrowHeight / 2,
    ST77XX_BLACK
  );
  if (y + arrowHeight > 50 && y < 65) {
    int blockWidth = 32;
    int blockIndex = x / blockWidth;
    if (blockIndex >= 0 && blockIndex < 4) {
      TFT.fillRect(blockIndex * blockWidth, 50, blockWidth, 15, randomizedBlockColorsStage5[blockIndex]);
    }
  }
}

void shuffleColorsStage5() {
  for (int i = 3; i > 0; i--) {
    int j = random(0, i + 1);
    int temp = colorIndices[i];
    colorIndices[i] = colorIndices[j];
    colorIndices[j] = temp;
  }
}

void checkCollisionStage5() {
  if (arrowY < 65 && arrowY > 50) {
    int blockWidth = 32;
    int blockIndex = arrowX / blockWidth;
    if (blockIndex >= 0 && blockIndex < 4) {
      int selectedColorIndex = getColorIndexByColorStage5(randomizedBlockColorsStage5[blockIndex]);
      if (selectedColorIndex == correctColorIndex) {
        eraseArrowStage5(arrowX, arrowY);
        Cscore++;
        arrowY = 145;
        resetGameStage5();
        drawArrowStage5(arrowX, arrowY);
        if (Cscore == 20 && levelStage5 == 1) {
          levelUpStage5();
        } else if (Cscore == 40 && levelStage5 == 2) {
          winGameStage5();
        }
      } else {
        eraseArrowStage5(arrowX, arrowY);
        CgameOver = true;
      }
    }
  }
}

int getColorIndexByColorStage5(uint16_t color) {
  for (int i = 0; i < 4; i++) {
    if (color == fixedBlockColors[i]) {
      return i;
    }
  }
  return -1;
}

void displayCgameOverStage5() {
  TFT.fillScreen(ST77XX_BLACK);
  TFT.setTextSize(1);
  TFT.setTextColor(ST77XX_RED);
  TFT.setCursor(30, 60);
  TFT.print("Game Over!");
  TFT.setCursor(30, 80);
  TFT.setTextColor(ST77XX_WHITE);
  TFT.print("Cscore: ");
  TFT.print(Cscore);
  TFT.setCursor(20, 100);
  TFT.print("Press Button to");
  TFT.setCursor(40, 110);
  TFT.print("Restart");
}

void displayWinScreenStage5() {
  TFT.fillScreen(ST77XX_BLACK);
  TFT.setTextSize(1);
  TFT.setTextColor(ST77XX_GREEN);
  TFT.setCursor(20, 60);
  TFT.print("Congratulations!");
  TFT.setCursor(20, 80);
  TFT.print("You Win!");
  TFT.setCursor(20, 100);
  TFT.print("Final Point ");
  TFT.print(points);
  TFT.setCursor(20, 120);
  TFT.print("Open the box ");
  stg5Points = Cscore * 5;
  points += stg5Points;
  timeTaken = ((millis() - stageStartTime) / 1000);
  totalTime += timeTaken;
  inStage5 = false;
  finished = true;
}

void displayLevelUpStage5() {
  TFT.fillScreen(ST77XX_BLACK);
  TFT.setTextSize(1);
  TFT.setTextColor(ST77XX_GREEN);
  TFT.setCursor(30, 70);
  TFT.print("Level 2 Unlocked!");
  TFT.setCursor(30, 90);
  TFT.print("Keep Going!");
  delay(2000);
  arrowX = 64;
  arrowY = 145;
  TFT.fillScreen(ST77XX_BLACK);
  TFT.setCursor(10, 5);
  TFT.setTextColor(ST77XX_WHITE);
  TFT.print("Color Match Game!");
  updateCscoreDisplayStage5();
  resetGameStage5();
  drawArrowStage5(arrowX, arrowY);
}

void levelUpStage5() {
  levelStage5 = 2;
  displayLevelUpStage5();
}

void winGameStage5() {
  eraseArrowStage5(arrowX, arrowY);
  CgameWon = true;
}

void restartGameStage5() {
  CgameOver = false;
  CgameWon = false;
  CgameOverScreenDisplayed = false;
  Cscore = 0;
  levelStage5 = 1;
  arrowX = 64;
  arrowY = 145;
  previousCscore = -1;
  TFT.fillScreen(ST77XX_BLACK);
  TFT.setTextSize(1);
  TFT.setTextColor(ST77XX_WHITE);
  TFT.setCursor(10, 5);
  TFT.print("Color Match Game!");
  updateCscoreDisplayStage5();
  resetGameStage5();
  drawArrowStage5(arrowX, arrowY);
}

void winGame() {
  eraseArrowStage5(arrowX, arrowY);
  CgameWon = true;
}

void updateCscoreDisplayStage5() {
  TFT.setTextSize(1);
  TFT.fillRect(70, 5, 28, 10, ST77XX_BLACK);
  TFT.setCursor(70, 5);
  TFT.setTextColor(ST77XX_WHITE);
  TFT.print("Score: ");
  TFT.print(Cscore);
}

/////////////////////////////
//----------I2C Communication-------
/////////////////////////////
void sendData() {
  if (inStage1 == false && inStage2 == true) {
    int finishedStage = 1;
    Wire.write((byte*)&finishedStage, sizeof(finishedStage));
    Wire.write((byte*)&stg1Points, sizeof(stg1Points));
    Wire.write((byte*)&points, sizeof(points));
    Wire.write((byte*)&timeTaken, sizeof(timeTaken));
  }
  else if (inStage2 == false && inStage3 == true) {
    int finishedStage = 2;
    Wire.write((byte*)&finishedStage, sizeof(finishedStage));
    Wire.write((byte*)&stg2Points, sizeof(stg2Points));
    Wire.write((byte*)&points, sizeof(points));
    Wire.write((byte*)&timeTaken, sizeof(timeTaken));
  }
  else if (inStage3 == false && inStage4 == true) {
    int finishedStage = 3;
    Wire.write((byte*)&finishedStage, sizeof(finishedStage));
    Wire.write((byte*)&stg3Points, sizeof(stg3Points));
    Wire.write((byte*)&points, sizeof(points));
    Wire.write((byte*)&timeTaken, sizeof(timeTaken));
  }
  else if (inStage4 == false && inStage5 == true) {
    int finishedStage = 4;
    Wire.write((byte*)&finishedStage, sizeof(finishedStage));
    Wire.write((byte*)&stg4Points, sizeof(stg4Points));
    Wire.write((byte*)&points, sizeof(points));
    Wire.write((byte*)&timeTaken, sizeof(timeTaken));
  }
  else if (inStage5 == false && finished == true) {
    int finishedStage = 5;
    Wire.write((byte*)&finishedStage, sizeof(finishedStage));
    Wire.write((byte*)&stg5Points, sizeof(stg5Points));
    Wire.write((byte*)&points, sizeof(points));
    Wire.write((byte*)&timeTaken, sizeof(timeTaken));
    Wire.write((byte*)&totalTime, sizeof(totalTime));
  }
}
