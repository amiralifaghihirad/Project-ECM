#ifndef KEYPAD_MANAGER_H
#define KEYPAD_MANAGER_H

#include <Keypad.h>
#include "system_config.h"

// بررسی وجود تعاریف لازم
#ifndef KEYPAD_ROWS
#error "KEYPAD_ROWS must be defined in system_config.h"
#endif
#ifndef KEYPAD_COLS
#error "KEYPAD_COLS must be defined in system_config.h"
#endif

// اعتبارسنجی مقادیر
#if KEYPAD_ROWS < 1 || KEYPAD_ROWS > 8
#error "KEYPAD_ROWS must be between 1 and 8"
#endif
#if KEYPAD_COLS < 1 || KEYPAD_COLS > 8
#error "KEYPAD_COLS must be between 1 and 8"
#endif

class KeypadManager {
private:
  // تعریف آرایه‌های پین‌ها با بررسی ایمنی
  byte rowPins[KEYPAD_ROWS];
  byte colPins[KEYPAD_COLS];
  
  // ماتریس کلیدها
  char keys[KEYPAD_ROWS][KEYPAD_COLS];
  
  Keypad* keypad;
  bool isInitialized;
  bool hardwareWorking;
  unsigned long lastKeyTime;
  char lastValidKey;
  
  static const unsigned long DEBOUNCE_DELAY = 50; // میلی‌ثانیه
  static const unsigned long MAX_TIMEOUT = 300000UL; // 5 دقیقه حداکثر
  
public:
  /**
   * سازنده کلاس KeypadManager
   */
  KeypadManager() : keypad(nullptr), isInitialized(false), hardwareWorking(false), 
                    lastKeyTime(0), lastValidKey(NO_KEY) {
    initializeArrays();
  }
  
  /**
   * تخریب‌گر
   */
  ~KeypadManager() {
    if (keypad) {
      delete keypad;
    }
  }
  
  /**
   * راه‌اندازی کی‌پد با بررسی کامل
   * @return true در صورت موفقیت
   */
  bool init() {
    try {
      // بررسی پین‌ها
      if (!validatePins()) {
        return false;
      }
      
      // ایجاد keymap
      char* keymap = makeKeymap((char*)keys);
      if (!keymap) {
        return false;
      }
      
      // ایجاد شیء Keypad
      keypad = new Keypad(keymap, rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);
      if (!keypad) {
        return false;
      }
      
      // تست اولیه
      keypad->setDebounceTime(DEBOUNCE_DELAY);
      keypad->setHoldTime(500);
      
      // تست سخت‌افزار
      if (!testHardware()) {
        delete keypad;
        keypad = nullptr;
        return false;
      }
      
      isInitialized = true;
      hardwareWorking = true;
      lastKeyTime = millis();
      
      // پیام موفقیت فقط در صورت فعال بودن Serial
      if (Serial) {
        Serial.println(F("Keypad initialized successfully"));
      }
      
      return true;
      
    } catch (...) {
      cleanup();
      return false;
    }
  }
  
  /**
   * دریافت کلید فشرده شده با بررسی خطا
   * @return کاراکتر کلید یا NO_KEY
   */
  char getKey() {
    if (!checkHardware()) return NO_KEY;
    
    try {
      char key = keypad->getKey();
      
      if (key != NO_KEY) {
        // بررسی debounce
        unsigned long currentTime = millis();
        if (currentTime < lastKeyTime) { // overflow check
          lastKeyTime = currentTime;
        }
        
        if (currentTime - lastKeyTime >= DEBOUNCE_DELAY) {
          lastValidKey = key;
          lastKeyTime = currentTime;
          return key;
        }
      }
      
      return NO_KEY;
      
    } catch (...) {
      hardwareWorking = false;
      return NO_KEY;
    }
  }
  
  /**
   * بررسی فشرده بودن کلید خاص
   * @param key کلید مورد نظر
   * @return true اگر کلید فشرده باشد
   */
  bool isPressed(char key) {
    if (!checkHardware() || !isValidKey(key)) return false;
    
    try {
      KeyState state = keypad->getState();
      char currentKey = keypad->getKey();
      
      return (state == PRESSED && currentKey == key);
      
    } catch (...) {
      hardwareWorking = false;
      return false;
    }
  }
  
  /**
   * دریافت وضعیت کی‌پد
   * @return وضعیت فعلی کی‌پد
   */
  KeyState getKeyState() {
    if (!checkHardware()) return IDLE;
    
    try {
      return keypad->getState();
    } catch (...) {
      hardwareWorking = false;
      return IDLE;
    }
  }
  
  /**
   * بررسی فشرده شدن هر کلید
   * @return true اگر کلیدی فشرده شده باشد
   */
  bool isAnyKeyPressed() {
    if (!checkHardware()) return false;
    
    char key = getKey();
    return (key != NO_KEY);
  }
  
  /**
   * انتظار برای فشردن کلید (غیرمسدود کننده) با بررسی overflow
   * @param timeout زمان انتظار (میلی‌ثانیه)
   * @return کلید فشرده شده یا NO_KEY در صورت timeout
   */
  char waitForKeyTimeout(unsigned long timeout) {
    if (!checkHardware() || timeout == 0 || timeout > MAX_TIMEOUT) {
      return NO_KEY;
    }
    
    unsigned long startTime = millis();
    char key = NO_KEY;
    
    while (key == NO_KEY) {
      unsigned long currentTime = millis();
      
      // بررسی overflow
      unsigned long elapsed;
      if (currentTime >= startTime) {
        elapsed = currentTime - startTime;
      } else {
        // overflow occurred
        elapsed = (ULONG_MAX - startTime) + currentTime + 1;
      }
      
      if (elapsed >= timeout) {
        break;
      }
      
      key = getKey();
      
      if (key == NO_KEY) {
        delay(10); // تاخیر قابل تنظیم
      }
    }
    
    return key;
  }
  00