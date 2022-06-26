void setup() {
  pinMode(13, OUTPUT);     
  pinMode(9, INPUT);   
}
 
void loop() {
  if (digitalRead(9) == HIGH)  {
    digitalWrite(13, HIGH); // sets the digital pin 13 on
    delay(1000);            // waits for a second
    digitalWrite(13, LOW);  // sets the digital pin 13 off
    delay(1000); 
    // Cam was here
  }
}
