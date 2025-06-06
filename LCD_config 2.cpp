
  /**
   * نمایش اطلاعات سیستم
   */
  bool showSystemInfo() {
    if (!checkHardware()) return false;
    
    char ramBuffer[LCD_COLS + 1];
    int freeRam = getFreeRam();
    
    if (freeRam >= 0) {
      snprintf(ramBuffer, sizeof(ramBuffer), "RAM: %d bytes", freeRam);
    } else {
      strncpy(ramBuffer, "RAM: Error", sizeof(ramBuffer) - 1);
      ramBuffer[sizeof(ramBuffer) - 1] = '\0';
    }
    
    return showSystemData("System v1.0", ramBuffer);
  }
  
  /**
   * نمایش پیام سفارشی
   * @param line1 خط اول (نباید null باشد)
   * @param line2 خط دوم (اختیاری)
   */
  bool showCustomMessage(const char* line1, const char* line2 = nullptr) {
    return showSystemData(line1, line2);
  }
  
  /**
   * بررسی وضعیت راه‌اندازی
   * @return true اگر LCD راه‌اندازی شده باشد
   */
  bool isReady() const {
    return isInitialized && hardwareWorking;
  }
  
  /**
   * بررسی سلامت سخت‌افزار
   * @return true اگر سخت‌افزار سالم باشد
   */
  bool isHardwareWorking() const {
    return hardwareWorking;
  }
  
  /**
   * تلاش برای بازیابی از خطای سخت‌افزاری
   */
  bool recover() {
    hardwareWorking = false;
    isInitialized = false;
    delay(100);
    return init();
  }
  
private:
  /**
   * بررسی سلامت سخت‌افزار و زمان‌سنج
   */
  bool checkHardware() {
    if (!isInitialized || !hardwareWorking) {
      return false;
    }
    
    // بررسی timeout برای عملیات طولانی
    unsigned long currentTime = millis();
    if (currentTime < lastOperation) { // overflow check
      lastOperation = currentTime;
    }
    
    return true;
  }
  
  /**
   * تست پین‌های LCD
   */
  bool testPins() {
    // بررسی اساسی پین‌ها
    int pins[] = {LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7};
    for (int i = 0; i < 6; i++) {
      if (pins[i] < 0 || pins[i] > 53) { // Arduino Mega max pins
        return false;
      }
      // بررسی تداخل پین‌ها
      for (int j = i + 1; j < 6; j++) {
        if (pins[i] == pins[j]) {
          return false;
        }
      }
    }
    return true;
  }
  
  /**
   * چاپ ایمن خط با بررسی طول
   */
  bool printSafeLine(const char* text, int row) {
    if (text == nullptr || row < 0 || row >= LCD_ROWS) {
      return false;
    }
    
    try {
      char buffer[LCD_COLS + 1];
      size_t textLen = strlen(text);
      size_t copyLen = (textLen > LCD_COLS) ? LCD_COLS : textLen;
      
      strncpy(buffer, text, copyLen);
      buffer[copyLen] = '\0';
      
      lcd.print(buffer);
      return true;
      
    } catch (...) {
      hardwareWorking = false;
      return false;
    }
  }
  
  /**
   * محاسبه ایمن حافظه آزاد
   * @return مقدار حافظه آزاد به بایت یا -1 در صورت خطا
   */
  int getFreeRam() {
    try {
      extern int __heap_start;
      extern void *__brkval;
      int v;
      
      void* heapEnd = __brkval ? __brkval : (void*)&__heap_start;
      int stackStart = (int)&v;
      
      // بررسی صحت آدرس‌ها
      if (stackStart <= (int)heapEnd) {
        return -1; // خطا در محاسبه
      }
      
      int freeRam = stackStart - (int)heapEnd;
      
      // بررسی منطقی بودن نتیجه
      if (freeRam < 0 || freeRam > 8192) { // حداکثر RAM Arduino Uno
        return -1;
      }
      
      return freeRam;
      
    } catch (...) {
      return -1;
    }
  }
};

#endif0