
// برنامه چشمک زن LED برای Arduino به زبان C++
void setup() {
  pinMode(13, OUTPUT); // تنظیم پایه 13 به عنوان خروجی (معمولاً LED داخلی)
}

void loop() {
  digitalWrite(13, HIGH); // روشن کردن LED
  delay(1000);            // صبر به مدت ۱ ثانیه
  digitalWrite(13, LOW);  // خاموش کردن LED
  delay(1000);            // صبر به مدت ۱ ثانیه
}
