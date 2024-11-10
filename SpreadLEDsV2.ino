#include <FastLED.h>
#include <WiFi.h>
#include <OSCMessage.h>

#define NUM_LEDS 560 // 560 - 240
#define LED_TYPE WS2813 //WS2812B
#define LED 2 
#define LEDt 1
#define DATA_PIN 16 // 19-16
#define MAX_INSTANCES 12
#define ACCUM_ARRAY_SIZE 1000 //  ????????

const char *ssid = "CebskiDIY";             //replace id 
const char *password = "Assolutamenteno";   //replace pass
const unsigned int localPort = 8888;        //replace port

float accumBuffer[ACCUM_ARRAY_SIZE];

CRGB source1[NUM_LEDS];
CRGB source2[NUM_LEDS];
CRGB leds[NUM_LEDS];

CRGB full[NUM_LEDS];
CRGB output[NUM_LEDS];

//index forward or backward
float currentIndex = 0;
float prevIndex = 0;
float diffIndex = 0;
int blendAmount = 0;
int solidAmount = 0;

float value1 = 0.3;  // Bell width (as a float)
float value2 = 0.3;  // Position of the peak of the bell curve (as a float)
float value3 = 0.3;      // Brightness
float value4 = 0.3;      // Blend wave and full/expansion
float value5 = 0.3;      // Random Trigger
float value6 = 0.3;   // Decrement factor

//init ramp
float lfob_freq = 1.0;
float lfob_phase = 0.0;
float lfob_target_frequency = 0.0;
float lerp_factor = 0.5;

//For randomPOS and expansion function
//float decrFactor = 2.0;
float iBrightness[MAX_INSTANCES];
bool isActive[MAX_INSTANCES];
bool oneShot[MAX_INSTANCES];
int rtp[MAX_INSTANCES];
float bellWidth[MAX_INSTANCES];
int OFFonFLAG[MAX_INSTANCES];

WiFiUDP Udp;

//////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {

  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  Serial.println("\nConnecting to " + String(ssid));

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.println("IP address: " + WiFi.localIP().toString());

  Udp.begin(localPort);
  Serial.println("Starting UDP");
  Serial.println("Local port: " + String(localPort));

  if (WiFi.status() == WL_CONNECTED){ //monitoring connection status via built-in led
    digitalWrite(LED, HIGH);
  } else digitalWrite(LED, LOW);
  FastLED.addLeds<LED_TYPE, DATA_PIN, GRB>(output, NUM_LEDS);
  //FastLED.setCorrection(TypicalLEDStrip); // color correction

  for (int i = 0; i < MAX_INSTANCES; i++) {  // initialize instances for randomTpos function
    isActive[i] = false;
    oneShot[i] = false;
  }
  FastLED.clear();
  FastLED.show();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {

  /*recvWithEndMarker();
  if (newData) {
    parseData();
    newData = false;
    if (value5 >= 0) {
      //Serial.println("v1: " + String(value5));
      triggerINIT();    // trigger for animation
      randomWAVEpos();  // animation
    }
    if (value5 == -1) {
      randomWAVEpos();
    }
  }*/

  OSCMessage msg;
  int size = Udp.parsePacket();
  
  if (size > 0) {
    while (size--) {
      msg.fill(Udp.read());
    }  
    if (!msg.hasError()) {
      msg.dispatch("/value1", value1_func);
      msg.dispatch("/value2", value2_func);
      msg.dispatch("/value3", value3_func);
      msg.dispatch("/value4", value4_func);
      msg.dispatch("/value5", value5_func);
      msg.dispatch("/value6", value6_func);
    }
  }

  lfob_target_frequency = value2;
  lfob_freq += lerp_factor * (lfob_target_frequency - lfob_freq);  // Smoothly interpolate the frequencies
  lfob_phase += lfob_freq * 0.001;
  if (lfob_phase >= 1.0) {
    lfob_phase -= 1.0;
  }
  float lfob = (lfob_phase < 0.5) ? lfob_phase * 2 : (1.0 - lfob_phase) * 2;  // Calculate the triangular wave value
  float seqIndex = lfob * NUM_LEDS - 1;
  currentIndex = seqIndex;
  diffIndex = currentIndex - prevIndex;
  if (diffIndex > 0) {  // if it goes forward then half-bell sinusoid on the left side
    blendAmount = 0;
  } else if (diffIndex < 0) {  // if it goes backwards then half-bell sinusoid on the right side
    blendAmount = 255;
  }
  solidAmount = value4;
  halfBellForward(seqIndex);   //animation
  halfBellBackward(seqIndex);  //animation

  //fullStrip(value3);
  //fullBell(seqIndex);
  blend(source1, source2, leds, NUM_LEDS, blendAmount);  //leds = output
  blend(leds, full, output, NUM_LEDS, solidAmount);

  FastLED.show();
  prevIndex = seqIndex;
  //Serial.println("iPrev: " + String(prevIndex));
}

/**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/

/*///////////////////////////////////////*/ void halfBellForward(float seqIndex) {
  seqIndex += 1;
  float bellWidth = value1;
  //float bExp = value5 * -1.;
  float peakPosition = seqIndex;
  for (int i = 0; i < NUM_LEDS; i++) {
    float distance = abs(i - peakPosition);                                                                      // Calculate the distance to the peak position
    float brightness = (i <= peakPosition) ? exp(-2.5 * (distance * distance) / (bellWidth * bellWidth)) : 0.0;  // Create a half bell curve by limiting the brightness to one side of the peak
    source1[i] = CHSV(250, 0, brightness * value3);
  }
}
/*///////////////////////////////////////*/ void halfBellBackward(float seqIndex) {
  float bellWidth = value1;
  //float bExp = value5 * -1.;
  float peakPosition = seqIndex;
  for (int i = 0; i < NUM_LEDS; i++) {
    float distance = abs(i - peakPosition);                                                                      // Calculate the distance to the peak position
    float brightness = (i >= peakPosition) ? exp(-2.5 * (distance * distance) / (bellWidth * bellWidth)) : 0.0;  // Create a half bell curve by limiting the brightness to one side of the peak
    source2[i] = CHSV(250, 0, brightness * value3);
  }
}

/*///////////////////////////////////////*/ void randomWAVEpos() {
  float incremFactor = value6;
  memset(accumBuffer, 0, sizeof(accumBuffer));
  for (int i = 0; i < MAX_INSTANCES; i++) {
    if (isActive[i]) {
      if (value5 >= 0 && oneShot[i]) {        // one shot trigger
        bellWidth[i] = 0.3;                   // value5? or 2?
        for (int j = 0; j < NUM_LEDS; j++) {
          float distance = abs(j - rtp[i]);   //rtp=peakposition
          float wBrightness = exp(-2.5 * (distance * distance) / (bellWidth[i] * bellWidth[i]));
          float wBrightScaled = (wBrightness * value3) * OFFonFLAG[i];
          accumBuffer[j] += wBrightScaled;
        }
        oneShot[i] = false;
      } else if (value5 == -1 || (value5 >= 0 && !oneShot[i]) || isActive[i]) {  // countinuous trigger
        bellWidth[i] += incremFactor;                                            // increment
        for (int j = 0; j < NUM_LEDS; j++) {
          float distance = abs(j - rtp[i]);  //rtp=peakposition
          float wBrightness = exp(-2.5 * (distance * distance) / (bellWidth[i] * bellWidth[i]));
          float wBrightScaled = (wBrightness * value3);
          accumBuffer[j] += wBrightScaled;
        }
        if (bellWidth[i] >= value1) {
          bellWidth[i] = 0;
          isActive[i] = false;
        }
      }
    }
    for (int k = 0; k < NUM_LEDS; k++) {  
        int constrainedBrightness = constrain(accumBuffer[k], 0, 255); // Apply the accumulated values to the 'full' array
      full[k] = CHSV(250, 0, constrainedBrightness);
    }
  }
}

void triggerINIT() {
  int index = checkTriggerVoice();
  if (index != -11) {
    //Serial.println(index);
    rtp[index] = value5;
    OFFonFLAG[index] = 1;
    oneShot[index] = true;
    isActive[index] = true;
  }
}
int checkTriggerVoice() {
  for (int i = 0; i < MAX_INSTANCES; i++) {
    if (!isActive[i]) {
      return i;
    }
  }
  return -11;
}

/*//////////////////////////////////////////// void fullBell(float seqIndex) {
  float bellWidth = value1;
  //float bExp = value5 * -1.;
  float peakPosition = seqIndex;
  for (int i = 0; i < NUM_LEDS; i++) {
    float distance = abs(i - peakPosition);
    float brightness = exp(-2.5 * (distance * distance) / (bellWidth * bellWidth));
    full[i] = CHSV(250, 0, brightness * value3);
  }
}*/

/**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/

void value1_func(OSCMessage &msg) { 
  value1 = msg.getFloat(0);
  //Serial.println("Value1: " + String(value1)); //getting message only when received 
}

void value2_func(OSCMessage &msg) {
  value2 = msg.getFloat(0);
  //Serial.println("Value2: " + String(value2)); //getting message only when received 
}

void value3_func(OSCMessage &msg) {
  value3 = msg.getFloat(0);
  //Serial.println("Value3: " + String(value3)); //getting message only when received 
}

void value4_func(OSCMessage &msg) {
  value4 = msg.getFloat(0);
  //Serial.println("Value4: " + String(value4)); //getting message only when received 
}

void value5_func(OSCMessage &msg) { // randomPosition
  value5 = msg.getFloat(0);
  if (value5 >= 0) {
      //Serial.println("v1: " + String(value5));
      triggerINIT();    // trigger for animation
      randomWAVEpos();  // animation
    }
    else if (value5 == -1) {
      randomWAVEpos();
    }
  //Serial.println("Value5: " + String(value5)); //getting message only when received 
}

void value6_func(OSCMessage &msg) { // expansion Time 
  value6 = msg.getFloat(0);
  //Serial.println("Value6: " + String(value6)); //getting message only when received 
}
