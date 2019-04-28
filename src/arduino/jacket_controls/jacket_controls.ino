/**
 * @file   jacket_controls.ino
 * 
 * @brief  Receiver code for bluetooth commmunication on arduino
 * 
 * @author Michael You <myou@andrew.cmu.edu>
 */

#include <SoftwareSerial.h>
#include <FastLED.h>
#include "jacket_protocol.h"

#define SLV_L_PIN   10
#define SLV_R_PIN   9
#define BUZ_L_PIN   8
#define BUZ_R_PIN   7
#define LED_PIN     6
#define TX_PIN      5
#define RX_PIN      4
#define VIB_L_PIN   3
#define VIB_R_PIN   2
#define NUM_LEDS    300
/** @brief number of LEDs that wrap around the turns */
#define NUM_LEDS_SKIP 3
#define LED_BRIGHTNESS_BRAKE 30

#define TURN_R 255
#define TURN_G 191
#define TURN_B 0

#define HEIGHT 16
#define WIDTH  16

#define COMM_LENGTH 8

/*
 * TODO: do we want to do the snake direction change here? or in the code
 * For now, our images are symmetric so it doesn't matter
 */
const uint16_t
  right_turn_a[] = { 
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
    0B1000000000000000
  };

 const uint16_t
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

void show_matrix (uint16_t M[], int r, int g, int b) {
  int curr_led = 0;
  
  for (int col = 0; col < WIDTH; col++) {
    uint16_t curr_bitvec = M[col];
    uint16_t mask = 0B0000000000000001;
    
    for (int row = 0; row < HEIGHT; row++) {
      if (curr_bitvec & mask == 1) {
        if (curr_led < NUM_LEDS)
          leds[curr_led] = CRGB(r, g, b);
      }

      curr_led++;
      curr_bitvec = curr_bitvec >> 1;
    }

    // Skip LEDs that loop around at the corners
    for (int i = 0; i < NUM_LEDS_SKIP; i++) {
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
}

void vib_buzz (long command) {
  if (JKP_MASK_VIB_L & command)
    digitalWrite(VIB_L_PIN, HIGH);
  else
    digitalWrite(VIB_L_PIN, LOW);

  if (JKP_MASK_VIB_R & command)
    digitalWrite(VIB_R_PIN, HIGH);
  else
    digitalWrite(VIB_R_PIN, LOW);

  if (JKP_MASK_BUZZ_L & command)
    digitalWrite(BUZ_L_PIN, HIGH);
  else
    digitalWrite(BUZ_L_PIN, LOW);

  if (JKP_MASK_BUZZ_R & command)
    digitalWrite(BUZ_R_PIN, HIGH);
  else
    digitalWrite(BUZ_R_PIN, LOW);

  if (JKP_MASK_PROX_SL & command)
    digitalWrite(SLV_L_PIN, HIGH);
  else 
    digitalWrite(SLV_L_PIN, LOW);

  if (JKP_MASK_PROX_SR & command)
    digitalWrite(SLV_R_PIN, HIGH);
  else 
    digitalWrite(SLV_R_PIN, LOW);
}

/**
 * Selects the matrix display image based on the user settings
 */
void process_comm(long command) {
  clear_screen();

  if (JKP_MASK_BRAKE & command)
    on_screen(LED_BRIGHTNESS_BRAKE, 0, 0);

  if (JKP_MASK_TURNSIG_L & command) {
     show_matrix(left_turn, TURN_R, TURN_G, TURN_B);
  }

  if (JKP_MASK_TURNSIG_R & command) {
    show_matrix(right_turn_a, TURN_R, TURN_G, TURN_B);
  }

  // Vibrate the motors and buzz the piezos
  vib_buzz(command);

  // Show LEDs
  FastLED.show();  
}

SoftwareSerial S(RX_PIN, TX_PIN);

void setup() {
  // Sets the baud for serial data transmission      
  S.begin(38400);
  Serial.begin(38400);
  // S.setTimeout(0);

  // Sets digital pin 13 as output pin
  pinMode(VIB_L_PIN, OUTPUT);
  pinMode(VIB_R_PIN, OUTPUT);
  pinMode(BUZ_L_PIN, OUTPUT);
  pinMode(BUZ_R_PIN, OUTPUT);
  pinMode(SLV_L_PIN, OUTPUT);
  pinMode(SLV_R_PIN, OUTPUT);

  // LED Setup
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

  clear_screen();
  screen_on = false;
}

// Clear the buffer because of sync issues
void clear_buffer () {
  while (S.available() > 0) {
    S.read();
  }
}

void loop() {  
  long COMMANDS, MAGIC;
  char* read_magic = (char*)&MAGIC;
  char* read_into = (char*)&COMMANDS;

  if (S.available() >= COMM_LENGTH) {
    S.readBytes(read_magic, COMM_LENGTH/2);
    if (MAGIC != JKP_MAGIC) {
      clear_buffer();
      return;
    }
    
    S.readBytes(read_into, COMM_LENGTH/2);
    process_comm(COMMANDS);
  }
}
