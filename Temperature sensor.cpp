#include <DHT.h>

// --- Configuration Constants ---
#define DHTPIN 2	  // Pin where DHT11 is connected
#define DHTTYPE DHT11 // DHT sensor type

const int READ_INTERVAL = 2000; // Time between readings (ms)
const float TEMP_MIN = -10.0;	// Minimum valid temperature (°C)
const float TEMP_MAX = 60.0;	// Maximum valid temperature (°C)
const float HUM_MIN = 0.0;		// Minimum valid humidity (%)
const float HUM_MAX = 100.0;	// Maximum valid humidity (%)
const uint8_t DECIMALS = 1;		// Decimal places in output

DHT dht(DHTPIN, DHTTYPE);
unsigned long previousMillis = 0;

void setup()
{
	Serial.begin(9600);

	// Wait for Serial connection on boards like Leonardo, Zero, Due
#if defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_AVR_LEONARDO)
	while (!Serial);
#endif

	dht.begin(); // Initialize the DHT sensor
}

void loop()
{
	unsigned long currentMillis = millis();
	if (currentMillis - previousMillis >= READ_INTERVAL)
	{
		previousMillis = currentMillis;

		float temp = dht.readTemperature(); // Read temperature in Celsius
		float hum = dht.readHumidity();		// Read relative humidity

		// Validate sensor readings
		if (isnan(temp) || isnan(hum))
		{
			Serial.println("{\"error\":\"DHT read failed\"}");
			return;
		}

		// Check if values are within expected range
		bool tempOutOfRange = (temp < TEMP_MIN || temp > TEMP_MAX);
		bool humOutOfRange = (hum < HUM_MIN || hum > HUM_MAX);

		// Start JSON output
		if (tempOutOfRange || humOutOfRange)
		{
			Serial.print("{\"warning\":\"Out of range\",");
		}
		else
		{
			Serial.print("{");
		}

		// Print values with limited decimal precision
		Serial.print("\"temp\":");
		Serial.print(temp, DECIMALS);
		Serial.print(",\"hum\":");
		Serial.print(hum, DECIMALS);
		Serial.println("}");
	}
}
