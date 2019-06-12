
/* three_sin_pal_demo
 *
 * By: Andrew Tuline
 *
 * Date: March, 2015
 *
 * 3 sine waves, one for each colour. I didn't take this far, but you could change the beat frequency and so on. . .
 *
 */


#include "FastLED.h"                                          // FastLED library. Preferably the latest copy of FastLED 2.1.

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

// Fixed definitions cannot change on the fly.
#define LED_DT 22                                             // Data pin to connect to the strip.
#define COLOR_ORDER GRB                                      // dont even think about it - it works ;) dont ask why
#define LED_TYPE WS2812B                                       // Using APA102, WS2812, WS2801. Don't forget to change LEDS.addLeds.
#define NUM_LEDS 96                                           // Number of LED's. - MUST MATCH THE NUMBER OF LEDS IN C++ CODE

// Initialize changeable global variables.
uint8_t max_bright = 100;                                      // Overall brightness definition. It can be changed on the fly.

CRGB leds[NUM_LEDS];

void setup() {

  Serial.begin(115200);                                        // Initialize serial port at max speed.
  delay(1000);                // Soft startup to ease the flow of electrons.
  
  FastLED.addLeds<LED_TYPE, LED_DT, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(max_bright);
  // leds[2].r = 10;
  // leds[2].g = 240;
  // leds[2].b = 150;
  
} // setup()



void loop () {
  
  if(Serial.readBytes((char*)leds, NUM_LEDS * 3) == NUM_LEDS * 3) {
    // leds[1].r = 0;
    // leds[1].g = 240;
    // leds[1].b = 0;
    FastLED.show();
  }else{
    // leds[0].r = 240;
    // leds[0].g = 0;
    // leds[0].b = 0;
    // FastLED.show();
  }
 
} // loop()



