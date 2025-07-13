#include <Adafruit_NeoPixel.h>

#define LED_PIN 10      // GPIO10 for NeoPixel
#define NUMPIXELS 1

Adafruit_NeoPixel pixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setLED(uint8_t r, uint8_t g, uint8_t b) {
  pixel.setPixelColor(0, pixel.Color(g, r, b));
  pixel.show();
}

void setup() {
  Serial.begin(115200);

  pixel.begin();
  setLED(255, 0, 0);  // RED
}

void loop() {
  setLED(0, 0, 255);  // BLUE
  delay(400);  
  setLED(0, 0, 0);    // OFF
  delay(100);
  Serial.println("Running blink program");
}
