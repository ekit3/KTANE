#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>
#include <stdlib.h>

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
String serial1Message = "";

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
  handlePortCommunication(port1, 0);
  handlePortCommunication(port2, 1);
  handlePortCommunication(port3, 2);
  handlePortCommunication(port4, 3);
  handlePortCommunication(port5, 4);

  
  if (Serial1.available()) {
      char receivedChar = Serial1.read();
      if (receivedChar == '\n') {
        Serial.print("Received from RP2040 hub: ");
        Serial.println(serial1Message);
        broadcastMessage(serial1Message, -1);
        serial1Message = "";
      } else {
        serial1Message += receivedChar;
      }
    }

}

void handlePortCommunication(SoftwareSerial &port, int ledIndex) {
  static unsigned long lastBlinkTimes[NUM_PIXELS] = {0, 0, 0, 0, 0};
  static bool blinkStates[NUM_PIXELS] = {false, false, false, false, false};
  unsigned long currentTime = millis();

  if (port.available()) {
    String data = port.readStringUntil('\n');
    data.trim();
    Serial.print("Port ");
    Serial.print(ledIndex+1);
    Serial.print(" received: ");
    Serial.println(data);

    broadcastMessage(data, ledIndex+1);

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

void broadcastMessage(String message, int senderPort) {
  Serial.print("Broadcasting message from port ");
  if (senderPort == -1) Serial.print("RP2040 hub");
  else Serial.print(senderPort);
  Serial.print(": ");
  Serial.println(message);

  // Broadcast to all ports except the sender
  if (senderPort != 0) port1.println(message);
  if (senderPort != 1) port2.println(message);
  if (senderPort != 2) port3.println(message);
  if (senderPort != 3) port4.println(message);
  if (senderPort != 4) port5.println(message);
  if (senderPort != -1) Serial1.println(message);
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