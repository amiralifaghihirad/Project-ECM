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

void waitForSerial(unsigned long timeoutMs) {
  unsigned long start = millis();
  while (!Serial && (millis() - start < timeoutMs));
}

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
          // اگر چاپ سریال موفق نبود (محدود به Serial بافر)
          // اینجا می‌شه خطای لاجیک یا LED هشدار اضافه کرد
        }
      }
    }
  }
}