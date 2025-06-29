#ifndef LCD_MANAGER_H
#define LCD_MANAGER_H

#include <LiquidCrystal.h>
#include "system_config.h"

// بررسی وجود تعاریف لازم در system_config.h
#ifndef LCD_RS
#error "LCD_RS must be defined in system_config.h"
#endif
#ifndef LCD_EN
#error "LCD_EN must be defined in system_config.h"
#endif
#ifndef LCD_D4
#error "LCD_D4 must be defined in system_config.h"
#endif
#ifndef LCD_D5
#error "LCD_D5 must be defined in system_config.h"
#endif
#ifndef LCD_D6
#error "LCD_D6 must be defined in system_config.h"
#endif
#ifndef LCD_D7
#error "LCD_D7 must be defined in system_config.h"
#endif
#ifndef LCD_COLS
#error "LCD_COLS must be defined in system_config.h"
#endif
#ifndef LCD_ROWS
#error "LCD_ROWS must be defined in system_config.h"
#endif

// اعتبارسنجی مقادیر
#if LCD_COLS < 8 || LCD_COLS > 40
#error "LCD_COLS must be between 8 and 40"
#endif
#if LCD_ROWS < 1 || LCD_ROWS > 4
#error "LCD_ROWS must be between 1 and 4"
#endif

class LCDManager {
private:
  LiquidCrystal lcd;
  bool isInitialized;
  bool hardwareWorking;
  unsigned long lastOperation;
  static const unsigned long OPERATION_TIMEOUT = 100; // میلی‌ثانیه
  
public:
  /**
   * سازنده کلاس LCDManager
   * پین‌های LCD را از system_config.h دریافت می‌کند
   */
  LCDManager() : lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7), 
                 isInitialized(false), hardwareWorking(false), lastOperation(0) {}
  
  /**
   * راه‌اندازی LCD با بررسی خطا
   * @return true در صورت موفقیت
   */
  bool init() {
    try {
      // تست پین‌ها
      if (!testPins()) {
        return false;
      }
      
      lcd.begin(LCD_COLS, LCD_ROWS);
      delay(50); // زمان برای تثبیت
      
      // تست نمایشگر با نوشتن و خواندن
      lcd.clear();
      delay(10);
      lcd.setCursor(0, 0);
      lcd.print("Test");
      delay(10);
      
      hardwareWorking = true;
      isInitialized = true;
      lastOperation = millis();
      
      // پاک کردن نهایی
      lcd.clear();
      return true;
      
    } catch (...) {
      isInitialized = false;
      hardwareWorking = false;
      return false;
    }
  }
  
  /**
   * پاک کردن نمایشگر با بررسی خطا
   */
  bool clear() {
    if (!checkHardware()) return false;
    
    try {
      lcd.clear();
      lastOperation = millis();
      return true;
    } catch (...) {
      hardwareWorking = false;
      return false;
    }
  }
  
  /**
   * نمایش پیام خوش‌آمدگویی
   */
  bool showWelcome() {
    if (!checkHardware()) return false;
    
    return showSystemData("Arduino System", "Press Key...");
  }
  
  /**
   * نمایش منوی اصلی
   */
  bool showMenu() {
    if (!checkHardware()) return false;
    
    return showSystemData("1:Toggle 2:Menu", "3:Info *:Clear");
  }
  
  /**
   * نمایش وضعیت سیستم
   * @param status رشته وضعیت (نباید null باشد)
   */
  bool showStatus(const char* status) {
    if (!checkHardware() || status == nullptr) return false;
    
    return showSystemData("Status:", status);
  }
  
  /**
   * نمایش ورودی کاربر
   * @param input رشته ورودی (نباید null باشد)
   */
  bool showInput(const char* input) {
    if (!checkHardware() || input == nullptr) return false;
    
    return showSystemData("Input:", input);
  }
  
  /**
   * نمایش داده‌های سیستم با فرمت بهینه
   * @param line1 خط اول (نباید null باشد)
   * @param line2 خط دوم (اختیاری)
   */
  bool showSystemData(const char* line1, const char* line2 = nullptr) {
    if (!checkHardware() || line1 == nullptr) return false;
    
    try {
      lcd.clear();
      delay(2);
      
      // نمایش خط اول
      lcd.setCursor(0, 0);
      if (!printSafeLine(line1, 0)) return false;
      
      // نمایش خط دوم در صورت وجود
      if (line2 != nullptr && LCD_ROWS > 1) {
        lcd.setCursor(0, 1);
        if (!printSafeLine(line2, 1)) return false;
      }
      
      lastOperation = millis();
      return true;
      
    } catch (...) {
      hardwareWorking = false;
      return false;
    }
  }
  