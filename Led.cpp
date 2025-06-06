#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <climits>

// ثابت‌های کمکی با اعتبارسنجی
#define MAX_STRING_TO_INT_LENGTH 10
#define MAX_TIME_BUFFER_SIZE 32  // افزایش اندازه برای اطمینان از فضای کافی
#define MIN_TIME_BUFFER_SIZE 20  // حداقل اندازه مورد نیاز

class Utils {
private:
  static bool isRandomInitialized;
  
  // بررسی اولیه‌سازی random
  static void ensureRandomInitialized() {
    if (!isRandomInitialized) {
      randomSeed(analogRead(0) + millis());
      isRandomInitialized = true;
    }
  }

public:
  /**
   * تبدیل میلی‌ثانیه به فرمت قابل خواندن
   * @param milliseconds مقدار میلی‌ثانیه
   * @param buffer بافر خروجی
   * @param bufferSize اندازه بافر
   * @return تعداد کاراکترهای نوشته شده یا -1 در صورت خطا
   */
  static int formatTime(unsigned long milliseconds, char* buffer, int bufferSize) {
    // بررسی ورودی‌ها
    if (buffer == nullptr || bufferSize < MIN_TIME_BUFFER_SIZE) {
      return -1;
    }
    
    // مقداردهی اولیه بافر
    buffer[0] = '\0';
    
    unsigned long seconds = milliseconds / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    
    seconds = seconds % 60;
    minutes = minutes % 60;
    
    int written = 0;
    int remaining = bufferSize - 1; // فضای باقی‌مانده (منهای null terminator)
    
    // بررسی سرریز در هر مرحله
    if (hours > 0 && remaining > 0) {
      int temp = snprintf(buffer + written, remaining, "%luh ", hours);
      if (temp < 0 || temp >= remaining) {
        return -1; // خطای snprintf یا عدم فضای کافی
      }
      written += temp;
      remaining -= temp;
    }
    
    if (minutes > 0 && remaining > 0) {
      int temp = snprintf(buffer + written, remaining, "%lum ", minutes);
      if (temp < 0 || temp >= remaining) {
        return -1;
      }
      written += temp;
      remaining -= temp;
    }
    
    if (remaining > 0) {
      int temp = snprintf(buffer + written, remaining, "%lus", seconds);
      if (temp < 0 || temp >= remaining) {
        return -1;
      }
      written += temp;
    }
    
    return written;
  }
  
  /**
   * تولید رقم تصادفی در محدوده
   * @param min حداقل مقدار
   * @param max حداکثر مقدار
   * @return عدد تصادفی یا INT_MIN در صورت خطا
   */
  static int randomRange(int min, int max) {
    if (min > max) {
      return INT_MIN; // خطای ورودی
    }
    
    ensureRandomInitialized();
    return random(min, max + 1);
  }
  
  /**
   * بررسی محدوده عدد
   * @param value مقدار مورد بررسی
   * @param min حداقل مقدار
   * @param max حداکثر مقدار
   * @return true اگر در محدوده باشد
   */
  static bool isInRange(int value, int min, int max) {
    if (min > max) return false; // محدوده نامعتبر
    return (value >= min && value <= max);
  }
  
  /**
   * تبدیل رشته به عدد با بررسی خطا و محدودیت طول
   * @param str رشته ورودی
   * @param success پوینتر به متغیر موفقیت
   * @return عدد تبدیل شده یا 0 در صورت خطا
   */
  static long stringToInt(const char* str, bool* success) {
    if (success) *success = false;
    
    // بررسی ورودی‌های نامعتبر
    if (str == nullptr) {
      return 0;
    }
    
    size_t len = strlen(str);
    if (len == 0 || len > MAX_STRING_TO_INT_LENGTH) {
      return 0;
    }
    
    long result = 0;
    bool isNegative = false;
    int startIndex = 0;
    
    // بررسی علامت
    if (str[0] == '-') {
      isNegative = true;
      startIndex = 1;
      if (len == 1) return 0; // فقط علامت منفی
    } else if (str[0] == '+') {
      startIndex = 1;
      if (len == 1) return 0; // فقط علامت مثبت
    }
    
    // پردازش ارقام
    for (int i = startIndex; str[i] != '\0'; i++) {
      if (str[i] < '0' || str[i] > '9') {
        return 0; // کاراکتر غیرعددی
      }
      
      int digit = str[i] - '0';
      
      // بررسی دقیق‌تر سرریز
      if (isNegative) {
        if (result < (LONG_MIN + digit) / 10) {
          return 0; // سرریز منفی
        }
        result = result * 10 - digit;
      } else {
        if (result > (LONG_MAX - digit) / 10) {
          return 0; // سرریز مثبت
        }
        result = result * 10 + digit;
      }
    }
    
    if (success) *success = true;
    return result;
  }
  
  /**
   * محدود کردن عدد در بازه
   * @param value مقدار ورودی
   * @param min حداقل مقدار
   * @param max حداکثر مقدار
   * @return مقدار محدود شده یا value در صورت محدوده نامعتبر
   */
  static int constrainValue(int value, int min, int max) {
    if (min > max) return value; // محدوده نامعتبر
    return constrain(value, min, max);
  }
  
  /**
   * تبدیل درجه حرارت سلسیوس به فارنهایت
   * @param celsius درجه سلسیوس
   * @return درجه فارنهایت
   */
  static float celsiusToFahrenheit(float celsius) {
    // بررسی محدوده منطقی
    if (celsius < -273.15f || celsius > 10000.0f) {
      return NAN; // دمای غیرمنطقی
    }
    return (celsius * 9.0f / 5.0f) + 32.0f;
  }
  
  /**
   * تبدیل درجه حرارت فارنهایت به سلسیوس
   * @param fahrenheit درجه فارنهایت
   * @return درجه سلسیوس
   */
  static float fahrenheitToCelsius(float fahrenheit) {
    // بررسی محدوده منطقی
    if (fahrenheit < -459.67f || fahrenheit > 18000.0f) {
      return NAN; // دمای غیرمنطقی
    }
    return (fahrenheit - 32.0f) * 5.0f / 9.0f;
  }
  
  /**
   * بررسی گذشت زمان مشخص با در نظر گرفتن سرریز millis()
   * @param startTime زمان شروع
   * @param interval فاصله زمانی
   * @return true اگر زمان گذشته باشد
   */
  static bool hasTimePassed(unsigned long startTime, unsigned long interval) {
    if (interval == 0) return true; // فاصله صفر همیشه گذشته
    
    unsigned long currentTime = millis();
    
    // بررسی سرریز millis() (هر 49.7 روز)
    if (currentTime >= startTime) {
      return (currentTime - startTime >= interval);
    } else {
      // سرریز رخ داده
      return ((ULONG_MAX - startTime + currentTime + 1) >= interval);
    }
  }
  
  /**
   * تأخیر غیرمسدود کننده با بررسی سرریز
   * @param previousTime پوینتر به زمان قبلی
   * @param interval فاصله زمانی
   * @return true اگر زمان رسیده باشد
   */
  static bool nonBlockingDelay(unsigned long* previousTime, unsigned long interval) {
    if (previousTime == nullptr || interval == 0) {
      return false;
    }
    
    if (hasTimePassed(*previousTime, interval)) {
      *previousTime = millis();
      return true;
    }
    return false;
  }
  
  /**
   * محاسبه میانگین آرایه با بررسی سرریز
   * @param values آرایه مقادیر
   * @param length طول آرایه
   * @return میانگین یا NAN در صورت خطا
   */
  static float calculateAverage(const int values[], int length) {
    if (values == nullptr || length <= 0) {
      return NAN;
    }
    
    // بررسی محدودیت طول برای جلوگیری از سرریز
    if (length > 1000000) { // محدودیت منطقی
      return NAN;
    }
    
    long long sum = 0;
    
    for (int i = 0; i < length; i++) {
      // بررسی سرریز قبل از جمع
      if ((sum > 0 && values[i] > LLONG_MAX - sum) ||
          (sum < 0 && values[i] < LLONG_MIN - sum)) {
        return NAN; // سرریز در جمع
      }
      sum += values[i];
    }
    
    return (float)sum / length;
  }
  
  /**
   * بررسی معتبر بودن پوینتر و اندازه آرایه
   * @param ptr پوینتر
   * @param size اندازه
   * @return true اگر معتبر باشد
   */
  static bool isValidArray(const void* ptr, int size) {
    return (ptr != nullptr && size > 0);
  }
  
  /**
   * محاسبه حداکثر مقدار در آرایه
   * @param values آرایه مقادیر
   * @param length طول آرایه
   * @param maxValue پوینتر به متغیر حداکثر
   * @return true در صورت موفقیت
   */
  static bool findMaxValue(const int values[], int length, int* maxValue) {
    if (!isValidArray(values, length) || maxValue == nullptr) {
      return false;
    }
    
    *maxValue = values[0];
    for (int i = 1; i < length; i++) {
      if (values[i] > *maxValue) {
        *maxValue = values[i];
      }
    }
    return true;
  }
  
  /**
   * محاسبه حداقل مقدار در آرایه
   * @param values آرایه مقادیر
   * @param length طول آرایه
   * @param minValue پوینتر به متغیر حداقل
   * @return true در صورت موفقیت
   */
  static bool findMinValue(const int values[], int length, int* minValue) {
    if (!isValidArray(values, length) || minValue == nullptr) {
      return false;
    }
    
    *minValue = values[0];
    for (int i = 1; i < length; i++) {
      if (values[i] < *minValue) {
        *minValue = values[i];
      }
    }
    return true;
  }
};

// تعریف متغیر static
bool Utils::isRandomInitialized = false;

#endif // UTILS_H