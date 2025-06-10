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