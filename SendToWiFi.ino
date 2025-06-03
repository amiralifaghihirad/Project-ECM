#include <Arduino.h>

struct SensorData {
    float gas;
    float temp;
    float sensor3;
};

void sendToWiFi(SensorData data) {
    // Initialize Serial only once (better for memory)
    static bool isSerialInitialized = false;
    if (!isSerialInitialized) {
        Serial.begin(9600);
        while (!Serial); // Wait for serial port to connect (for boards like Leonardo)
        isSerialInitialized = true;
    }

    // Use ArduinoJson for more reliable JSON generation
    // (Install via Library Manager: "ArduinoJson" by Benoit Blanchon)
    DynamicJsonDocument doc(128);
    doc["gas"] = data.gas;
    doc["temp"] = data.temp;
    doc["sensor3"] = data.sensor3;

    String payload;
    serializeJson(doc, payload);

    // Debug output
    Serial.print("[WiFi] Sending: ");
    Serial.println(payload);

    // TODO: Add actual WiFi transmission logic here
    // Example for ESP8266:
    // WiFiClient client;
    // if (client.connect("api.example.com", 80)) {
    //     client.println("POST /data HTTP/1.1");
    //     client.println("Host: api.example.com");
    //     client.println("Content-Type: application/json");
    //     client.print("Content-Length: ");
    //     client.println(payload.length());
    //     client.println();
    //     client.println(payload);
    // }
}

// Example usage
void setup() {
    SensorData testData = {12.5, 23.7, 45.0};
    sendToWiFi(testData);
}

void loop() {
    // Your main loop
}
