#include <ArtnetWifi.h>
#include <Arduino.h>
#include <FastLED.h>

// Wifi settings
const char* ssid = "CebskiDIY";
const char* password = "Assolutamenteno";
//const char* localPort = "8888"; 

// LED settings
const byte dataPin = 26;
const int ledsX = 4;
const int ledsY = 60;
const int ledsZ = 4;
const int ledsXY = ledsX * ledsY;
const int numLeds = ledsX * ledsY * ledsZ; // (+20 a little tollerance because of the way max msp is storing data)
const int numberOfChannels = numLeds * 3;  // Total number of channels you want to receive (1 led = 3 channels) 

// first eps32 with 10 strips (600 leds)
// second esp32 with 6 strips  (360 leds)
const int firstBoardLeds = 600;
const int secondBoardLeds = 360;


#define LED_TYPE WS2813
const int testcolor = 500;  // color test in milliseconds while initializing
#define LED 2               // wifi connection monitoring

CRGB leds[numLeds];

// Art-Net settings
ArtnetWifi artnet;
const int startUniverse = 0;  // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.

// Check if we got all universes
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
bool universesReceived[maxUniverses];
bool sendFrame = 1;


// connect to wifi – returns true if successful or false if not
bool ConnectWifi(void) {
  bool state = true;
  int i = 0;

  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 20) {
      state = false;
      break;
    }
    i++;
  }
  if (state) {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    digitalWrite(LED, HIGH);
  } else {
    Serial.println("");
    Serial.println("Connection failed.");
    digitalWrite(LED, LOW);
  }

  return state;
}

void initTest() {
  for (int i = 0; i < numLeds; i++) {
    leds[i] = CRGB(127, 0, 0);
  }
  FastLED.show();
  delay(testcolor);
  for (int i = 0; i < numLeds; i++) {
    leds[i] = CRGB(0, 127, 0);
  }
  FastLED.show();
  delay(testcolor);
  for (int i = 0; i < numLeds; i++) {
    leds[i] = CRGB(0, 0, 127);
  }
  FastLED.show();
  delay(testcolor);
  for (int i = 0; i < numLeds; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data) {
  sendFrame = 1;
  // set brightness of the whole strip
  /*if (universe == 15) {
    FastLED.setBrightness(data[0]);
    //Serial.println(data[0]);
    FastLED.show();
  }*/

  // range check
  if (universe < startUniverse) {
    return;
  }
  uint8_t index = universe - startUniverse;
  if (index >= maxUniverses) {
    return;
  }

  // Store which universe has got in
  universesReceived[index] = true;

  for (int i = 0; i < maxUniverses; i++) {
    if (!universesReceived[i]) {
      sendFrame = 0;
      break;
    }
  }

  // read universe and put into the right part of the display buffer
  for (int i = 0; i < length / 3; i++) {

   /*static int reverseStrip = 0;
    static int reversePlane = 0;
    int led;  //final index output. !!!!!!!!!!!!!!!!!!!!!!!!!


    // mapping pixels for LEDS in series
    if ((i % ledsXY) == 0) {
      reversePlane = (reversePlane + 1) % 2;  //if it's 1 it will reverse the index order of the next Plane
    }
    if ((i % ledsY) == 0) {
      reverseStrip = (reverseStrip + 1) % 2;  //if it's 1 it will reverse the index order of the next Strip
    }

    if (reversePlane == 1) {
      if (reverseStrip == 0) {
        led = i + (index * 170);
      } else if (reverseStrip == 1) {
        led = i + (index * 170) + (ledsY - (i % ledsY) - 1 - (i % ledsY));  // 24 = ledX * ledy
      }
    } else if (reversePlane == 0) {
      if (reverseStrip == 1) {
        led = i + (index * 170) + (ledsXY - (i % ledsXY) - 1 - (i % ledsXY));  // 8 = ledY
      } else if (reverseStrip == 0) {
        led = i + (index * 170) + (ledsXY - (i % ledsXY) - 1 - (i % ledsXY)) - ((ledsY - 1) - (i % ledsY) - (i % ledsY));  // 24 = ledX * ledy. 8 = ledY
      }
    }*/

    /*
    if (reverseStrip == 0) {
      led = i + (index * 170);
    } else if (reverseStrip == 1) {
      led = i + (index * 170) + (8 - (i % 8) - 1 - (i % 8));  // 8 = ledY
    }
*/
/*
   int led = i + (index * 170);
    if (led < numLeds) {
      leds[led] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
    }
  }
*/

int led = i + (index * 170);
    if (led < firstBoardLeds) {
      leds[led] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
    }
  }

  /*int led = i + (index * 170);
  if (led >= firstBoardLeds && led < numLeds) {
      int secondLed = led - firstBoardLeds;
      leds[secondLed] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
    }
  }*/

  if (sendFrame) {
    FastLED.show();
    // Reset universeReceived to 0
    memset(universesReceived, 0, maxUniverses);
  }
}


void setup() {
  FastLED.clear();
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  ConnectWifi();
  artnet.begin();
  FastLED.addLeds<LED_TYPE, dataPin, RGB>(leds, numLeds);

  initTest();

  memset(universesReceived, 0, maxUniverses);
  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);
  FastLED.setBrightness(255);
  FastLED.show();
}

void loop() {
  // we call the read function inside the loop
  artnet.read();
}
