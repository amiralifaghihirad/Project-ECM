// تعریف ثابت‌ها برای پین‌ها
const int SELECT_PIN_A = 2;
const int SELECT_PIN_B = 3;
const int SELECT_PIN_C = 4;
const int ANALOG_INPUT_PIN = A0;

// تنظیمات اولیه
void setup() {
  // بررسی اتصال سریال
  Serial.begin(9600);
  while (!Serial) {
    ; // صبر تا آماده شدن سریال (مخصوص Arduinoهایی مثل Leonardo)
  }

  // تنظیم پین‌ها به‌عنوان خروجی
  pinMode(SELECT_PIN_A, OUTPUT);
  pinMode(SELECT_PIN_B, OUTPUT);
  pinMode(SELECT_PIN_C, OUTPUT);

  // اعلام شروع
  Serial.println("CD4051 Multiplexer Initialized.");
}

// تابع انتخاب کانال بین 0 تا 7
void selectChannel(byte channel) {
  if (channel > 7) return; // جلوگیری از مقادیر نامعتبر

  digitalWrite(SELECT_PIN_A, bitRead(channel, 0));
  digitalWrite(SELECT_PIN_B, bitRead(channel, 1));
  digitalWrite(SELECT_PIN_C, bitRead(channel, 2));
}

// تابع خواندن مقدار آنالوگ با بررسی خطا
int readAnalogSafe(int pin) {
  int value = analogRead(pin);
  if (value < 0 || value > 1023) {
    Serial.println("خطا در خواندن مقدار آنالوگ!");
    return -1; // مقدار نامعتبر
  }
  return value;
}

// حلقه اصلی
void loop() {
  for (byte channel = 0; channel < 8; channel++) {
    selectChannel(channel);
    delay(20); // افزایش تأخیر برای تثبیت بهتر سیگنال

    int gasValue = readAnalogSafe(ANALOG_INPUT_PIN);
    if (gasValue >= 0) {
      Serial.print("MQ-9 [Y");
      Serial.print(channel);
      Serial.print("] مقدار: ");
      Serial.println(gasValue);
    }
  }

  Serial.println("-------------");
  delay(2000); // تأخیر بین اسکن کامل کانال‌ها
}