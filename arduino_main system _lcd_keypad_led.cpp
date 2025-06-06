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
  
  // راه‌اندازی ماژول‌ها
  lcd.init();
  keypad.init();
  ledManager.init();
  
  // نمایش پیام خوش‌آمدگویی
  lcd.showWelcome();
  delay(2000);
  
  Serial.println("System Ready!");
}

void loop() {
  // بررسی کلیدها
  char key = keypad.getKey();
  if (key != NO_KEY) {
    handleKeyPress(key);
  }
  
  // بروزرسانی نمایشگر هر 2 ثانیه
  if (millis() - lastUpdate >= UPDATE_INTERVAL) {
    updateDisplay();
    lastUpdate = millis();
  }
  
  // مدیریت چراغ خطر
  ledManager.update();
  
  delay(50); // تاخیر کوچک برای پایداری
}

void handleKeyPress(char key) {
  Serial.print("Key pressed: ");
  Serial.println(key);
  
  switch (key) {
    case '1':
      systemActive = !systemActive;
      ledManager.setWarningState(systemActive);
      lcd.showStatus(systemActive ? "فعال" : "غیرفعال");
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
    // نمایش اطلاعات سیستم
    String info = "Time: " + String(millis()/1000) + "s";
    info += "\nTemp: " + String(random(20, 35)) + "C";
    info += "\nStatus: Running";
    
    lcd.showSystemData(info);
  }
}