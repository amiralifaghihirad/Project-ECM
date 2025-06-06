#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include "system_config.h"

// بررسی وجود تعاریف لازم
#ifndef WARNING_LED_PIN
#error "WARNING_LED_PIN must be defined in system_config.h"
#endif
#ifndef LED_BLINK_INTERVAL
#error "LED_BLINK_INTERVAL must be defined in system_config.h"
#endif

// اعتبارسنجی مقادیر
#if WARNING_LED_PIN < 0 || WARNING_LED_PIN > 53
#error "WARNING_LED_PIN must be a valid Arduino pin (0-53)"
#endif
#if LED_BLINK_INTERVAL < 50 || LED_BLINK_INTERVAL > 10000
#error "LED_BLINK_INTERVAL must be between 50 and 10000 milliseconds"
#endif

// تعریف حالت‌های LED
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
  
  // متغیرهای چشمک محدود
  int blinkCount;
  int maxBlinks;
  bool blinkingActive;
  
  // تنظیمات الگوها
  unsigned long currentInterval;
  
  // محدودیت‌های امنیتی
  static const unsigned long MIN_INTERVAL = 50;
  static const unsigned long MAX_INTERVAL = 10000;
  static const int MAX_BLINKS = 1000;
  static const unsigned long OPERATION_TIMEOUT = 5000; // 5 ثانیه
  
public:
  /**
   * سازنده کلاس LEDManager
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
   * راه‌اندازی LED Manager با بررسی کامل
   * @return true در صورت موفقیت
   */
  bool init() {
    try {
      // بررسی پین
      if (!validatePin()) {
        return false;
      }
      
      // راه‌اندازی پین
      pinMode(WARNING_LED_PIN, OUTPUT);
      
      // تست سخت‌افزار
      if (!testHardware()) {
        return false;
      }
      
      // تنظیم اولیه
      digitalWrite(WARNING_LED_PIN, LOW);
      ledState = false;
      currentMode = LED_OFF;
      lastBlink = millis();
      
      isInitialized = true;
      hardwareWorking = true;
      
      // پیام موفقیت
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
   * بروزرسانی وضعیت LED (باید در loop فراخوانی شود)
   */
  void update() {
    if (!checkHardware()) return;
    
    unsigned long currentTime = millis();
    
    // بررسی overflow
    if (currentTime < lastBlink) {
      lastBlink = currentTime;