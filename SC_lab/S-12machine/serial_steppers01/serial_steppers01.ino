#include <Stepper.h>

const int stepsPerRevolution = 200;  //200 for our steppers.
int rec_int[100];     //byte array of numbers to be parsed from serial string.  


// initialize the stepper library for two steppers:
Stepper Stepper1(stepsPerRevolution, 11, 10, 9, 8);
Stepper Stepper2(stepsPerRevolution, 7, 6, 5, 4);


void setup() {
    Serial.begin(0);              //This is the USB serial.
    Serial.setTimeout(20);        //Set wait time for serial data to 20ms (default 1 sec).
        while (!Serial) {             //Wait until Serial wakes up.
    ; 
    }

    Serial.println("ready");
}

void loop() {

  int ind = get_data();           //calls the function that collects serial data.
  step_two(rec_int[0],1,1);
  step_two(rec_int[0],1,-1);
  step_two(rec_int[0],-1,-1);
  step_two(rec_int[0],-1,1);
  
  
// for (int i=0; i<ind; i++){
//  Serial.print(rec_int[i]);
//  Serial.print(", ");
//  }
//    
// Serial.print(ind);
// Serial.println(" integers received");          //how many bytes sent.  
}
