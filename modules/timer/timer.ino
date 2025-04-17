#include <Arduino.h>
#include <TM1637Display.h>
#include <Adafruit_NeoPixel.h> 
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>


#define CLK 2
#define DIO 3
#define PIXEL_PIN 13
#define PIEZO_PIN 17

#define DEFAULT_TIMER 300.0 // 5 min in seconds

#define START_BUTTON_PIN 5
#define STRIKE_BUTTON_PIN 6
#define WIN_BUTTON_PIN 7
#define RESET_BUTTON_PIN 8

int lastDisplayedSeconds = -1;


const int numPixels = 4; // each strike had 2 LED
Adafruit_NeoPixel strip = Adafruit_NeoPixel(numPixels, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
TM1637Display display(CLK, DIO);

float timeLeft = DEFAULT_TIMER; 
int strikes = 0;

unsigned long lastUpdate = 0;
unsigned long lastBlink = 0;
bool blinkState = false;

bool gameOver = false;
bool gameWon = false;
bool gameStarted = false;

void setup() {
  Serial.begin(9600);

  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STRIKE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(WIN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PIEZO_PIN, OUTPUT);

  display.setBrightness(7);
  
  strip.begin();
  strip.setBrightness(20);
  
  resetGame();
}

void loop() {
  unsigned long now = millis();

  handleButtons();

  // Game Lost
  if (!gameOver && !gameWon && (timeLeft <= 0.0 || strikes >= 3)) {
    gameOver = true;
    timeLeft = 0.0;
  }

  if (gameOver) {
    if (now - lastBlink >= 500) {
      lastBlink = now;
      blinkState = !blinkState;

      // Blink only relevant LEDs based on strike count
      strip.clear();
      int ledsToBlink = strikes >= 2 ? 4 : 2;
      for (int i = 0; i < ledsToBlink && i < numPixels; i++) {
        strip.setPixelColor(i, blinkState ? strip.Color(255, 0, 0) : 0);
      }
      strip.show();

      // Blink 88:88
      if (blinkState) {
        uint8_t data[] = {0xff, 0xff, 0xff, 0xff};
        data[1] |= 0x80;
        display.setSegments(data);
      } else {
        display.clear();
      }
    }
    return;
  }

  if (gameWon) {
    if (now - lastBlink >= 500) {
      lastBlink = now;
      blinkState = !blinkState;

      // Blink same LEDs used during play to represent strikes
      strip.clear();
      int ledsToBlink = strikes >= 2 ? 4 : (strikes == 1 ? 2 : 0);
      if( strikes > 0){
        for (int i = 0; i < ledsToBlink && i < numPixels; i++) {
          strip.setPixelColor(i, blinkState ? strip.Color(255, 0, 0) : 0);
        }
      }
      strip.show();

      // Blink final time
      if (blinkState) {
        int totalSec = (int)timeLeft;
        int minutes = totalSec / 60;
        int seconds = totalSec % 60;
        display.showNumberDecEx(minutes * 100 + seconds, 0b01000000, true, 4, 0);
      } else {
        display.clear();
      }
    }
    return;
  }


  strip.clear();
  if (strikes == 1) {
    strip.setPixelColor(0, strip.Color(255, 0, 0));
    strip.setPixelColor(1, strip.Color(255, 0, 0));
  } else if (strikes == 2) {
    strip.setPixelColor(0, strip.Color(255, 0, 0));
    strip.setPixelColor(1, strip.Color(255, 0, 0));
    strip.setPixelColor(2, strip.Color(255, 0, 0));
    strip.setPixelColor(3, strip.Color(255, 0, 0));
  }
  strip.show();

  // Timer logic
  if (gameStarted && timeLeft > 0.0) {
    float speedMultiplier = 1.0 + (0.25 * strikes);
    float elapsed = (now - lastUpdate) / 1000.0 * speedMultiplier;

    if (elapsed >= 0.01) {
      int displayedSec = 0;
      
      timeLeft -= elapsed;
      if (timeLeft < 0.0) timeLeft = 0.0;
      lastUpdate = now;

      if (timeLeft > 10.0) {
        int totalSec = (int)timeLeft;
        int minutes = totalSec / 60;
        int seconds = totalSec % 60;
        displayedSec = totalSec;
        display.showNumberDecEx(minutes * 100 + seconds, 0b01000000, true, 4, 0);
      } else {
        int sec = (int)timeLeft;
        int hundredths = (int)((timeLeft - sec) * 100);
        displayedSec = sec;
        display.showNumberDecEx(sec * 100 + hundredths, 0b01000000, false, 4, 0);
      }
    
      if (displayedSec != lastDisplayedSeconds) {
        lastDisplayedSeconds = displayedSec;

        // Every full minute
        if (displayedSec % 60 == 0 && displayedSec != 0) {
          bip();
        }

        // At 30, 20, and 15 seconds left
        if (displayedSec == 30 || displayedSec == 20 || displayedSec == 15) {
          bip();
        }

        // From 10 to 1 seconds
        if (displayedSec <= 10 && displayedSec >= 1) {
          bip();
        }
      }
    }
  }
}

void bip(){
  digitalWrite(PIEZO_PIN, HIGH);  
  delay(100);
  digitalWrite(PIEZO_PIN, LOW);
}

void handleButtons(){
  //debug method to trigger methods 

  if (digitalRead(START_BUTTON_PIN) == LOW) {
    gameStart();
  }

  if (digitalRead(STRIKE_BUTTON_PIN) == LOW) {
      addStrike();
  }

  if (digitalRead(WIN_BUTTON_PIN) == LOW) {
    winGame();
  }

  if (digitalRead(RESET_BUTTON_PIN) == LOW) {
    resetGame();
  }
}

void gameStart() {
  if (!gameStarted && !gameOver && !gameWon) {
    gameStarted = true;
    lastUpdate = millis();
  }
}

void addStrike() {
  if (!gameOver && !gameWon && strikes < 3) {
    strikes++;
  }
}

void winGame() {
  if (!gameOver && !gameWon) {
    gameWon = true;
    gameStarted = false;  // stop timer
    timeLeft = max(0.0, timeLeft);
  }
}

void resetGame() {
  timeLeft = DEFAULT_TIMER;
  strikes = 0;
  lastUpdate = millis();
  lastBlink = millis();
  blinkState = false;
  gameOver = false;
  gameWon = false;
  gameStarted = false;

  strip.clear();
  strip.show();
  display.clear();
  int minutes = (int) timeLeft / 60;
  int seconds = (int) timeLeft % 60;
  display.showNumberDecEx(minutes * 100 + seconds, 0b01000000, true, 4, 0);
}

