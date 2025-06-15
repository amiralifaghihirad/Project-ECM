#include "lcd_manager.h"
#include "keypad_manager.h" 
#include "led_manager.h"
#include "system_config.h"

LCDManager lcd;
KeypadManager keypad;
LEDManager ledManager;

unsigned long lastUpdate = 0;
String currentData = "";
bool systemActive = false;

void setup() {
  Serial.begin(9600);
  
  // Initialize modules
  lcd.init();
  keypad.init();
  ledManager.init();
  
  // Display welcome message
  lcd.showWelcome();
  delay(2000);
  
  Serial.println("System Ready!");
}

void loop() {
  // Check keys
  char key = keypad.getKey();
  if (key != NO_KEY) {
    handleKeyPress(key);
  }
  
  // Update display every 2 seconds
  if (millis() - lastUpdate >= UPDATE_INTERVAL) {
    updateDisplay();
    lastUpdate = millis();
  }
  
  // Manage warning light
  ledManager.update();
  
  delay(50); // Small delay for stability
}

void handleKeyPress(char key) {
  Serial.print("Key pressed: ");
  Serial.println(key);
  
  switch (key) {
    case '1':
      systemActive = !systemActive;
      ledManager.setWarningState(systemActive);
      lcd.showStatus(systemActive ? "Active" : "Inactive");
      break;
      
    case '2':
      lcd.showMenu();
      break;
      
    case '3':
      lcd.showSystemInfo();
      break;
      
    case '*':
      lcd.clear();
      break;
      
    case '#':
      lcd.showWelcome();
      break;
      
    default:
      currentData += key;
      lcd.showInput(currentData);
      break;
  }
}

void updateDisplay() {
  if (systemActive) {
    // Display system information
    String info = "Time: " + String(millis()/1000) + "s";
    info += "\nTemp: " + String(random(20, 35)) + "C";
    info += "\nStatus: Running";
    
    lcd.showSystemData(info);
  }
}
