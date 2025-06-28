/*
    Author: [یوسف رستمی]
*/


#include <Arduino.h>
#include <ArduinoJson.h>
#if defined(ESP8266) || defined(ESP32)
  #include <WiFiClient.h>
  #include <ESP8266WiFi.h> // یا <WiFi.h> برای ESP32
#endif

struct SensorData {
  float gas;
  float temp;
  float sensor3;
};

void initSerialForDebug() {
  // فقط برای بردهای AVR (مثل Uno، Leonardo) و SAM
  #if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM)
    Serial.begin(9600);
    while (!Serial); // انتظار فقط برای این بردها
  #else
    // برای ESPها و سایر بردها، سریال را بدون انتظار راه‌اندازی می‌کنیم
    Serial.begin(9600);
  #endif
}

void sendToWiFi(SensorData data) {
  // سریال‌سازی JSON با مدیریت خطا
  DynamicJsonDocument doc(128);
  if (doc.capacity() == 0) {
    Serial.println("[ERROR] Failed to allocate memory for JSON");
    return;
  }

  doc["gas"] = data.gas;
  doc["temp"] = data.temp;
  doc["sensor3"] = data.sensor3;

  String payload;
  if (serializeJson(doc, payload) == 0) {
    Serial.println("[ERROR] JSON serialization failed");
    return;
  }

  Serial.print("[WiFi] Sending: ");
  Serial.println(payload);

  // بخش مخصوص ESPها
  #if defined(ESP8266) || defined(ESP32)
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[ERROR] WiFi not connected");
    return;
  }

  WiFiClient client;
  if (!client.connect("api.example.com", 80)) {
    Serial.println("[ERROR] Connection failed");
    return;
  }

  // ارسال داده (مشابه قبل)
  #endif
}

void setup() {
  initSerialForDebug(); // مقداردهی هوشمند سریال
  
  #if defined(ESP8266) || defined(ESP32)
  WiFi.begin("SSID", "PASSWORD");
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  #endif

  SensorData testData = {12.5, 23.7, 45.0};
  sendToWiFi(testData);
}

void loop() {}
