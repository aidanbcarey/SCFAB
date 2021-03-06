/*
 * Code to parse serial data into integers.  Send characters repsesnting numbers.
 * Serial.parseInt() finds combinations that make sense as integers, ignoring other characters.
 * Can receive many integers in one load, thanks to serial buffer on microcontroller board.
 * 
 */




int rec_int;     //byte array of numbers parsed from serial string.  Received on Serial.

void setup() {
    Serial.begin(0);              //This is the USB serial.
    Serial.setTimeout(20);        //Set wait time for serial data to 20ms (default 1 sec).
        while (!Serial) {             //Wait until Serial wakes up.
    ; 
    }

    Serial.println("ready");
}

void loop() {
    int counter=0;   // count integers received.  
    
    while (!Serial.available() ){             //Loop and do nothing until there is a serial character received.
      ;
    }
    
    while(Serial.available()){                //Receive bytes and parse them into integers.
       rec_int = Serial.parseInt();
       counter++;                             //increment counter    
       Serial.print(rec_int);
       Serial.print(", ");
   }
   
   Serial.print("  "); 
   Serial.print(counter);
   Serial.println(" integers received");          //how many bytes sent.
   
}
