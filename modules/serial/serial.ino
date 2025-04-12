#include <Adafruit_NeoPixel.h> 
#include <LCDI2C_Multilingual.h>  

/**
This code will handle Serial Code & Identificateur display for KTANE
**/
const int pixelPin = 13;
const int numPixels = 1;

// I2C address: 0x27; Display size: 16x2
// SDL: GPIO5 , SDA: GPIO4
LCDI2C_UTF8 lcd(0x27, 16, 2);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(numPixels, pixelPin, NEO_GRB + NEO_KHZ800);

String indicators[] = {"SND", "CLR", "CAR", "IND", "FRG", "SIG", "NSA", "MSA", "TRN", "BOB", "FRK"};
int listSize = sizeof(indicators) / sizeof(indicators[0]);


String serialNumber = "";
String indicator = "";


String generateSerialNumber(int randomizeSeed) {
  randomSeed(analogRead(randomizeSeed));
  String serial = "";
  bool hasLetter = false;
  bool hasDigit = false;

  
  int serialLength = random(5, 9); 

  // Ensure there's at least one number at the end
  for (int i = 0; i < serialLength - 1; i++) { 
    if (random(0, 2) == 0) {  // Randomly decide if it's a letter or number
      // Randomly pick a vowel (A, E, I, U) or a consonant (excluding O)
      char letter = random(0, 2) == 0 ? random('A', 'Z') : random('A', 'Z');
      
      // Make sure the letter is not 'O'
      while (letter == 'O') {
        letter = random('A', 'Z');
      }
      
      serial += letter;
      hasLetter = true;
    } else {
      char digit = random(0, 2) == 0 ? random('1', '9') : '0';
      serial += digit;
      hasDigit = true;
    }
  }
  

  char lastDigit = random('0', '9');
  serial += lastDigit;
  
  return serial;
}

void reset(){
  randomSeed(analogRead(0));
  int randomNum = random(0,100);

  int ledState = random(0, 10000);
  strip.setBrightness(20);
  strip.setPixelColor(0, strip.Color(255, 255, 255));
  
  if(ledState%2==0){
    strip.show();    
  }

  

  do{
    randomSeed(analogRead(randomNum));
    int randomIndex = random(0, listSize);
    serialNumber = generateSerialNumber(randomNum);
    indicator = indicators[randomIndex];
  
    lcd.println("Serial #"+serialNumber, 3); 
    lcd.setCursor(0, 1);
    lcd.println("Indicateur: "+indicator,3);
    randomNum--;
  }while(randomNum>0);
}

void setup() {
  
  lcd.init();
  lcd.backlight();
  reset();

}

void loop() {}
