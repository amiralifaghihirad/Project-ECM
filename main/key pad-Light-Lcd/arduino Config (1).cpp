```cpp
#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <climits>

// Helper constants with validation
#define MAX_STRING_TO_INT_LENGTH 10
#define MAX_TIME_BUFFER_SIZE 32  // Increased size to ensure enough space
#define MIN_TIME_BUFFER_SIZE 20  // Minimum required size

class Utils {
private:
  static bool isRandomInitialized;
  
  // Check random initialization
  static void ensureRandomInitialized() {
    if (!isRandomInitialized) {
      randomSeed(analogRead(0) + millis());
      isRandomInitialized = true;
    }
  }

public:
  /**
   * Converts milliseconds to readable format
   * @param milliseconds Millisecond value
   * @param buffer Output buffer
   * @param bufferSize Buffer size
   * @return Number of characters written or -1 on error
   */
  static int formatTime(unsigned long milliseconds, char* buffer, int bufferSize) {
    // Input validation
    if (buffer == nullptr || bufferSize < MIN_TIME_BUFFER_SIZE) {
      return -1;
    }
    
    // Initialize buffer
    buffer[0] = '\0';
    
    unsigned long seconds = milliseconds / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    
    seconds = seconds % 60;
    minutes = minutes % 60;
    
    int written = 0;
    int remaining = bufferSize - 1; // Remaining space (minus null terminator)
    
    // Check for overflow at each step
    if (hours > 0 && remaining > 0) {
      int temp = snprintf(buffer + written, remaining, "%luh ", hours);
      if (temp < 0 || temp >= remaining) {
        return -1; // snprintf error or insufficient space
      }
      written += temp;
      remaining -= temp;
    }
    
    if (minutes > 0 && remaining > 0) {
      int temp = snprintf(buffer + written, remaining, "%lum ", minutes);
      if (temp < 0 || temp >= remaining) {
        return -1;
      }
      written += temp;
      remaining -= temp;
    }
    
    if (remaining > 0) {
      int temp = snprintf(buffer + written, remaining, "%lus", seconds);
      if (temp < 0 || temp >= remaining) {
        return -1;
      }
      written += temp;
    }
    
    return written;
  }
  
  /**
   * Generates a random number within a range
   * @param min Minimum value
   * @param max Maximum value
   * @return Random number or INT_MIN on error
   */
  static int randomRange(int min, int max) {
    if (min > max) {
      return INT_MIN; // Input error
    }
    
    ensureRandomInitialized();
    return random(min, max + 1);
  }
  
  /**
   * Checks if a number is within a range
   * @param value Value to check
   * @param min Minimum value
   * @param max Maximum value
   * @return true if within range
   */
  static bool isInRange(int value, int min, int max) {
    if (min > max) return false; // Invalid range
    return (value >= min && value <= max);
  }
  
  /**
   * Converts string to integer with error checking and length limit
   * @param str Input string
   * @param success Pointer to success variable
   * @return Converted number or 0 on error
   */
  static long stringToInt(const char* str, bool* success) {
    if (success) *success = false;
    
    // Check for invalid inputs
    if (str == nullptr) {
      return 0;
    }
    
    size_t len = strlen(str);
    if (len == 0 || len > MAX_STRING_TO_INT_LENGTH) {
      return 0;
    }
    
    long result = 0;
    bool isNegative = false;
    int startIndex = 0;
    
    // Check sign
    if (str[0] == '-') {
      isNegative = true;
      startIndex = 1;
      if (len == 1) return 0; // Only negative sign
    } else if (str[0] == '+') {
      startIndex = 1;
      if (len == 1) return 0; // Only positive sign
    }
    
    // Process digits
    for (int i = startIndex; str[i] != '\0'; i++) {
      if (str[i] < '0' || str[i] > '9') {
        return 0; // Non-numeric character
      }
      
      int digit = str[i] - '0';
      
      // More precise overflow checking
      if (isNegative) {
        if (result < (LONG_MIN + digit) / 10) {
          return 0; // Negative overflow
        }
        result = result * 10 - digit;
      } else {
        if (result > (LONG_MAX - digit) / 10) {
          return 0; // Positive overflow
        }
        result = result * 10 + digit;
      }
    }
    
    if (success) *success = true;
    return result;
  }
  
  /**
   * Constrains a number within a range
   * @param value Input value
   * @param min Minimum value
   * @param max Maximum value
   * @return Constrained value or original value if range is invalid
   */
  static int constrainValue(int value, int min, int max) {
    if (min > max) return value; // Invalid range
    return constrain(value, min, max);
  }
  
  /**
   * Converts Celsius temperature to Fahrenheit
   * @param celsius Celsius temperature
   * @return Fahrenheit temperature
   */
  static float celsiusToFahrenheit(float celsius) {
    // Check logical range
    if (celsius < -273.15f || celsius > 10000.0f) {
      return NAN; // Illogical temperature
    }
    return (celsius * 9.0f / 5.0f) + 32.0f;
  }
  
  /**
   * Converts Fahrenheit temperature to Celsius
   * @param fahrenheit Fahrenheit temperature
   * @return Celsius temperature
   */
  static float fahrenheitToCelsius(float fahrenheit) {
    // Check logical range
    if (fahrenheit < -459.67f || fahrenheit > 18000.0f) {
      return NAN; // Illogical temperature
    }
    return (fahrenheit - 32.0f) * 5.0f / 9.0f;
  }
  
  /**
   * Checks if a specified time has passed, considering millis() overflow
   * @param startTime Start time
   * @param interval Time interval
   * @return true if time has passed
   */
  static bool hasTimePassed(unsigned long startTime, unsigned long interval) {
    if (interval == 0) return true; // Zero interval always passed
    
    unsigned long currentTime = millis();
    
    // Check for millis() overflow (every 49.7 days)
    if (currentTime >= startTime) {
      return (currentTime - startTime >= interval);
    } else {
      // Overflow occurred
      return ((ULONG_MAX - startTime + currentTime + 1) >= interval);
    }
  }
  
  /**
   * Non-blocking delay with overflow checking
   * @param previousTime Pointer to previous time
   * @param interval Time interval
   * @return true if time has elapsed
   */
  static bool nonBlockingDelay(unsigned long* previousTime, unsigned long interval) {
    if (previousTime == nullptr || interval == 0) {
      return false;
    }
    
    if (hasTimePassed(*previousTime, interval)) {
      *previousTime = millis();
      return true;
    }
    return false;
  }
  
  /**
   * Calculates the average of an array with overflow checking
   * @param values Array of values
   * @param length Array length
   * @return Average or NAN on error
   */
  static float calculateAverage(const int values[], int length) {
    if (values == nullptr || length <= 0) {
      return NAN;
    }
    
    // Check length limit to prevent overflow
    if (length > 1000000) { // Logical limit
      return NAN;
    }
    
    long long sum = 0;
    
    for (int i = 0; i < length; i++) {
      // Check for overflow before addition
      if ((sum > 0 && values[i] > LLONG_MAX - sum) ||
          (sum < 0 && values[i] < LLONG_MIN - sum)) {
        return NAN; // Overflow in sum
      }
      sum += values[i];
    }
    
    return (float)sum / length;
  }
  
  /**
   * Checks if a pointer and array size are valid
   * @param ptr Pointer
   * @param size Size
   * @return true if valid
   */
  static bool isValidArray(const void* ptr, int size) {
    return (ptr != nullptr && size > 0);
  }
  
  /**
   * Calculates the maximum value in an array
   * @param values Array of values
   * @param length Array length
   * @param maxValue Pointer to max variable
   * @return true on success
   */
  static bool findMaxValue(const int values[], int length, int* maxValue) {
    if (!isValidArray(values, length) || maxValue == nullptr) {
      return false;
    }
    
    *maxValue = values[0];
    for (int i = 1; i < length; i++) {
      if (values[i] > *maxValue) {
        *maxValue = values[i];
      }
    }
    return true;
  }
  
  /**
   * Calculates the minimum value in an array
   * @param values Array of values
   * @param length Array length
   * @param minValue Pointer to min variable
   * @return true on success
   */
  static bool findMinValue(const int values[], int length, int* minValue) {
    if (!isValidArray(values, length) || minValue == nullptr) {
      return false;
    }
    
    *minValue = values[0];
    for (int i = 1; i < length; i++) {
      if (values[i] < *minValue) {
        *minValue = values[i];
      }
    }
    return true;
  }
};

// Definition of static variable
bool Utils::isRandomInitialized = false;

#endif // UTILS_H
