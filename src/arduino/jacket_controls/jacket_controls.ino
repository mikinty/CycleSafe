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
#define LED_BRIGHTNESS_BRAKE 30

#define TURN_R 255
#define TURN_G 191
#define TURN_B 0

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
  }, 
  left_turn[] = { 
    0B1000000000000001, 
    0B0100000000000010,
    0B0010000000000100,
    0B0001000000001000,
    0B0000100000010000,
    0B0000010000100000,
    0B0000001001000000,
    0B0000000110000000,
    0B1000000000000001, 
    0B0100000000000010,
    0B0010000000000100,
    0B0001000000001000,
    0B0000100000010000,
    0B0000010000100000,
    0B0000001001000000,
    0B0000000110000000
  };

CRGB leds[NUM_LEDS];
bool screen_on;

/**
 * TODO: give color options...for now it's going to be red
 */
void show_matrix (uint16_t M[], int r, int g, int b) {
  int curr_led = 0;
  
  for (int col = 0; col < WIDTH; col++) {
    uint16_t curr_bitvec = M[col];
    uint16_t mask = 0B0000000000000001;
    
    for (int row = 0; row < HEIGHT; row++) {
      if (curr_bitvec & mask == 1) {
        leds[curr_led] = CRGB(r, g, b);
      } else {
        leds[curr_led] = CRGB(0, 0, 0);
      }

      curr_led++;
      curr_bitvec = curr_bitvec >> 1;
    }

    // Skip LEDs that loop around at the corners
    for (int i = 0; i < NUM_LEDS_SKIP; i++) {
      if (curr_led < 300)
        leds[curr_led] = CRGB(0, 0, 0);
      curr_led++;
    }
  }
}

void on_screen(int r, int g, int b) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(r, g, b);
  }
}


void clear_screen() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
}

/**
 * Selects the matrix display image based on the user settings
 */
void display_signal (int type) {
  switch (type) {
    case 0:
      show_matrix(right_turn, TURN_R, TURN_G, TURN_B);
      break;

    case 1:
      show_matrix(left_turn, TURN_R, TURN_G, TURN_B);
      break;

    case 2:
      on_screen(LED_BRIGHTNESS_BRAKE, 0, 0);
      break;

    // HAZARD

    // BRAKE AND TURN AT THE SAME TIME
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

  for (int i = 0; i < 3; i++) {
    clear_screen();
    delay(1000);
    display_signal(i);
    delay(1000);  
  }

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
