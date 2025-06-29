#include <Arduino.h>

// --- Sensor Read Intervals (in ms) ---
const unsigned long TEMP_INTERVAL = 2000;
const unsigned long HUMIDITY_INTERVAL = 5000;
const unsigned long LIGHT_INTERVAL = 10000;

// --- Timing variables ---
unsigned long lastTempRead = 0;
unsigned long lastHumidityRead = 0;
unsigned long lastLightRead = 0;

// --- Sensor enable flags ---
bool tempSensorEnabled = true;
bool humiditySensorEnabled = true;
bool lightSensorEnabled = true;

// --- Struct to hold sensor data ---
struct SensorData {
  float temperature;
  float humidity;
  int light;
  bool hasTemperature;
  bool hasHumidity;
  bool hasLight;
};

// --- Simulated temperature reading ---
float readTemperature() {
  return 20.0 + random(-50, 50) / 10.0;  // 15.0 - 25.0
}

// --- Simulated humidity reading ---
float readHumidity() {
  return 50.0 + random(-200, 200) / 10.0;  // 30.0 - 70.0
}

// --- Simulated light level reading ---
int readLightLevel() {
  return random(0, 1024);  // Assume analog range
}

// --- Read enabled sensors ---
SensorData readSensors(bool readTemp, bool readHum, bool readLight) {
  SensorData data;
  data.hasTemperature = false;
  data.hasHumidity = false;
  data.hasLight = false;

  if (readTemp && tempSensorEnabled) {
    data.temperature = readTemperature();
    data.hasTemperature = true;
  }

  if (readHum && humiditySensorEnabled) {
    data.humidity = readHumidity();
    data.hasHumidity = true;
  }

  if (readLight && lightSensorEnabled) {
    data.light = readLightLevel();
    data.hasLight = true;
  }

  return data;
}

// --- Send data as JSON ---
void sendToWiFi(const SensorData& data) {
  Serial.println("Sending data to WiFi...");
  Serial.print("{");

  bool first = true;

  if (data.hasTemperature) {
    Serial.print("\"temperature\":");
    Serial.print(data.temperature);
    first = false;
  }

  if (data.hasHumidity) {
    if (!first) Serial.print(", ");
    Serial.print("\"humidity\":");
    Serial.print(data.humidity);
    first = false;
  }

  if (data.hasLight) {
    if (!first) Serial.print(", ");
    Serial.print("\"light\":");
    Serial.print(data.light);
  }

  Serial.println("}");
}

// --- Handle Serial input for enabling/disabling sensors ---
void handleSerialCommands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "TEMP ON") {
      tempSensorEnabled = true;
      Serial.println("Temperature sensor ENABLED.");
    } else if (command == "TEMP OFF") {
      tempSensorEnabled = false;
      Serial.println("Temperature sensor DISABLED.");
    } else if (command == "HUM ON") {
      humiditySensorEnabled = true;
      Serial.println("Humidity sensor ENABLED.");
    } else if (command == "HUM OFF") {
      humiditySensorEnabled = false;
      Serial.println("Humidity sensor DISABLED.");
    } else if (command == "LIGHT ON") {
      lightSensorEnabled = true;
      Serial.println("Light sensor ENABLED.");
    } else if (command == "LIGHT OFF") {
      lightSensorEnabled = false;
      Serial.println("Light sensor DISABLED.");
    } else if (command == "STATUS") {
      Serial.println("--- Sensor Status ---");
      Serial.print("Temperature: "); Serial.println(tempSensorEnabled ? "ON" : "OFF");
      Serial.print("Humidity: "); Serial.println(humiditySensorEnabled ? "ON" : "OFF");
      Serial.print("Light: "); Serial.println(lightSensorEnabled ? "ON" : "OFF");
    } else {
      Serial.println("Unknown command.");
    }
  }
}

void setup() {
  Serial.begin(9600);
  delay(1000); // Wait for Serial monitor
  Serial.println("System initialized. Type 'STATUS' to check sensor status.");
}

void loop() {
  unsigned long currentMillis = millis();

  bool readTemp = false;
  bool readHum = false;
  bool readLight = false;

  if (currentMillis - lastTempRead >= TEMP_INTERVAL) {
    lastTempRead = currentMillis;
    readTemp = true;
  }

  if (currentMillis - lastHumidityRead >= HUMIDITY_INTERVAL) {
    lastHumidityRead = currentMillis;
    readHum = true;
  }

  if (currentMillis - lastLightRead >= LIGHT_INTERVAL) {
    lastLightRead = currentMillis;
    readLight = true;
  }

  // Only read if needed
  if (readTemp || readHum || readLight) {
    SensorData data = readSensors(readTemp, readHum, readLight);
    sendToWiFi(data);
  }

  handleSerialCommands();
}