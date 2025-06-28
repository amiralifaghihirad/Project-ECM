#include <DHT.h>

#define DHTPIN 7
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

unsigned long previousMillis = 0;
const unsigned long interval = 2000;

const float MIN_TEMP = -40.0;
const float MAX_TEMP = 80.0;
const float MIN_HUM  = 0.0;
const float MAX_HUM  = 100.0;

void setup() {
  Serial.begin(9600);
  dht.begin();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    bool validHumidity = humidity >= MIN_HUM && humidity <= MAX_HUM;
    bool validTemperature = temperature >= MIN_TEMP && temperature <= MAX_TEMP;

    if (!validHumidity || !validTemperature) {
      Serial.println("Sensor data out of valid range.");
      return;
    }

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print(" % | ");

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
  }
}