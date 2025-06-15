/**
   * Check for numeric keys
   * @param key The key to check
   * @return true if the key is numeric
   */
  bool isNumericKey(char key) const {
    return (key >= '0' && key <= '9');
  }
  
  /**
   * Check for letter keys
   * @param key The key to check
   * @return true if the key is a letter
   */
  bool isLetterKey(char key) const {
    return (key >= 'A' && key <= 'D');
  }
  
  /**
   * Check for special keys
   * @param key The key to check
   * @return true if the key is special
   */
  bool isSpecialKey(char key) const {
    return (key == '*' || key == '#');
  }
  
  /**
   * Check for valid key
   * @param key The key to check
   * @return true if the key is valid
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
   * Convert key to number
   * @param key The key to convert
   * @return Corresponding number or -1 on error
   */
  int keyToNumber(char key) const {
    if (isNumericKey(key)) {
      return key - '0';
    }
    return -1;
  }
  
  /**
   * Check initialization status
   * @return true if keypad is initialized
   */
  bool isReady() const {
    return isInitialized && hardwareWorking && keypad != nullptr;
  }
  
  /**
   * Full keypad reset
   */
  bool reset() {
    if (!keypad) return false;
    
    try {
      // Clear internal state
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
   * Recover from hardware error
   */
  bool recover() {
    cleanup();
    delay(100);
    return init();
  }
  
private:
  /**
   * Initialize arrays with default values
   */
  void initializeArrays() {
    // Row pins (default values)
    const byte defaultRowPins[] = {9, 8, 7, 6, 5, 4, 3, 2};
    const byte defaultColPins[] = {A3, A2, A1, A0, 13, 12, 11, 10};
    
    // Copy with boundary check
    for (int i = 0; i < KEYPAD_ROWS && i < 8; i++) {
      rowPins[i] = defaultRowPins[i];
    }
    for (int i = 0; i < KEYPAD_COLS && i < 8; i++) {
      colPins[i] = defaultColPins[i];
    }
    
    // Default key matrix
    const char defaultKeys[4][4] = {
      {'1','2','3','A'},
      {'4','5','6','B'},
      {'7','8','9','C'},
      {'*','0','#','D'}
    };
    
    // Copy key matrix
    for (int row = 0; row < KEYPAD_ROWS && row < 4; row++) {
      for (int col = 0; col < KEYPAD_COLS && col < 4; col++) {
        keys[row][col] = defaultKeys[row][col];
      }
    }
  }
  
  /**
   * Validate pins
   */
  bool validatePins() {
    // Check pin range
    for (int i = 0; i < KEYPAD_ROWS; i++) {
      if (rowPins[i] > 53) return false; // Arduino Mega max
    }
    for (int i = 0; i < KEYPAD_COLS; i++) {
      if (colPins[i] > 53) return false;
    }
    
    // Check for pin conflicts
    for (int i = 0; i < KEYPAD_ROWS; i++) {
      for (int j = 0; j < KEYPAD_COLS; j++) {
        if (rowPins[i] == colPins[j]) return false;
      }
    }
    
    return true;
  }
  
  /**
   * Test keypad hardware
   */
  bool testHardware() {
    if (!keypad) return false;
    
    // Basic test for keypad access
    try {
      KeyState state = keypad->getState();
      (void)state; // Prevent unused variable warning
      return true;
    } catch (...) {
      return false;
    }
  }
  
  /**
   * Check hardware health
   */
  bool checkHardware() {
    if (!isInitialized || !hardwareWorking || !keypad) {
      return false;
    }
    
    // Check for time overflow
    unsigned long currentTime = millis();
    if (currentTime < lastKeyTime) { // overflow
      lastKeyTime = currentTime;
    }
    
    return true;
  }
  
  /**
   * Clean up resources
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