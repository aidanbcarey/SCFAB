
//Ring oscillator.  Jumper between pins 7 and 9.


void setup() {
 pinMode(7, INPUT);
 pinMode(9, OUTPUT);

}

void loop() {
  
  digitalWrite(9, !digitalRead(7));

}
