#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

// پین‌های LCD
#define LCD_RS 12
#define LCD_EN 11
#define LCD_D4 5
#define LCD_D5 4
#define LCD_D6 3
#define LCD_D7 2

// پین‌های Keypad
#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4
#define ROW_PINS {9, 8, 7, 6}
#define COL_PINS {A3, A2, A1, A0}

// پین چراغ خطر
#define WARNING_LED_PIN 13

// تنظیمات زمانی
#define UPDATE_INTERVAL 2000  // 2 ثانیه
#define LED_BLINK_INTERVAL 500 // نیم ثانیه

// تنظیمات نمایشگر
#define LCD_COLS 16
#define LCD_ROWS 2

// کلید خالی
#define NO_KEY '\0'

#endif