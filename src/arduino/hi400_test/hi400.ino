
int val1;
int val2;

void setup()
{
  Serial.begin(9600);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
}

void loop()
{
  val1 = analogRead(A1);
  Serial.print("1 ");
  Serial.println(val1);
  val2 = analogRead(A2);
  Serial.print("2 "); 
  Serial.println(val2);
  delay(200);
}
