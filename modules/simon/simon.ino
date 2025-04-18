#include <Adafruit_NeoPixel.h>

#define BUTTON_RED_PIN    0
#define BUTTON_GREEN_PIN  2
#define BUTTON_BLUE_PIN   4
#define BUTTON_YELLOW_PIN 6

#define LED_RED_PIN       1
#define LED_GREEN_PIN     3
#define LED_BLUE_PIN      5
#define LED_YELLOW_PIN    7

#define STATUS_LED 17

Adafruit_NeoPixel status = Adafruit_NeoPixel(1, STATUS_LED, NEO_GRB + NEO_KHZ800);

struct ColorPin {
  int buttonPin;
  int ledPin;
  ColorPin(int button, int led) : buttonPin(button), ledPin(led) {}
};

struct Color {
  static const ColorPin RED;
  static const ColorPin GREEN;
  static const ColorPin BLUE;
  static const ColorPin YELLOW;
};

const ColorPin Color::RED(BUTTON_RED_PIN, LED_RED_PIN);
const ColorPin Color::GREEN(BUTTON_GREEN_PIN, LED_GREEN_PIN);
const ColorPin Color::BLUE(BUTTON_BLUE_PIN, LED_BLUE_PIN);
const ColorPin Color::YELLOW(BUTTON_YELLOW_PIN, LED_YELLOW_PIN);

const ColorPin colors[] = {Color::RED, Color::GREEN, Color::BLUE, Color::YELLOW};

char vowels[] = {'A', 'E', 'I', 'O', 'U'};

int colorTableWithVowel[3][4] = {
  {1, 0, 3, 2}, // 0 erreur
  {2, 3, 1, 0}, // 1 erreur
  {3, 2, 0, 1}  // 2 erreurs
};

int colorTableNoVowel[3][4] = {
  {2, 3, 0, 1}, // 0 erreur
  {0, 2, 3, 1}, // 1 erreur
  {1, 0, 2, 3}  // 2 erreurs
};

int sequence[10];
int userInputStep = 0;
int sequenceLength = 0;
int maxSequenceLength = 10;
int strikes = 0;
bool serialHasVowel = false;
bool inGame = false;


void setup() {
   pinMode(A1, INPUT);
   for (const auto& color : colors) {
       pinMode(color.buttonPin, INPUT_PULLUP);
       pinMode(color.ledPin, OUTPUT);
    }

    status.begin();
    status.clear();


    long seed = 0;
    for (int i = 0; i < 32; i++) {
        seed = (seed << 1) | (analogRead(A1) & 1);
    }
    randomSeed(seed);
    setupGame();
}

void loop() {
   if (inGame) {
      playSequence();
      checkUserInput();
    }
}

void setupGame() {
  strikes = 0; //TODO: this need to came from CAN bus
  serialHasVowel = hasVowel("SERIAL_NUMBER"); //TODO: this need to came from CAN bus

  userInputStep = 0;
  sequenceLength = 1;
  maxSequenceLength = random(5,11);
  inGame = true;

  for (int i = 0; i < maxSequenceLength; i++) {
    sequence[i] = random(0, 4);
  }
}

void setStatusColor(uint8_t red, uint8_t green, uint8_t blue) {
  status.setPixelColor(0, status.Color(red, green, blue));
  status.show();
}

void turnOn(const ColorPin& color) {
    digitalWrite(color.ledPin, HIGH);
}

void turnOff(const ColorPin& color) {
    digitalWrite(color.ledPin, LOW);
}

void handleStrike() {
  strikes++;
  if (strikes >= 3) {
    inGame = false;
    flashAllLeds(10, 500);
    setStatusColor(255, 0, 0);
  }
}

void flashAllLeds(int times, int duration) {
  for (int i = 0; i < times; i++) {
    for (const auto& color : colors) {
      turnOn(color);
    }
    delay(duration);
    for (const auto& color : colors) {
      turnOff(color);
    }
    delay(duration);
  }
}

void playSequence() {
  for (int i = 0; i < sequenceLength; i++) {
    turnOn(colors[sequence[i]]);
    delay(500);
    turnOff(colors[sequence[i]]);
    delay(250);
  }
}

int translateInput(int color) {
    return serialHasVowel ? colorTableWithVowel[strikes][color] : colorTableNoVowel[strikes][color];
}

void checkUserInput() {
  for (int step = 0; step < sequenceLength; step++) {
    bool pressed = false;
    while (!pressed) {
      for (int i = 0; i < 4; i++) {
        if (digitalRead(colors[i].buttonPin) == LOW) {
          int userColor = i; 
          pressed = true;
          delay(500);
          
          int expectedColor = translateInput(sequence[step]);
          if (userColor != expectedColor) {
            handleStrike(); 
            return;
          }
        }
      }
    }
  }

  sequenceLength++;
  if (sequenceLength > maxSequenceLength) {
    inGame = false;
    setStatusColor(0, 255, 0);
  }
}

bool hasVowel(String serial) {
  for (int i = 0; i < serial.length(); i++) {
    char c = toupper(serial[i]);
    for (char vowel : vowels) {
      if (c == vowel) {
        return true;
      }
    }
  }
  return false;
}
