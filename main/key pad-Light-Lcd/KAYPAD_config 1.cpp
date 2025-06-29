```cpp
#ifndef KEYPAD_MANAGER_H
#define KEYPAD_MANAGER_H

#include <Keypad.h>
#include "system_config.h"

// Checking for the existence of necessary definitions
#ifndef KEYPAD_ROWS
#error "KEYPAD_ROWS must be defined in system_config.h"
#endif
#ifndef KEYPAD_COLS
#error "KEYPAD_COLS must be defined in system_config.h"
#endif

// Validation of values
#if KEYPAD_ROWS < 1 || KEYPAD_ROWS > 8
#error "KEYPAD_ROWS must be between 1 and 8"
#endif
#if KEYPAD_COLS < 1 || KEYPAD_COLS > 8
#error "KEYPAD_COLS must be between 1 and 8"
#endif

class KeypadManager {
private:
  // Defining pin arrays with safety checks
  byte rowPins[KEYPAD_ROWS];
  byte colPins[KEYPAD_COLS];
  
  // Key matrix
  char keys[KEYPAD_ROWS][KEYPAD_COLS];
  
  Keypad* keypad;
  bool isInitialized;
  bool hardwareWorking;
  unsigned long lastKeyTime;
  char lastValidKey;
  
  static const unsigned long DEBOUNCE_DELAY = 50; // milliseconds
  static const unsigned long MAX_TIMEOUT = 300000UL; // 5 minutes maximum
  
public:
  /**
   * KeypadManager class constructor
   */
  KeypadManager() : keypad(nullptr), isInitialized(false), hardwareWorking(false), 
                    lastKeyTime(0), lastValidKey(NO_KEY) {
    initializeArrays();
  }
  
  /**
   * Destructor
   */
  ~KeypadManager() {
    if (keypad) {
      delete keypad;
    }
  }
  
  /**
   * Initialize keypad with full validation
   * @return true if successful
   */
  bool init() {
    try {
      // Pin validation
      if (!validatePins()) {
        return false;
      }
      
      // Create keymap
      char* keymap = makeKeymap((char*)keys);
      if (!keymap) {
        return false;
      }
      
      // Create Keypad object
      keypad = new Keypad(keymap, rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);
      if (!keypad) {
        return false;
      }
      
      // Initial test
      keypad->setDebounceTime(DEBOUNCE_DELAY);
      keypad->setHoldTime(500);
      
      // Hardware test
      if (!testHardware()) {
        delete keypad;
        keypad = nullptr;
        return false;
      }
      
      isInitialized = true;
      hardwareWorking = true;
      lastKeyTime = millis();
      
      // Success message only if Serial is active
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
   * Get pressed key with error checking
   * @return Key character or NO_KEY
   */
  char getKey() {
    if (!checkHardware()) return NO_KEY;
    
    try {
      char key = keypad->getKey();
      
      if (key != NO_KEY) {
        // Debounce check
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
   * Check if a specific key is pressed
   * @param key The key to check
   * @return true if the key is pressed
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
   * Get keypad state
   * @return Current keypad state
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
   * Check if any key is pressed
   * @return true if any key is pressed
   */
  bool isAnyKeyPressed() {
    if (!checkHardware()) return false;
    
    char key = getKey();
    return (key != NO_KEY);
  }
  
  /**
   * Wait for key press (non-blocking) with overflow check
   * @param timeout Wait time (milliseconds)
   * @return Pressed key or NO_KEY if timeout
   */
  char waitForKeyTimeout(unsigned long timeout) {
    if (!checkHardware() || timeout == 0 || timeout > MAX_TIMEOUT) {
      return NO_KEY;
    }
    
    unsigned long startTime = millis();
    char key = NO_KEY;
    
    while (key == NO_KEY) {
      unsigned long currentTime = millis();
      
      // Overflow check
      unsigned long elapsed;
      if (currentTime >= startTime) {
        elapsed = currentTime - startTime;
      } else {
        // Overflow occurred
        elapsed = (ULONG_MAX - startTime) + currentTime + 1;
      }
      
      if (elapsed >= timeout) {
        break;
      }
      
      key = getKey();
      
      if (key == NO_KEY) {
        delay(10); // Adjustable delay
      }
    }
    
    return key;
  }