#include <IRremote.h>
#include <EEPROM.h>

// Pin Definitions
#define IR_RECEIVE_PIN 7
#define LED_PIN 5

// Global Variables
String decodedData = "";     // Current decoded data
String previousData = "";    // Last processed data
const String lockCode = "12345678";  // Lock code
const String unlockCode = "87654321"; // Unlock code
const String hearModeCode = "HEARMODE"; // Command to enter hear mode
const String keyRequestCode = "KEY12345"; // Command to request stored key
bool inHearMode = false; // Track if we're in hear mode
String newCommand = "";  // To store new command in hear mode

// Function to initialize components
void setup() {
  Serial.begin(115200);
  IrReceiver.begin(IR_RECEIVE_PIN);  // Initialize IR receiver
  pinMode(LED_PIN, OUTPUT);          // Initialize LED pin as output
}

// Main loop
void loop() {
  String currentData = processIRData();  // Decode IR data

  // Process the command if valid data is received
  if (currentData != "") {
    Serial.print("Decoded Data: ");
    Serial.println(currentData);
    processCommand(currentData);
  }
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
