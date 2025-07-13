#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <Adafruit_NeoPixel.h>
#include <HardwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <OSCBundle.h>

// ---------- CONFIGURATION ----------
const char* ssid = "ESP32-C3-AP";
const char* password = "12345678";

const IPAddress broadcastIP(192, 168, 4, 255);
const int destPort = 9000;
const int localPort = 8888;

#define ANALOG_PIN 1    // GPIO1 for analog input
#define LED_PIN 10      // GPIO10 for NeoPixel
#define NUMPIXELS 1
#define DF_RX 3         // DFPlayer TX → ESP32 RX
#define DF_TX 4         // DFPlayer RX → ESP32 TX

int threshold = 30;

// ---------- GLOBALS ----------
Adafruit_NeoPixel pixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
WiFiUDP Udp;
HardwareSerial dfSerial(1);
DFRobotDFPlayerMini myDFPlayer;

bool clientConnected = false;
int lastClientCount = 0;

// ---------- FUNCTIONS ----------
void setLED(uint8_t r, uint8_t g, uint8_t b) {
  pixel.setPixelColor(0, pixel.Color(g, r, b));
  pixel.show();
}

void handleOSCMessage(OSCMessage &msg, int addrOffset) {
  if (msg.fullMatch("/play")) {
    int32_t playValue = msg.getInt(0);  // Get the first argument as int
    Serial.print("Received /play ");
    Serial.println(playValue);

    if (playValue == 1) {
      Serial.println("Playing sound due to OSC.");
      myDFPlayer.play(1);
    }
  }
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);
  delay(1000);

  pixel.begin();
  setLED(255, 0, 0);  // 🔴 Red - starting

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  delay(500);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  setLED(0, 255, 0);  // 🟢 Green - AP running, no clients
  Udp.begin(localPort);

  dfSerial.begin(9600, SERIAL_8N1, DF_RX, DF_TX);
  if (!myDFPlayer.begin(dfSerial)) {
    Serial.println("DFPlayer Mini not found!");
  } else {
    Serial.println("DFPlayer Mini ready.");
    myDFPlayer.volume(20);
    myDFPlayer.play(1);
  }
}

// ---------- LOOP ----------
void loop() {
  int clientCount = WiFi.softAPgetStationNum();
  if (clientCount != lastClientCount) {
    Serial.print("Client count: ");
    Serial.println(clientCount);
    clientConnected = (clientCount > 0);
    setLED(0, clientConnected ? 0 : 255, clientConnected ? 255 : 0);
    lastClientCount = clientCount;
  }

  // Read analog and send OSC
  int val = analogRead(ANALOG_PIN);
  if (clientConnected) {
    OSCMessage msg("/wind");
    msg.add(val);
    Udp.beginPacket(broadcastIP, destPort);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
  }

  // Local threshold trigger
  if (val > threshold) {
    if (myDFPlayer.available()) {
      if (myDFPlayer.readType() == DFPlayerPlayFinished) {
        myDFPlayer.play(1);
      }
    }
  }

  // ---------- Handle OSC In via Bundle ----------
  OSCBundle bundle;
  int size = Udp.parsePacket();
  if (size > 0) {
      while (size--) {
          bundle.fill(Udp.read());
      }

      if (!bundle.hasError()) {
          bundle.route("/play", handleOSCMessage);
      } else {
          Serial.print("OSC Error: ");
          Serial.println(bundle.getError());
      }
  }


  delay(50);
}
