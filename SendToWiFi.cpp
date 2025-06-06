/*
    Author: [یوسف رستمی]
*/


#include <Arduino.h>
#include <ArduinoJson.h>  // افزودن کتابخانه ضروری JSON
#include <WiFiClient.h>   // برای ارتباط WiFi (مخصوص ESP)

struct SensorData {
    float gas;
    float temp;
    float sensor3;
};

void sendToWiFi(SensorData data) {
    // مدیریت اولیه سریال (فقط برای دیباگ)
    #if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) // فقط برای بردهایی که نیاز به انتظار دارند
    static bool isSerialInitialized = false;
    if (!isSerialInitialized) {
        Serial.begin(9600);
        while (!Serial) {} // فقط برای بردهای خاص مثل Leonardo
        isSerialInitialized = true;
    }
    #endif

    // مدیریت حافظه JSON با بررسی خطا
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

    // دیباگ خروجی
    Serial.print("[WiFi] Sending: ");
    Serial.println(payload);

    // منطق واقعی ارسال WiFi (برای ESP8266/ESP32)
    #if defined(ESP8266) || defined(ESP32)
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[ERROR] WiFi not connected");
        return;
    }

    WiFiClient client;
    if (!client.connect("api.example.com", 80)) {
        Serial.println("[ERROR] Connection to server failed");
        return;
    }

    client.println("POST /data HTTP/1.1");
    client.println("Host: api.example.com");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(payload.length());
    client.println();
    client.println(payload);

    // دریافت پاسخ (اختیاری)
    delay(10);
    while (client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print("[SERVER] ");
        Serial.println(line);
    }
    #endif
}

// مثال استفاده با پیکربندی WiFi
void setup() {
    #if defined(ESP8266) || defined(ESP32)
    WiFi.begin("SSID", "PASSWORD"); // جایگزین با اطلاعات شبکه شما
    Serial.begin(9600);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected!");
    #endif

    SensorData testData = {12.5, 23.7, 45.0};
    sendToWiFi(testData);
}

void loop() {
    // کد اصلی (در صورت نیاز)
}
