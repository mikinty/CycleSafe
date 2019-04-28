
#include "speed_protocol.h"

const int pin_input = 2;

const int samples = 16;
const int pos_diff = 4;

const int pina_5v = A5;
const int pina_accel = A4;
const int pina_gnd = A1;

// TODO make this customizable
long wheel_circumference_um_by_4 = 7816000; //um

int init_ready;

int pos = samples - 1;
int timings[samples];
long bike_speed = 0;

void makePass() {
  int curr_time = millis();
  if (curr_time - timings[pos] < 200) return;
  if (pos == 0) pos = samples - 1;
  else pos--;
  timings[pos] = curr_time;
}

void setup() {
  pinMode(pin_input, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pin_input), makePass, FALLING);
  Serial.begin(SPEED_BAUD_RATE);
  for (int i = 0; i < samples; i++) {
    timings[i] = 0;
  }

  bike_speed = 0;

  pinMode(pina_5v, OUTPUT);
  pinMode(pina_gnd, OUTPUT);
  pinMode(pina_accel, INPUT);

  digitalWrite(pina_gnd, LOW);
  digitalWrite(pina_5v, HIGH);

  init_ready = 0;
}

void loop() {

  int c;

  if (Serial.available() > 0) {
    c = Serial.read();

    switch(c) {
      case SPEED_REQ_SPEED: {
        int curr_pos = pos;

        // check that the latest timing is recent
        int curr_time = millis();
        if (curr_time - timings[curr_pos] > 2000) {
          // If no rotation for too long, reset to 0.
          // The current setting is 2000 ms.
          bike_speed = 0;
        }
        else {
          int prev_pos = curr_pos + pos_diff;
          if (prev_pos >= samples) prev_pos -= samples;
          long dt = timings[curr_pos] - timings[prev_pos];
          if (dt == 0) bike_speed = 0;
          else bike_speed = wheel_circumference_um_by_4 / dt;
        }

        // TODO transmit only the latest value to the RPi
        // Serial.println(bike_speed);
        Serial.write((char *)&bike_speed, 4);

        break;
      }

      case SPEED_REQ_ACCEL: {

        long accel;
        accel = analogRead(pina_accel);
        //Serial.println(accel);
        Serial.write((char *)&accel, 4);
        
      }

      case SPEED_REQ_INIT_MAGIC: {
        init_ready = 1;
        break;
      }
    }
  }

  if (!init_ready) {
    for (int i = 0; i < 4; i++) {
      Serial.write(SPEED_REQ_INIT_MAGIC);
    }
  }


}
