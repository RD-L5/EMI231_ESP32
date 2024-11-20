#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define RXD2 16
#define TXD2 17

// WiFi credentials
const char* ssid = "Baliteam";
const char* password = "baliteam88";

const char* serverUrl = "http://192.168.0.13:80/data";

byte request[] = {0x01, 0x03, 0x03, 0xF2, 0x00, 0x06, 0x64, 0x7F};
byte response[20];
int responseIndex = 0;

float parseFloat32(byte* bytes);

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
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
      
      // Round to 1 decimal place
      voltageA = round(voltageA * 10.0) / 10.0;
      voltageB = round(voltageB * 10.0) / 10.0;
      voltageC = round(voltageC * 10.0) / 10.0;

      // Create JSON document
      StaticJsonDocument<200> doc;
      doc["deviceAddress"] = deviceAddress;
      doc["functionCode"] = functionCode;
      doc["byteLength"] = byteLength;
      doc["voltageA"] = String(voltageA, 1); // Format dengan 1 desimal
      doc["voltageB"] = String(voltageB, 1); // Format dengan 1 desimal
      doc["voltageC"] = String(voltageC, 1); // Format dengan 1 desimal

      // Convert JSON to string
      String jsonString;
      serializeJson(doc, jsonString);

      // Send data to server if WiFi is connected
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/json");
        
        int httpResponseCode = http.POST(jsonString);
        
        if (httpResponseCode > 0) {
          String response = http.getString();
          Serial.println("HTTP Response code: " + String(httpResponseCode));
          Serial.println("Response: " + response);
        } else {
          Serial.println("Error on sending POST: " + String(httpResponseCode));
        }
        
        http.end();
      }

      // Print values to Serial for debugging
      Serial.print("Device Address: ");
      Serial.println(deviceAddress);
      Serial.print("Function Code: ");
      Serial.println(functionCode);
      Serial.print("Byte Length: ");
      Serial.println(byteLength);
      Serial.print("Phase A Voltage: ");
      Serial.print(String(voltageA, 1));
      Serial.println("V");
      Serial.print("Phase B Voltage: ");
      Serial.print(String(voltageB, 1));
      Serial.println("V");
      Serial.print("Phase C Voltage: ");
      Serial.print(String(voltageC, 1));
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