#include <HardwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <Adafruit_NeoPixel.h>

#define DF_RX 3  // DFPlayer TX → ESP32 RX
#define DF_TX 4  // DFPlayer RX → ESP32 TX

#define ANALOG_PIN 1    // GPIO1 for analog input
#define LED_PIN 10      // GPIO10 for NeoPixel
#define NUMPIXELS 1

HardwareSerial dfSerial(1);  // Use UART1
DFRobotDFPlayerMini myDFPlayer;

int threshold = 20;

Adafruit_NeoPixel pixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setLED(uint8_t r, uint8_t g, uint8_t b) {
  pixel.setPixelColor(0, pixel.Color(g, r, b));
  pixel.show();
}

void setup() {
  Serial.begin(115200);

  pixel.begin();
  setLED(0, 255, 0);  // 🟢 Green

  Serial.begin(115200);  // Debug output
  dfSerial.begin(9600, SERIAL_8N1, DF_RX, DF_TX);  // DFPlayer Serial

  if (!myDFPlayer.begin(dfSerial)) {
    Serial.println("DFPlayer Mini not found!");
  } else {
    Serial.println("DFPlayer Mini ready.");
  }
  
  myDFPlayer.volume(20);  // Set volume (0–30)
  myDFPlayer.play(1);
}

void loop() {
  int val = analogRead(ANALOG_PIN);
  Serial.println(val);

  if (val > threshold) {
    if (myDFPlayer.available()) {
      uint8_t type = myDFPlayer.readType();
      
      if (type == DFPlayerPlayFinished) {
          myDFPlayer.play(1);     // Play first track
      }

    }
  }

  delay(50);  // ~20Hz update rate
}
