#define FASTLED_ESP8266_RAW_PIN_ORDER

#include <FastLED.h>
#include <RTClib.h>

#include <Wire.h>
#include "RTClib.h"

#define NUM_LEDS 114
#define DATA_PIN 5


RTC_DS1307 rtc;
CRGB leds[NUM_LEDS];
boolean enableLeds[NUM_LEDS];
boolean disableLeds[NUM_LEDS];

// Coordinates on grid
const byte ES[2] = {0, 2};
const byte IST[2] = {3, 3};
const byte FUENF_MIN[2] = {7, 4};
const byte ZEHN_MIN[2] = {18, 4};
const byte VIERTEL[2] = {11, 7};
const byte ZWANZIG[2] = {22, 7};
const byte VOR[2] = {30, 3};
const byte NACH[2] = {40, 4};
const byte HALB[2] = {33, 4};
const byte UHR[2] = {96, 3};
const byte GUTEN[2] = {105, 5};
const byte MORGEN[2] = {99, 6};


#define VIER {44, 4}
#define FUENF_HR {51, 4}
#define ZWEI {62, 4}
#define EINS {60, 4}

#define SIEBEN {55, 6}
#define SECHS {66, 5}
#define ZWOELF {72, 5}
#define ACHT {84, 4}
#define ZEHN_HR {80, 4}
#define NEUN {77, 4}
#define DREI {88, 4}
#define ELF {92, 3}

byte EIN [2] = {61, 3};

const byte HOURS [12][2] = { ZWOELF, EINS, ZWEI, DREI, VIER, FUENF_HR, SECHS, SIEBEN, ACHT, NEUN, ZEHN_HR, ELF };


void enable_word(const byte b[2]) {
  for (int i = b[0]; i < b[0] + b[1]; i++) {
    enableLeds[i] = true;  
  }
}


// Input: t in seconds
// Output: Coordinates to light up for hours (first byte is length of array)
void enable_hrs(byte hours, byte minutes) {
  if (hours > 5 && hours < 10) {  
    enable_word(GUTEN);
    enable_word(MORGEN);
  }
  if (minutes >= 25) hours++;
  const byte* w = HOURS[hours % 12];
  if (hours == 1 && minutes / 5 == 0) { // ES IST EIN UHR nicht ES IST EINS UHR
    w = EIN;
  }
  enable_word(w);
}

void enable_time(byte hours, byte mins) {

  enable_hrs(hours, mins);
  int fm = (mins / 5);

  byte detailed_minutes[2] = {NUM_LEDS - 4, mins % 5};
  enable_word(detailed_minutes);
  
  enable_word(ES);
  enable_word(IST);
  byte rounded_time = ((mins / 5) * 5) % 60;
  if ((rounded_time % 30) == 5) { // FÜNF NACH HALB; FÜNF NACH
    enable_word(FUENF_MIN);
    enable_word(NACH);
  } else if ((rounded_time % 30) == 25) { // FÜNF VOR HALB; FÜNF VOR
    enable_word(FUENF_MIN);
    enable_word(VOR);
  }
  if (rounded_time == 25 || rounded_time == 30 || rounded_time == 35) {
    enable_word(HALB);
    return;
  }
  
  if (rounded_time == 20 || rounded_time == 40) {
    enable_word(ZWANZIG);
  } else if (rounded_time == 15 || rounded_time == 45) {
    enable_word(VIERTEL);
  } else if (rounded_time == 10 || rounded_time == 50) {
    enable_word(ZEHN_MIN);
  }

  if (rounded_time == 0) {
    enable_word(UHR);
  } else if (rounded_time > 30) {
    enable_word(VOR);
  } else {
    enable_word(NACH);
  }

}

int brightness = 150;

void changeHue(long seconds_of_day) {
  int hue = (seconds_of_day / 337) - 128;
  for (int s = 0; s < brightness; s++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      if (enableLeds[i]) {
        leds[i] = CHSV(hue, 255, s);
      } else if (disableLeds[i]) {
        leds[i].fadeToBlackBy(s);
      }
    }
    FastLED.show();
    delay(5);
  }
  for (int i = 0; i < NUM_LEDS; i++) {
    CRGB led = leds[i];
    if (led.r != 0 || led.g != 0 || led.b != 0) {
      leds[i] = CHSV(hue, 255, brightness);
    }
  }
  
}



void setup() {
  Wire.begin();
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  rtc.begin();
}

int lastMinute = -1;


void loop() {
  while (true) {
    DateTime now = rtc.now();
    
    int mnt = now.minute();
    int hr = now.hour();
    int snd = now.second();

    if (mnt != lastMinute) {
      lastMinute = mnt;

      // Show new time

       // Reset enableLeds and disableLeds arrays
      for (int i = 0; i < NUM_LEDS; i++) {
        enableLeds[i] = false;
        disableLeds[i] = false;
      }

      
      enable_time(hr, mnt);

      // Set enableLeds and disableLeds to actual 
      for (int i = 0; i < NUM_LEDS; i++) {
        CRGB led = leds[i];
        if (led.r != 0 || led.g != 0 || led.b != 0) {
          if (enableLeds[i]) {
            enableLeds[i] = false;
          } else {
            disableLeds[i] = true;
          }
        }
      }
          
      changeHue(((long) hr) * 60 * 60 + (long) mnt * 60 + (long) snd);
       
    }
    
    delay(1000);
  }
}

