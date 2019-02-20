/*
* Bluetooth Basic Setup
*/
char data = 0;            //Variable for storing received data

#define LED_PIN 7

void setup()
{
    Serial.begin(9600);   //Sets the baud for serial data transmission    
    pinMode(LED_PIN, OUTPUT);                      
}
void loop()
{
   if(Serial.available() > 0)      // Send data only when you receive data:
   {
      data = Serial.read();        //Read the incoming data & store into data
      Serial.print(data);          //Print Value inside data in Serial monitor

      if (data == '1') {
        digitalWrite(LED_PIN, HIGH);
      } else {
        digitalWrite(LED_PIN, LOW);
      }
   }
}
