#include<Servo.h>

#define servoPin 5

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

Servo servo;

void setup() {
  // reserve 200 bytes for the inputString:
  servo.attach(servoPin);
  servo.writeMicroseconds(1500);
  Serial.begin(9600);
  Serial.println("Hello");
  inputString.reserve(200);
  
}

void loop() {
  // print the string when a newline arrives:
  if (stringComplete) {
    Serial.println("Recieved");
    Serial.println(inputString); 
    // clear the string:
    Serial.println(inputString.toInt());
    servo.writeMicroseconds(inputString.toInt());
    inputString = "";
    stringComplete = false;
  }
}

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    } 
  }
}

