#define GAS_SENSOR A0
#define TEMP_SENSOR A1
#define SENSOR_3 A2

const int NUM_SAMPLES = 10; // Number of samples for averaging

// Structure to hold sensor readings
struct SensorData {
    int gas;        // Raw gas sensor value
    float tempC;    // Temperature in Celsius
    int sensor3;    // Raw value from the third sensor
};

// Function to read and average multiple analog readings
int averageAnalogRead(int pin) {
    long sum = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        int value = analogRead(pin);
        if (value >= 0 && value <= 1023) // Valid range check
            sum += value;
        delay(5); // Small delay for stable readings
    }
    return sum / NUM_SAMPLES;
}

// Function to read all sensors
SensorData readSensors() {
    SensorData data;

    // Read and validate gas sensor
    int gasRaw = averageAnalogRead(GAS_SENSOR);
    data.gas = (gasRaw > 0) ? gasRaw : -1; // -1 indicates sensor not connected

    // Read and convert temperature sensor value
    int tempRaw = averageAnalogRead(TEMP_SENSOR);
    if (tempRaw > 0) {
        // Assuming LM35 (10mV per °C), and 5V reference
        data.tempC = tempRaw * (5.0 / 1023.0) * 100.0;
    } else {
        data.tempC = -1000.0; // Invalid flag
    }

    // Read and validate third sensor
    int sensor3Raw = averageAnalogRead(SENSOR_3);
    data.sensor3 = (sensor3Raw > 0) ? sensor3Raw : -1;

    return data;
}

// Arduino setup
void setup() {
    Serial.begin(9600);
}

// Arduino loop
void loop() {
    SensorData readings = readSensors();

    // Print sensor values
    if (readings.gas != -1)
        Serial.print("Gas: "), Serial.print(readings.gas);
    else
        Serial.print("Gas: Not Connected");

    Serial.print(" | ");

    if (readings.tempC > -100.0)
        Serial.print("Temp (C): "), Serial.print(readings.tempC);
    else
        Serial.print("Temp: Not Connected");

    Serial.print(" | ");

    if (readings.sensor3 != -1)
        Serial.print("Sensor3: "), Serial.println(readings.sensor3);
    else
        Serial.println("Sensor3: Not Connected");

    delay(1000); // Delay between readings
}