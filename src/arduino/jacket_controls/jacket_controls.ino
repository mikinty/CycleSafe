/**
 * @file   jacket_controls.ino
 * 
 * @brief  Receiver code for bluetooth commmunication on arduino
 * 
 * @author Michael You <myou@andrew.cmu.edu>
 */

#include <FastLED.h>

#define LED_STATUS  13
#define LED_PIN     6
#define NUM_LEDS    256

#define HEIGHT 16
#define WIDTH  16

CRGB leds[NUM_LEDS];
bool screen_on;

void on_screen() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
}


void clear_screen() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
}

void setup() {
  // Sets the baud for serial data transmission      
  Serial.begin(9600);                            

  // Sets digital pin 13 as output pin
  pinMode(LED_STATUS, OUTPUT);  

  // LED Setup
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

  clear_screen();
  screen_on = false;
}

void loop() {  
  int data;
  
  if(Serial.available() > 0) {
    data = Serial.read();
    Serial.print(data);
    Serial.print("\n");        
    if(data == '1') {
      digitalWrite(LED_STATUS, HIGH);
        if (!screen_on) {
          on_screen(); 
        }
    } else {
      digitalWrite(LED_STATUS, LOW);
      if (screen_on) {
          clear_screen(); 
      }
    }
  }
}
