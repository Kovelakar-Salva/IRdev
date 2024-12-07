#include <IRremote.h>

#define IR_RECEIVE_PIN 7  // IR receiver pin
String hex = "";      
String hexData = "";      // String to store the hexadecimal data
String decodedData = "";  // String to store the decoded characters

void setup() {
  Serial.begin(115200);  
  pinMode(5,OUTPUT);            // Start serial communication
  IrReceiver.begin(IR_RECEIVE_PIN);  // Initialize IR receiver on specified pin
}

void loop() {
  if (IrReceiver.decode()) {  // If an IR signal is decoded
  if(IrReceiver.decodedIRData.numberOfBits >1){
    uint_fast8_t tNumberOfArrayData = 0;

    // Determine the number of data arrays
    tNumberOfArrayData = ((IrReceiver.decodedIRData.numberOfBits - 1) / 32) + 1;

    Serial.print(F("0x"));

    for (uint_fast8_t i = 0; i < tNumberOfArrayData; ++i) {
      // Print each raw data element in hexadecimal
      Serial.print(IrReceiver.decodedIRData.decodedRawDataArray[i], HEX);

      if (i != tNumberOfArrayData - 1) {
      }
    }

    Serial.println(F(""));
    Serial.print(F("Decoded String: "));

    // Call the function to convert the raw hex data to characters
    decodedData = decodeIRDataToString(IrReceiver.decodedIRData.decodedRawDataArray, tNumberOfArrayData);
    hexCode(decodedData);
    // Print the decoded string
    Serial.println(decodedData);
  }
  }
  // Ready to decode the next IR signal
  IrReceiver.resume();
}
// Function to decode raw IR data to a human-readable string
String decodeIRDataToString(uint32_t* rawData, uint_fast8_t numData) {
  String hexString = "";

  // Loop through the raw data array in reverse order
  for (int_fast8_t i = numData - 1; i >= 0; --i) {
    char result[5];  // Buffer for hex representation, 4 characters + null terminator

    // Extract each byte from the 32-bit integer (rawData[i])
    result[0] = (rawData[i] >> 24) & 0xFF;  // Extract the first byte
    result[1] = (rawData[i] >> 16) & 0xFF;  // Extract the second byte
    result[2] = (rawData[i] >> 8) & 0xFF;   // Extract the third byte
    result[3] = rawData[i] & 0xFF;          // Extract the fourth byte
    result[4] = '\0';  // Null-terminate the string

    hexString += String(result);  // Append to hex string
  }

  return hexString;
}

// Function to get 8 char
void hexCode(String hex) {

String hexlock = "12345678";
String hexunlock = "87654321";

if (hexlock == hex){
  digitalWrite(5,HIGH);
  Serial.println("led on");
}

if (hexunlock == hex) {
  digitalWrite(5,LOW);
  Serial.println("led off");
}

}
