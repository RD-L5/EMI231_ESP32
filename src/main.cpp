#include <Arduino.h>
#define RXD2 16
#define TXD2 17
byte request[] = {0x01, 0x03, 0x03, 0xF2, 0x00, 0x06, 0x64, 0x7F};
byte response[20];
int responseIndex = 0;
float parseFloat32(byte* bytes);
void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
}

void loop() {
  Serial2.write(request, sizeof(request));
  delay(500);
  
  responseIndex = 0;
  while (Serial2.available()) {
    response[responseIndex] = Serial2.read();
    responseIndex++;
  }
  
  if (responseIndex > 0) {
    byte deviceAddress = response[0];
    byte functionCode = response[1];
    byte byteLength = response[2];  

    if (responseIndex >= 15) { 
      float voltageA = parseFloat32(&response[3]);
      float voltageB = parseFloat32(&response[7]);
      float voltageC = parseFloat32(&response[11]);
      
      Serial.print("Device Address: ");
      Serial.println(deviceAddress);
      Serial.print("Function Code: ");
      Serial.println(functionCode);
      Serial.print("Byte Length: ");
      Serial.println(byteLength);
      Serial.print("Phase A Voltage: ");
      Serial.print(voltageA, 1);
      Serial.println("V");
      Serial.print("Phase B Voltage: ");
      Serial.print(voltageB, 1);
      Serial.println("V");
      Serial.print("Phase C Voltage: ");
      Serial.print(voltageC, 1);
      Serial.println("V");
    }
    Serial.println("------------------------");
  }
  
  delay(5000);
}


float parseFloat32(byte* bytes) {
  union {
    byte array[4];
    float value;
  } converter;
  
  converter.array[3] = bytes[0];
  converter.array[2] = bytes[1];
  converter.array[1] = bytes[2];
  converter.array[0] = bytes[3];
  
  return converter.value;
}