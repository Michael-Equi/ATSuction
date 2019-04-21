#include<Servo.h>
#include <FlashStorage.h>

//Board manager URL: https://adafruit.github.io/arduino-board-index/package_adafruit_index.json
// for use with Adafruit Feather M0

//Emulated EEPROM library is from here: https://github.com/cmaglie/FlashStorage
//Download from library manager as FlashStorage

#define servoPin 5
#define relayPin 10
#define switchPin A0 //pull up and look for 0 on button press

Servo servo;

//Serial variables
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

//Runtime variables
bool switchVal = true; //Store the value of the last switch value
bool switchPressed = false; //Used to determine wether to analyze button information (duration, sequence, etc.)
bool setSuctionPoseMode = false;
bool setRestPoseMode = false;
int numPresses = 0;
int currentPosition = 1500; //1500 is just for instantiation
unsigned long buttonDuration = 0; //Holder variable for computing the lastButtonDuration
unsigned long lastButtonDuration = 0; //The duration that the button was held at in the last press sequence

//User configurable variables
int restPosition = 1900; //1100 <= restPosition <= 1900
int suctionPosition = 1100; //The position where the suction hits the mouth
FlashStorage(restPosition_storage, int);
FlashStorage(suctionPosition_storage, int);

void setup() {
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);

  //Adafruit Feather M0 Serial goes straight to USB
  Serial.begin(115200);

  //Setup the IO pins
  pinMode(switchPin, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);
  servo.attach(servoPin);

  //Initialize stored variables
  restPosition = restPosition_storage.read();
  suctionPosition = suctionPosition_storage.read();
  //Make sure vars are valid
  if(!(restPosition > 1500 && restPosition < 1900)){
    restPosition = 1800;
    restPosition_storage.write(restPosition);
    }
   if(!(suctionPosition > 1100 && suctionPosition < 1500)){
    suctionPosition = 1200;
    suctionPosition_storage.write(suctionPosition);
    }
    
  servo.writeMicroseconds(restPosition); //Set servo to initial rest position
  currentPosition = restPosition;
}

void loop() {
  //Check serial input buffer for avialable data
  if(Serial.available() > 0){
    serialEvent();
  }
    
  // print the string when a newline arrives:
  if (stringComplete) {

    //Handle input
    Serial.println("Recieved: " + inputString);
    //Serial.println(inputString); 
    //Serial.println(inputString.toInt());
    int setting = inputString.toInt();
    servo.writeMicroseconds(setting);

    // clear the string:
    inputString = "";
    stringComplete = false;
  }

  //Button logic
  if((switchVal ^ digitalRead(switchPin))){ //Look for a change in the button state
    
    delay(5); //debounce
    bool oldSwitchVal = switchVal;
    switchVal = digitalRead(switchPin);
    
    if(oldSwitchVal){ //Look for falling edge
        buttonDuration = millis();
//        Serial.print("ButtonPressed: ");
//        numPresses++;
//        Serial.println(numPresses);
      } else {
        lastButtonDuration = millis() - buttonDuration;
        //Serial.println(lastButtonDuration);
        switchPressed = true;
        }
    }

    //Determine action to take
    if(switchPressed && lastButtonDuration > 250 && lastButtonDuration < 1000 && (!(setSuctionPoseMode || setRestPoseMode))){ //normal suction if not in selected mode or outside click range
      moveArmature(suctionPosition, 3);
      digitalWrite(relayPin, HIGH);
      delay(3000);
      digitalWrite(relayPin, LOW);
      moveArmature(restPosition, 3);
      switchPressed = false;
      } 
      else if(switchPressed && lastButtonDuration > 1000 && lastButtonDuration < 3000&& (!(setSuctionPoseMode || setRestPoseMode))){
        Serial.println("Set new suction position");
        setSuctionPoseMode = true;
        moveArmature(restPosition + 50, 25);
        moveArmature(restPosition - 50, 25);
        moveArmature(1500, 25);
        switchPressed = false;
      }
      else if(switchPressed && lastButtonDuration > 3000 && lastButtonDuration < 6000&& (!(setSuctionPoseMode || setRestPoseMode))){
        Serial.println("Set new rest position");
        setRestPoseMode = true;
        moveArmature(restPosition + 50, 25);
        moveArmature(restPosition - 50, 25);
        moveArmature(restPosition + 50, 25);
        moveArmature(restPosition - 50, 25);
        moveArmature(1500, 25);
        switchPressed = false;
      }

      //Handle special modes
      if(setSuctionPoseMode){ //Suction position set mode
        if(switchPressed){
           suctionPosition = currentPosition;
           //suctionPosition_storage.write(suctionPosition); //Store in nonvolitile memory
           moveArmature(restPosition, 5);
           setSuctionPoseMode = false;
          }
        else{
          moveArmature(currentPosition-1, 1); //Suction position is the smaller number (1100-1500)
          }
        }
      else if(setRestPoseMode){ //Rest position set mode
        if(switchPressed){
           restPosition = currentPosition;
           //restPosition_storage.write(restPosition); //Store in nonvolitile memory
           moveArmature(restPosition, 5);
           setRestPoseMode = false;
          }
        else{
          moveArmature(currentPosition+1, 1); //Rest position is the bigger number (1500-1900)
          }
        }
}

//Move the servo from current position to the stop point with speed armSpeed in micro-second pulses per 2 milliseconds
//Arm speed is relative based on the servo so it can be set by the user
void moveArmature(int stopPosition, int armSpeed){
  int startingPosition = currentPosition;
  for(int i = 0; i < abs(stopPosition - startingPosition)/armSpeed; i++){
    int newPosition = currentPosition + (armSpeed*(abs(stopPosition - currentPosition)/(stopPosition - currentPosition))); //No divide by zero due to for loop condition
    servo.writeMicroseconds(newPosition);
    delay(15);
    currentPosition = newPosition;
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
