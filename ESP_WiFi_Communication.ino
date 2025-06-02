/*
 * ESP WiFi Communication Module
 * Version: 1.1
 * Author: Amir Saleh
 * Description: Handles WiFi connection and data transmission using ESP module
 * Repository: [GitHub Repository URL]
 */

#include <Arduino.h>

// Define your network credentials
const char* ssid = "YourSSID";
const char* password = "YourPassword";

// Server configuration
const char* server = "192.168.1.100";
const int port = 80;

// Structure for sensor data
struct SensorData {
  int gas;
  int temp;
  int sensor3;
};

// Timeout settings
const unsigned long AT_TIMEOUT = 5000; // 5 seconds timeout for AT commands
const unsigned long WIFI_CONNECT_TIMEOUT = 30000; // 30 seconds for WiFi connection

void sendATCommand(const char* cmd, const char* expectedResponse = "OK", unsigned long timeout = AT_TIMEOUT) {
  Serial.print("Sending: ");
  Serial.println(cmd);
  Serial1.println(cmd);
  
  unsigned long startTime = millis();
  String response = "";
  
  while (millis() - startTime < timeout) {
    if (Serial1.available()) {
      char c = Serial1.read();
      response += c;
      Serial.write(c); // Echo to serial monitor
      
      // Check if expected response is found
      if (response.indexOf(expectedResponse) != -1) {
        return;
      }
    }
  }
  
  Serial.println("Error: Timeout waiting for response");
}

bool setupWiFi() {
  Serial.println("Initializing ESP module...");
  
  // Reset module (optional)
  // sendATCommand("AT+RST");
  // delay(1000);
  
  // Check if module is responsive
  sendATCommand("AT", "OK");
  
  // Set WiFi mode to station (client)
  sendATCommand("AT+CWMODE=1", "OK");
  
  // Connect to WiFi network
  char connectCmd[128];
  snprintf(connectCmd, sizeof(connectCmd), "AT+CWJAP=\"%s\",\"%s\"", ssid, password);
  sendATCommand(connectCmd, "OK", WIFI_CONNECT_TIMEOUT);
  
  // Get IP address (for debugging)
  sendATCommand("AT+CIFSR", "OK");
  
  return true;
}

bool sendDataToServer(const SensorData &data) {
  // Start TCP connection
  char tcpCmd[128];
  snprintf(tcpCmd, sizeof(tcpCmd), "AT+CIPSTART=\"TCP\",\"%s\",%d", server, port);
  sendATCommand(tcpCmd, "OK");
  
  // Prepare HTTP GET request
  char payload[128];
  snprintf(payload, sizeof(payload), 
           "GET /log?gas=%d&temp=%d&s3=%d HTTP/1.1\r\nHost: %s\r\n\r\n", 
           data.gas, data.temp, data.sensor3, server);
  
  // Send payload length
  char sendCmd[32];
  snprintf(sendCmd, sizeof(sendCmd), "AT+CIPSEND=%d", strlen(payload));
  sendATCommand(sendCmd, ">");
  
  // Send actual payload
  Serial1.print(payload);
  
  // Wait for response
  unsigned long startTime = millis();
  String response = "";
  while (millis() - startTime < AT_TIMEOUT) {
    if (Serial1.available()) {
      char c = Serial1.read();
      response += c;
      Serial.write(c);
      
      if (response.indexOf("SEND OK") != -1) {
        return true;
      }
    }
  }
  
  Serial.println("Error: Data transmission failed");
  return false;
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial1.begin(115200); // Default baud rate for ESP modules
  
  delay(1000); // Wait for serial ports to initialize
  
  Serial.println("Starting WiFi communication...");
  
  if (!setupWiFi()) {
    Serial.println("Failed to connect to WiFi");
    while(1); // Halt if WiFi connection fails
  }
}

void loop() {
  // Example usage
  static unsigned long lastSendTime = 0;
  
  if (millis() - lastSendTime > 5000) { // Send data every 5 seconds
    SensorData data = {
      .gas = analogRead(A0),
      .temp = analogRead(A1),
      .sensor3 = analogRead(A2)
    };
    
    if (!sendDataToServer(data)) {
      Serial.println("Failed to send data to server");
    }
    
    lastSendTime = millis();
  }
}
