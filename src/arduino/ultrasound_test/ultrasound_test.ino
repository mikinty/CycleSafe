
const int PIN_UTS1_TRIGGER = 8;
const int PIN_UTS1_ECHO = 9;

void setup()
{
  pinMode(PIN_UTS1_TRIGGER, OUTPUT);
  pinMode(PIN_UTS1_ECHO, INPUT);
  Serial.begin(9600);
}

int getDistance() {
  int dt;
  digitalWrite(PIN_UTS1_TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_UTS1_TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_UTS1_TRIGGER, LOW);
  
  // Maximum distance 200 cm. Speed of sound 334 00 cm/s
  // Timeout after 2 * 200/33400 s = 11976 us
  dt = pulseIn(PIN_UTS1_ECHO, HIGH);

  return dt / 60;
  
}

void loop() {
  int dist = getDistance();
  Serial.println(dist);
  delay(20);
}
