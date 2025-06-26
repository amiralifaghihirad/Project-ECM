#include "sensor_utils.h"

void setup() {
  delay(1000);
  Serial.begin(9600);
#if defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_AVR_LEONARDO)
  waitForSerial(SERIAL_TIMEOUT_MS);
#endif
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= READ_INTERVAL) {
    previousMillis = currentMillis;

    float analogSum = 0;
    bool invalidSample = false;

    for (int i = 0; i < NUM_SAMPLES; i++) {
      int sample = analogRead(LM35_PIN);
      if (sample < 0 || sample > ADC_MAX_VALUE) {
        invalidSample = true;
        break;
      }
      analogSum += sample;
      delay(10);
    }

    if (invalidSample) {
      Serial.println("{\"error\":\"Invalid ADC reading\"}");
    } else {
      float analogAverage = analogSum / NUM_SAMPLES;
      float voltage = analogAverage * (VREF / ADC_MAX_VALUE);

      if (voltage < MIN_VALID_VOLTAGE) {
        Serial.println("{\"error\":\"Sensor not connected or faulty\"}");
      } else {
        float temperature = voltage * LM35_SCALE_FACTOR;
        bool outOfRange = (temperature < TEMP_MIN || temperature > TEMP_MAX);

        bool status = Serial.print("{");
        if (outOfRange) {
          status &= Serial.print("\"warning\":\"Temperature out of range\",");
        }
        status &= Serial.print("\"temp\":");
        status &= Serial.print(temperature, DECIMALS);
        status &= Serial.println("}");

        if (!status) {
          // TODO: handle serial failure (optional)
        }
      }
    }
  }
}