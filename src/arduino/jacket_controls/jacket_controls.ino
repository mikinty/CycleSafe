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
#define NUM_LEDS    300
/** @brief number of LEDs that wrap around the turns */
#define NUM_LEDS_SKIP 3
#define LED_BRIGHTNESS 255

#define HEIGHT 16
#define WIDTH  16

/*
 * TODO: do we want to do the snake direction change here? or in the code
 * For now, our images are symmetric so it doesn't matter
 */
const uint16_t
  right_turn[] = { 
    0B0000000110000000,
    0B0000001001000000,
    0B0000010000100000,
    0B0000100000010000,
    0B0001000000001000,
    0B0010000000000100,
    0B0100000000000010,
    0B1000000000000001,
    0B0000000110000000,
    0B0000001001000000,
    0B0000010000100000,
    0B0000100000010000,
    0B0001000000001000,
    0B0010000000000100,
    0B0100000000000010,
    0B1000000000000001 
  };

CRGB leds[NUM_LEDS];
bool screen_on;

/**
 * TODO: give color options...for now it's going to be red
 */
void show_matrix (uint16_t M[]) {
  int curr_led = 0;
  
  for (int col = 0; col < WIDTH; col++) {
    Serial.print("CURR COL ");
    Serial.println(col);
    uint16_t curr_bitvec = M[col];
    uint16_t mask = 0B0000000000000001;
    
    for (int row = 0; row < HEIGHT; row++) {
      if (curr_bitvec & mask == 1) {
        leds[curr_led] = CRGB(LED_BRIGHTNESS, 0, 0);
        Serial.print(curr_led);
        Serial.print('|');
        Serial.println(1);
      } else {
        leds[curr_led] = CRGB(0, 0, 0);
        Serial.println(curr_led);
      }

      curr_led++;
      curr_bitvec = curr_bitvec >> 1;
      Serial.println(curr_bitvec);
    }

    for (int i = 0; i < NUM_LEDS_SKIP; i++) {
      if (curr_led < 300)
        leds[curr_led] = CRGB(0, 0, 0);
      curr_led++;
    }
  }
}

void display_signal (int type) {
  switch (type) {
    case 0:
      show_matrix(right_turn);
      break;
  }
  FastLED.show();
}

void on_screen() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(10, 0, 10);
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

  clear_screen();
  delay(1000);
  // on_screen();
  display_signal(0);
  delay(1000);

  /*
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
  */
}
