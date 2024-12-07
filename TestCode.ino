#include <IRremote.h>
#include <EEPROM.h>
#include <avr/wdt.h>  // Include for watchdog timer (for resetting the Arduino)


// Pin Definitions
#define IR_RECEIVE_PIN 7
#define LED_PIN 13

// Define pins for DRV8833
#define AIN1 A2 //IN1
#define AIN2 A3 //IN2
#define BIN1 A4 //IN3
#define BIN2 A5 //1N4
const int stepDelay = 500;          // Microseconds between steps
const int totalSteps = 1400;// Steps for to-and-fro motion

// LED and Control Pins
int lv_counter = 0;
const int redPin = 8;          // Pin connected to the red LED
//const int lowBatteryPin = 4;    // Define pin 4 for low battery indication
const int greenPin = 5;         // Pin connected to the green LED
const int orangePin = 6;        // Pin connected to the orange LED
const int controlPin = A1;       // Pin to check if functionalities should occur

bool isOrangeLedOn = false;     // Variable to store the orange LED state
bool isRedLedOn = false;        // Variable to store the red LED state
bool updateMode = false;        // Flag to indicate if we're in update mode
unsigned long updateStartTime = 0; // Time when update mode is entered
const unsigned long updateWaitTime = 2000; // 2 seconds wait time for new key
bool greenLedBlinking = false;  // Flag to control the green LED blinking
bool greenLedContinuous = false; // Flag for continuous green LED
bool motorLockPreviousState = false;
bool motorunLockPreviousState = true;

// EEPROM addresses for saving LED states
const int EEPROM_ORANGE_LED_STATE = 4; // Address to store orange LED state
const int EEPROM_GREEN_LED_STATE = 5;  // Address to store green LED state
const int EEPROM_RED_LED_STATE = 6;    // Address to store red LED state

// Global Variables
String decodedData = "";     // Current decoded data
String previousData = "";    // Last processed data
const String lockCode = "12345678";  // Lock code
const String unlockCode = "87654321"; // Unlock code
const String hearModeCode = "HEARMODE"; // Command to enter hear mode
const String resetModeCode = "RESETMOD"; // Command to enter reset mode
const String keyRequestCode = "KEY12345"; // Command to request stored key
const String clearModeCode = "CLREPROM"; // Command to request Clear EEPROM key
const String lockModeCode = "LOCKDEVI";  // Lock code
const String unlockModeCode = "UNLOCKDE";  // UnLock code
bool isUnlock = false;

bool inHearMode = false; // Track if we're in hear mode

String newCommand = "";  // To store new command in hear mode

// Function to save LED states to EEPROM
void saveLedStatesToEEPROM() {
    EEPROM.write(EEPROM_ORANGE_LED_STATE, isOrangeLedOn);
    EEPROM.write(EEPROM_GREEN_LED_STATE, greenLedContinuous);
    EEPROM.write(EEPROM_RED_LED_STATE, isRedLedOn); // Save the red LED state
}

// Function to clear EEPROM and reset LED states
void clearEEPROMAndReset() {
    motorLock(0);
    EEPROM.write(EEPROM_ORANGE_LED_STATE, 0);
    EEPROM.write(EEPROM_GREEN_LED_STATE, 0);
    EEPROM.write(EEPROM_RED_LED_STATE, 0); // Clear red LED state

    // Reset LED states
    isOrangeLedOn = false;
    greenLedContinuous = false;
    isRedLedOn = false; // Reset red LED state

    // Set LED pins to LOW
    digitalWrite(orangePin, LOW);
    digitalWrite(greenPin, LOW);
    digitalWrite(redPin, LOW); // Turn off red LED

    Serial.println("EEPROM cleared and LED states reset.");

    // Blink orange and green LEDs 3 times
    for (int i = 0; i < 3; i++) {
        digitalWrite(orangePin, HIGH);
        digitalWrite(greenPin, HIGH);
        delay(300);
        digitalWrite(orangePin, LOW);
        digitalWrite(greenPin, LOW);
        delay(300);
    }
}

// Function to blink the green LED when the key is updated
void blinkGreenLed(int times, int delayTime) {
    if (isOrangeLedOn) {
        digitalWrite(orangePin, LOW); // Turn off the orange LED
        isOrangeLedOn = false; // Reset orange LED state
        saveLedStatesToEEPROM(); // Save state
        Serial.println("Orange LED turned off as green LED starts blinking.");
    }

    for (int i = 0; i < times; i++) {
        digitalWrite(greenPin, HIGH); // Turn the green LED on
        delay(delayTime);             // Wait
        digitalWrite(greenPin, LOW);  // Turn the green LED off
        delay(delayTime);             // Wait
    }
}

// Function to blink all LEDs before resetting
void blinkAllLeds(int times, int delayTime) {
    for (int i = 0; i < times; i++) {
        digitalWrite(redPin, HIGH);
        digitalWrite(orangePin, HIGH);
        digitalWrite(greenPin, HIGH);
        delay(delayTime);  // Wait

        digitalWrite(redPin, LOW);
        digitalWrite(orangePin, LOW);
        digitalWrite(greenPin, LOW);
        delay(delayTime);  // Wait
    }
}

void lockDevice() {
  Serial.print("Locking ");
  // Move forward
  for (int i = 0; i < totalSteps; i++) {
    stepMotor(i % 4); // Perform the step based on the sequence
    delayMicroseconds(stepDelay); // Wait for the motor to stabilize
    //Serial.print("Locking ");
    //Serial.println(i);
  }
  stop(); // Stop the motor after moving
  delay(1000); // Pause at the end of backward motion
}


void unlockDevice() {

  Serial.print("UnLocking ");
  // Move backward
  for (int i = totalSteps; i > 0; i--) {
    stepMotor(i % 4);
    delayMicroseconds(stepDelay); // Wait for the motor to stabilize
    //Serial.print("UnLocking ");
    //Serial.println(i);
  }
  stop(); // Stop the motor after moving
  delay(1000); // Pause at the end of forward motion
}

// Stop function to disable all motor pins
void stop() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);
}
void motorLock(int state){
  
    Serial.print("state :");
    Serial.println(state);
    // Check if the state of lock has changed
    if (state != motorLockPreviousState) {
        if (state == true) {
            lockDevice();
        }
        // Update the previous state
        motorLockPreviousState = state;
    }
    // Check if the state of unlock has changed
    if (state != motorunLockPreviousState) {
        if (state == false) {
            unlockDevice();
        }
        // Update the previous state
        motorunLockPreviousState = state;
    }
 
}

// Function to control motor steps
void stepMotor(int step) {
 // Serial.println(step);
  switch (step) {
    case 0: // Coil 1 forward, Coil 2 off
      digitalWrite(AIN1, HIGH);
      digitalWrite(AIN2, LOW);
      digitalWrite(BIN1, HIGH);
      digitalWrite(BIN2, LOW);
      break;
    case 1: // Coil 1 off, Coil 2 forward
      digitalWrite(AIN1, LOW);
      digitalWrite(AIN2, HIGH);
      digitalWrite(BIN1, HIGH);
      digitalWrite(BIN2, LOW);
      break;
    case 2: // Coil 1 backward, Coil 2 off
      digitalWrite(AIN1, LOW);
      digitalWrite(AIN2, HIGH);
      digitalWrite(BIN1, LOW);
      digitalWrite(BIN2, HIGH);
      break;
    case 3: // Coil 1 off, Coil 2 backward
      digitalWrite(AIN1, HIGH);
      digitalWrite(AIN2, LOW);
      digitalWrite(BIN1, LOW);
      digitalWrite(BIN2, HIGH);
      break;
  }
}

// Function to initialize components
void setup() {
  Serial.begin(115200);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  IrReceiver.begin(IR_RECEIVE_PIN);  // Initialize IR receiver
  pinMode(LED_PIN, OUTPUT);          // Initialize LED pin as output
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT); // Initialize green LED pin
  pinMode(orangePin, OUTPUT); // Initialize orange LED pin
  pinMode(controlPin, INPUT); // Initialize control pin as input
  digitalWrite(redPin, LOW);  // Ensure the red LED is off initially

  // Restore LED states from EEPROM
  isOrangeLedOn = EEPROM.read(EEPROM_ORANGE_LED_STATE);
  greenLedContinuous = EEPROM.read(EEPROM_GREEN_LED_STATE);
  isRedLedOn = EEPROM.read(EEPROM_RED_LED_STATE); // Read red LED state

  // Set the LEDs to their previous states
  digitalWrite(orangePin, isOrangeLedOn ? HIGH : LOW);
  digitalWrite(greenPin, greenLedContinuous ? HIGH : LOW);
  digitalWrite(redPin, isRedLedOn ? HIGH : LOW); // Set red LED state
}

// Main loop
void loop() {

  String currentData = processIRData();  // Decode IR data

  float pinA1State = analogRead(controlPin);
  Serial.print("pin state: ");
  Serial.println(pinA1State);
  if (pinA1State < 1000) {
  lv_counter = lv_counter +1;
  }
  else{
    lv_counter = 0;
  } 
  if(lv_counter >5) {
    // Turn ON the red LED and turn off the other LEDs
    digitalWrite(greenPin, LOW);
    digitalWrite(orangePin, LOW);
    digitalWrite(redPin, HIGH); // Turn ON the red LED
    isOrangeLedOn = false;
    greenLedBlinking = false;
    greenLedContinuous = false;
    isRedLedOn = true; // Set the red LED state to ON
    saveLedStatesToEEPROM(); // Save the state

    if (currentData == clearModeCode){
      Serial.println("Clear EEPROM command received.");
      clearEEPROMAndReset(); // Clear EEPROM and reset LED states
    }

  }else{

    if(currentData == hearModeCode){
      inHearMode = true;
    }

    if (currentData == lockModeCode){
      greenLedBlinking = false;
      greenLedContinuous = true;
      motorLock(1);
      digitalWrite(greenPin, HIGH);
      digitalWrite(orangePin, LOW);
      isOrangeLedOn = false;
      saveLedStatesToEEPROM(); // Save state
      Serial.println("Green LED turned on continuously. Orange LED turned off."); 
    }

    
      if(currentData == resetModeCode){
        Serial.println("Reset signal received. Blinking all LEDs and resetting.");

        // Save the current LED states to EEPROM
        saveLedStatesToEEPROM();
        blinkAllLeds(3, 300);
        wdt_enable(WDTO_15MS);
        while (1);
      }

      if (currentData == clearModeCode){
      Serial.println("Clear EEPROM command received.");
      clearEEPROMAndReset(); // Clear EEPROM and reset LED states
      }


    if(inHearMode){
      // Ensure the command is exactly 8 characters long
      if (currentData.length() == 8) { 
        greenLedBlinking = false;
        greenLedContinuous = true;
        motorLock(1);
        digitalWrite(greenPin, HIGH);
        digitalWrite(orangePin, LOW);
        isOrangeLedOn = false;
        saveLedStatesToEEPROM(); // Save state
        Serial.println("Green LED turned on continuously. Orange LED turned off."); 
        newCommand = currentData;      
        storeKeyInEEPROM(newCommand);      
        Serial.print("Stored Command: ");       
        Serial.println(newCommand);       
        inHearMode = false; // Exit hear mode after storing    
      } else {
        Serial.println("Command must be exactly 8 characters long. Try again."); 
      }

      if (currentData == unlockModeCode){
        isUnlock = true;
      }

      if (isUnlock){
        String storedKey = retrieveKeyFromEEPROM();
        if (currentData == storedKey){
          isOrangeLedOn = true;
          digitalWrite(orangePin, HIGH);
          digitalWrite(greenPin, LOW);
          greenLedBlinking = false;
          greenLedContinuous = false;
          isUnlock = false;
          motorLock(0);
          saveLedStatesToEEPROM(); // Save state
          Serial.println("Orange LED turned on. Green LED turned off.");
        }
        else {
          Serial.println("Incorrect Password");
        }
      }
    }
  }

  if (greenLedBlinking) {
    digitalWrite(greenPin, HIGH);
    delay(200);
    digitalWrite(greenPin, LOW);
    delay(200);
    }

  if (greenLedContinuous) {
      digitalWrite(greenPin, HIGH);
    }





  
  // Process the command if valid data is received
  /*if (currentData != "") {
    Serial.print("Decoded Data: ");
    Serial.println(currentData);
    processCommand(currentData);
  }*/
}

// Function to decode raw IR data into a human-readable string
String decodeIRDataToString(uint32_t* rawData, uint8_t numData) {
  String decodedString = "";
  for (int8_t i = numData - 1; i >= 0; --i) {
    char result[5];
    result[0] = (rawData[i] >> 24) & 0xFF;  // Extract byte 1
    result[1] = (rawData[i] >> 16) & 0xFF;  // Extract byte 2
    result[2] = (rawData[i] >> 8) & 0xFF;   // Extract byte 3
    result[3] = rawData[i] & 0xFF;          // Extract byte 4
    result[4] = '\0';  // Null-terminate the string
    decodedString += String(result);
  }
  return decodedString;
}

// Function to process IR data and return decoded string
String processIRData() {
  if (IrReceiver.decode()) {
    if (IrReceiver.decodedIRData.numberOfBits > 1) {
      uint8_t numData = (IrReceiver.decodedIRData.numberOfBits - 1) / 32 + 1;
      String currentData = decodeIRDataToString(IrReceiver.decodedIRData.decodedRawDataArray, numData);

      // Check if the current data is the same as the previous data
      if (currentData == previousData) {
        currentData = "";  // Ignore if it's the same
      } else {
        previousData = currentData;  // Update previous data
      }

      IrReceiver.resume();
      return currentData;
    }
    IrReceiver.resume();
  }
  return "";
}

// Function to process the decoded command
void processCommand(const String& command) {
  if (command == lockCode) {
    digitalWrite(LED_PIN, HIGH);  // Turn on LED
    Serial.println("LED ON: Locked");
  } else if (command == unlockCode) {
    digitalWrite(LED_PIN, LOW);  // Turn off LED
    Serial.println("LED OFF: Unlocked");
  } else if (command == hearModeCode) {
    // Enter hear mode to store a new command
    Serial.println("Enter a new 8-character command (max length 8):");
    inHearMode = true;
  } else if (command == keyRequestCode) {
    // Retrieve the stored command (key) from EEPROM
    String storedKey = retrieveKeyFromEEPROM();
    if (storedKey != "") {
      Serial.print("Stored Key: ");
      Serial.println(storedKey);
    } else {
      Serial.println("No key stored yet.");
    }
  } else if (inHearMode) {
    // Ensure the command is 8 characters long
    if (command.length() == 8) {
      newCommand = command;
      storeKeyInEEPROM(newCommand);
      Serial.print("Stored Command: ");
      Serial.println(newCommand);
      inHearMode = false; // Exit hear mode after storing
    } else {
      Serial.println("Password must be exactly 8 characters long. Try again.");
    }
  } else if (command == "") {
    Serial.println("Unknown Code: No Action Taken");
  }
}

// Function to store the Key in EEPROM
void storeKeyInEEPROM(const String& command) {
  // Store the command in EEPROM (8 characters)
  for (int i = 0; i < 8; i++) {
    EEPROM.write(i, command[i]);
  }
  // Ensure the EEPROM data is written
  //EEPROM.commit();
}

// Function to retrieve the stored Key from EEPROM
String retrieveKeyFromEEPROM() {
  String storedCommand = "";
  for (int i = 0; i < 8; i++) {
    char c = EEPROM.read(i);
    storedCommand += c;
  }
  // Return the stored command if it's not empty
  return (storedCommand.length() == 8) ? storedCommand : "";
}
