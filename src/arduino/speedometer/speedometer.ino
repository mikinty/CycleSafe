
const int pin_input = 2;
const int samples = 16;
const int pos_diff = 4;

// TODO make this customizable
float wheel_circumference_by_4 = 7816.283; //mm

int pos = samples - 1;
int timings[samples];
float bike_speed = 0.0;

void makePass() {
  int curr_time = millis();
  if (curr_time - timings[pos] < 200) return;
  if (pos == 0) pos = samples - 1;
  else pos--;
  timings[pos] = curr_time;
}

void setup() {
  pinMode(pin_input, INPUT);
  attachInterrupt(digitalPinToInterrupt(pin_input), makePass, RISING);  Serial.begin(9600);
  for (int i = 0; i < samples; i++) {
    timings[i] = 0;
  }
  
}

void loop() {
  
  int curr_pos = pos;

  // check that the latest timing is recent
  int curr_time = millis();
  if (curr_time - timings[curr_pos] > 2000) {
    // If no rotation for too long, reset to 0.
    // The current setting is 2000 ms.
    bike_speed = 0.0;
  }
  else {
    int prev_pos = curr_pos + pos_diff;
    if (prev_pos >= samples) prev_pos -= samples;
    int dt = timings[curr_pos] - timings[prev_pos];
    bike_speed = wheel_circumference_by_4 / (float) dt;
  }
  
  // TODO transmit only the latest value to the RPi
  Serial.println(bike_speed);
  delay(1000);
}
