#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>

#define RX_PIN 0
#define TX_PIN 1

// Ports SoftwareSerial
SoftwareSerial port1(2, 3); // RX, TX
SoftwareSerial port2(4, 5);
SoftwareSerial port3(6, 7);
SoftwareSerial port4(8, 9);
SoftwareSerial port5(10, 11);

#define PIN_NEOPIXEL 12
#define NUM_PIXELS 5
Adafruit_NeoPixel strip(NUM_PIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);


String incomingMessage = "";


void setup() {
  Serial.begin(115200);

  Serial1.begin(9600);


  port1.begin(9600);
  port2.begin(9600);
  port3.begin(9600);
  port4.begin(9600);
  port5.begin(9600);


  strip.begin();
  strip.setBrightness(25);
  strip.show();

  indicateStartup();

  Serial.println("RP2040 Hub Ready");

}

void loop() {
  handlePortCommunication(port1, 0, 1);
  handlePortCommunication(port2, 1, 2);
  handlePortCommunication(port3, 2, 3);
  handlePortCommunication(port4, 3, 4);
  handlePortCommunication(port5, 4, 5);

  
  if (Serial1.available()) {
    char receivedChar = Serial1.read();
    if (receivedChar == '\n') {
      processMessage();
      incomingMessage = "";
    } else {
      incomingMessage += receivedChar;
    }
  }

}

void handlePortCommunication(SoftwareSerial &port, int ledIndex,int portNumber) {
  static unsigned long lastBlinkTimes[NUM_PIXELS] = {0, 0, 0, 0, 0};
  static bool blinkStates[NUM_PIXELS] = {false, false, false, false, false};
  unsigned long currentTime = millis();

  if (port.available()) {
    String data = port.readString();
    Serial.print("Port ");
    Serial.print(ledIndex + 1);
    Serial.print(" received: ");
    Serial.println(data);

    // Exemple : redirect to  "Serial 4" of RP2040-2
    String message = String(portNumber) + ",4:" + data; // Format : source,destination:data
    Serial1.println(message);

    if (currentTime - lastBlinkTimes[ledIndex] > 200) {
      lastBlinkTimes[ledIndex] = currentTime;
      blinkStates[ledIndex] = !blinkStates[ledIndex];

      
      if (blinkStates[ledIndex]) {
        strip.setPixelColor(ledIndex, strip.Color(0, 255, 0));
      } else {
        strip.setPixelColor(ledIndex, strip.Color(0, 0, 0));
      }
      strip.show();
    }
  } else {
    strip.setPixelColor(ledIndex, strip.Color(0, 0, 0));
    strip.show();
  }
}

void indicateStartup() {
  for (int i = 0; i < NUM_PIXELS; i++) {
    strip.setPixelColor(i, strip.Color(255, 165, 0));
  }
  strip.show();
  delay(2000);

  
  for (int i = 0; i < NUM_PIXELS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();
}

void processMessage() {
  Serial.print("Message from RP2040-2: ");
  Serial.println(incomingMessage);

  
  int separatorIndex = incomingMessage.indexOf(':');
  if (separatorIndex != -1) {
    String header = incomingMessage.substring(0, separatorIndex); // source,destination
    String data = incomingMessage.substring(separatorIndex + 1); // data

    int commaIndex = header.indexOf(',');
    if (commaIndex != -1) {
      int sourcePort = header.substring(0, commaIndex).toInt();
      int destinationPort = header.substring(commaIndex + 1).toInt();

      if (destinationPort == 1) port1.println(data);
      if (destinationPort == 2) port2.println(data);
      if (destinationPort == 3) port3.println(data);
      if (destinationPort == 4) port4.println(data);
      if (destinationPort == 5) port5.println(data);
    }
  }
}
