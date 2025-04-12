#include <Arduino.h>
#include <TM1637Display.h>

#define CLK 2
#define DIO 3

TM1637Display display(CLK, DIO);

float timeLeft = 300.0;   // 5 minutes in seconds
int strikes = 0;

unsigned long lastUpdate = 0;
unsigned long lastBlink = 0;
bool blinkState = false;

void setup() {
  display.setBrightness(7);
  lastUpdate = millis();
  lastBlink = millis();
}

void loop() {
  unsigned long now = millis();

  if (timeLeft > 0.0) {
    float speedMultiplier = 1.0 + (0.25 * strikes);
    float elapsed = (now - lastUpdate) / 1000.0 * speedMultiplier;

    if (elapsed >= 0.01) {
      timeLeft -= elapsed;
      if (timeLeft < 0.0) timeLeft = 0.0;
      lastUpdate = now;

      if (timeLeft > 10.0) {
        int totalSec = (int)timeLeft;
        int minutes = totalSec / 60;
        int seconds = totalSec % 60;

        uint8_t dots = 0b01000000;  // Colon between minutes and seconds

        display.showNumberDecEx(minutes * 100 + seconds, dots, true, 4, 0); // Show MM:SS
      } else {
        int sec = (int)timeLeft;
        int hundredths = (int)((timeLeft - sec) * 100);

        uint8_t dots = 0b00100000;  // Colon between seconds and hundredths (SS.MM)
        display.showNumberDecEx(sec * 100 + hundredths, dots, false, 4, 0); // Show SS.mm
      }
    }
  } else {
    // Blink 88:88 forever
    if (now - lastBlink >= 500) {
      lastBlink = now;
      blinkState = !blinkState;

      if (blinkState) {
        uint8_t data[] = {0xff, 0xff, 0xff, 0xff}; // all segments on
        data[1] |= 0x80; // colon on
        display.setSegments(data);
      } else {
        display.clear();
      }
    }
  }
}
