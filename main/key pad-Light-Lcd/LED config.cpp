```cpp
#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include "system_config.h"

// Checking for necessary definitions
#ifndef WARNING_LED_PIN
#error "WARNING_LED_PIN must be defined in system_config.h"
#endif
#ifndef LED_BLINK_INTERVAL
#error "LED_BLINK_INTERVAL must be defined in system_config.h"
#endif

// Validating values
#if WARNING_LED_PIN < 0 || WARNING_LED_PIN > 53
#error "WARNING_LED_PIN must be a valid Arduino pin (0-53)"
#endif
#if LED_BLINK_INTERVAL < 50 || LED_BLINK_INTERVAL > 10000
#error "LED_BLINK_INTERVAL must be between 50 and 10000 milliseconds"
#endif

// Define LED modes
enum LEDMode {
  LED_OFF,
  LED_ON,
  LED_WARNING_BLINK,
  LED_LIMITED_BLINK,
  LED_PATTERN_EMERGENCY,
  LED_PATTERN_ALERT
};

class LEDManager {
private:
  LEDMode currentMode;
  bool ledState;
  unsigned long lastBlink;
  bool isInitialized;
  bool hardwareWorking;
  
  // Limited blink variables
  int blinkCount;
  int maxBlinks;
  bool blinkingActive;
  
  // Pattern settings
  unsigned long currentInterval;
  
  // Security limitations
  static const unsigned long MIN_INTERVAL = 50;
  static const unsigned long MAX_INTERVAL = 10000;
  static const int MAX_BLINKS = 1000;
  static const unsigned long OPERATION_TIMEOUT = 5000; // 5 seconds
  
public:
  /**
   * Constructor for LEDManager class
   */
  LEDManager() {
    currentMode = LED_OFF;
    ledState = false;
    lastBlink = 0;
    isInitialized = false;
    hardwareWorking = false;
    blinkCount = 0;
    maxBlinks = 0;
    blinkingActive = false;
    currentInterval = LED_BLINK_INTERVAL;
  }
  
  /**
   * Initialize LED Manager with full validation
   * @return true if successful
   */
  bool init() {
    try {
      // Pin validation
      if (!validatePin()) {
        return false;
      }
      
      // Initialize pin
      pinMode(WARNING_LED_PIN, OUTPUT);
      
      // Hardware test
      if (!testHardware()) {
        return false;
      }
      
      // Initial setup
      digitalWrite(WARNING_LED_PIN, LOW);
      ledState = false;
      currentMode = LED_OFF;
      lastBlink = millis();
      
      isInitialized = true;
      hardwareWorking = true;
      
      // Success message
      if (Serial) {
        Serial.println(F("LED Manager initialized successfully"));
      }
      
      return true;
      
    } catch (...) {
      isInitialized = false;
      hardwareWorking = false;
      return false;
    }
  }
  
  /**
   * Update LED status (must be called in loop)
   */
  void update() {
    if (!checkHardware()) return;
    
    unsigned long currentTime = millis();
    
    // Check for overflow
    if (currentTime < lastBlink) {
      lastBlink = currentTime;