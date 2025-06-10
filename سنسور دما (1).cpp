#ifndef SENSOR_UTILS_H
#define SENSOR_UTILS_H

#define LM35_PIN A0
const float VREF = 5.0;
const int READ_INTERVAL = 2000;
const int NUM_SAMPLES = 10;
const float TEMP_MIN = -10.0;
const float TEMP_MAX = 60.0;
const uint8_t DECIMALS = 1;
const float MIN_VALID_VOLTAGE = 0.05;
const float LM35_SCALE_FACTOR = 100.0;
const int ADC_MAX_VALUE = 1023;
const int SERIAL_TIMEOUT_MS = 3000;

unsigned long previousMillis = 0;

inline void waitForSerial(unsigned long timeoutMs) {
  unsigned long start = millis();
  while (!Serial && (millis() - start < timeoutMs));
}

#endif
