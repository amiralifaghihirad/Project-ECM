/**
   * بررسی کلیدهای عددی
   * @param key کلید مورد بررسی
   * @return true اگر کلید عددی باشد
   */
  bool isNumericKey(char key) const {
    return (key >= '0' && key <= '9');
  }
  
  /**
   * بررسی کلیدهای حرفی
   * @param key کلید مورد بررسی
   * @return true اگر کلید حرفی باشد
   */
  bool isLetterKey(char key) const {
    return (key >= 'A' && key <= 'D');
  }
  
  /**
   * بررسی کلیدهای خاص
   * @param key کلید مورد بررسی
   * @return true اگر کلید خاص باشد
   */
  bool isSpecialKey(char key) const {
    return (key == '*' || key == '#');
  }
  
  /**
   * بررسی معتبر بودن کلید
   * @param key کلید مورد بررسی
   * @return true اگر کلید معتبر باشد
   */
  bool isValidKey(char key) const {
    if (key == NO_KEY) return false;
    
    for (int row = 0; row < KEYPAD_ROWS; row++) {
      for (int col = 0; col < KEYPAD_COLS; col++) {
        if (keys[row][col] == key) {
          return true;
        }
      }
    }
    return false;
  }
  
  /**
   * تبدیل کلید به عدد
   * @param key کلید مورد نظر
   * @return عدد معادل یا -1 در صورت خطا
   */
  int keyToNumber(char key) const {
    if (isNumericKey(key)) {
      return key - '0';
    }
    return -1;
  }
  
  /**
   * بررسی وضعیت راه‌اندازی
   * @return true اگر کی‌پد راه‌اندازی شده باشد
   */
  bool isReady() const {
    return isInitialized && hardwareWorking && keypad != nullptr;
  }
  
  /**
   * ریست کامل کی‌پد
   */
  bool reset() {
    if (!keypad) return false;
    
    try {
      // پاک کردن وضعیت داخلی
      while (keypad->getKey() != NO_KEY) {
        delay(10);
      }
      
      lastValidKey = NO_KEY;
      lastKeyTime = millis();
      hardwareWorking = true;
      
      return true;
      
    } catch (...) {
      hardwareWorking = false;
      return false;
    }
  }
  
  /**
   * بازیابی از خطای سخت‌افزاری
   */
  bool recover() {
    cleanup();
    delay(100);
    return init();
  }
  
private:
  /**
   * مقداردهی اولیه آرایه‌ها با مقادیر پیش‌فرض
   */
  void initializeArrays() {
    // پین‌های ردیف (مقادیر پیش‌فرض)
    const byte defaultRowPins[] = {9, 8, 7, 6, 5, 4, 3, 2};
    const byte defaultColPins[] = {A3, A2, A1, A0, 13, 12, 11, 10};
    
    // کپی کردن با بررسی محدوده
    for (int i = 0; i < KEYPAD_ROWS && i < 8; i++) {
      rowPins[i] = defaultRowPins[i];
    }
    for (int i = 0; i < KEYPAD_COLS && i < 8; i++) {
      colPins[i] = defaultColPins[i];
    }
    
    // ماتریس کلیدهای پیش‌فرض
    const char defaultKeys[4][4] = {
      {'1','2','3','A'},
      {'4','5','6','B'},
      {'7','8','9','C'},
      {'*','0','#','D'}
    };
    
    // کپی کردن ماتریس کلیدها
    for (int row = 0; row < KEYPAD_ROWS && row < 4; row++) {
      for (int col = 0; col < KEYPAD_COLS && col < 4; col++) {
        keys[row][col] = defaultKeys[row][col];
      }
    }
  }
  
  /**
   * بررسی صحت پین‌ها
   */
  bool validatePins() {
    // بررسی محدوده پین‌ها
    for (int i = 0; i < KEYPAD_ROWS; i++) {
      if (rowPins[i] > 53) return false; // Arduino Mega max
    }
    for (int i = 0; i < KEYPAD_COLS; i++) {
      if (colPins[i] > 53) return false;
    }
    
    // بررسی تداخل پین‌ها
    for (int i = 0; i < KEYPAD_ROWS; i++) {
      for (int j = 0; j < KEYPAD_COLS; j++) {
        if (rowPins[i] == colPins[j]) return false;
      }
    }
    
    return true;
  }
  
  /**
   * تست سخت‌افزار کی‌پد
   */
  bool testHardware() {
    if (!keypad) return false;
    
    // تست اساسی دسترسی به کی‌پد
    try {
      KeyState state = keypad->getState();
      (void)state; // جلوگیری از هشدار unused variable
      return true;
    } catch (...) {
      return false;
    }
  }
  
  /**
   * بررسی سلامت سخت‌افزار
   */
  bool checkHardware() {
    if (!isInitialized || !hardwareWorking || !keypad) {
      return false;
    }
    
    // بررسی overflow در زمان
    unsigned long currentTime = millis();
    if (currentTime < lastKeyTime) { // overflow
      lastKeyTime = currentTime;
    }
    
    return true;
  }
  
  /**
   * پاکسازی منابع
   */
  void cleanup() {
    isInitialized = false;
    hardwareWorking = false;
    
    if (keypad) {
      delete keypad;
      keypad = nullptr;
    }
    
    lastValidKey = NO_KEY;
    lastKeyTime = 0;
  }
};

#endif