#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <Adafruit_NeoPixel.h>

// ---------- CONFIGURATION ----------
const char* ssid = "ESP32-C3-AP";
const char* password = "12345678";
// const int apChannel = 6;

const IPAddress broadcastIP(192, 168, 4, 255);  // Replace with actual IP on AP
const int destPort = 9000;

#define ANALOG_PIN 1    // GPIO1 for analog input
#define LED_PIN 10      // GPIO10 for NeoPixel
#define NUMPIXELS 1

Adafruit_NeoPixel pixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
WiFiUDP Udp;

int lastClientCount = 0;
bool clientConnected = false;

void setLED(uint8_t r, uint8_t g, uint8_t b) {
  pixel.setPixelColor(0, pixel.Color(g, r, b));
  pixel.show();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pixel.begin();
  setLED(255, 0, 0);  // 🔴 Red - starting

  WiFi.mode(WIFI_AP);
  // WiFi.softAP(ssid, password, apChannel);
  WiFi.softAP(ssid, password);

  delay(500);  // Allow AP to settle
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  setLED(0, 255, 0);  // 🟢 Green - AP running, no clients

  Udp.begin(8888);  // Initialize UDP
  // Udp.beginPacketMulticast(broadcastIP, destPort, WiFi.softAPIP());  // multicast
}

void loop() {
  // Check if a client has connected to the AP
  int clientCount = WiFi.softAPgetStationNum();

  if (clientCount != lastClientCount) {
    Serial.print("Client count: ");
    Serial.println(clientCount);

    if (clientCount > 0) {
      clientConnected = true;
      setLED(0, 0, 255);  // 🔵 Blue - client connected
    } else {
      clientConnected = false;
      setLED(0, 255, 0);  // 🟢 Green - AP only
    }

    lastClientCount = clientCount;
  }

  if (clientConnected) {
    int val = analogRead(ANALOG_PIN);
    Serial.println(val);

    OSCMessage msg("/wind");
    msg.add(val);

    Udp.beginPacket(broadcastIP, destPort);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
  }

  delay(50);  // ~20Hz update rate
}
