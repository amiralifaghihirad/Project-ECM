// Define the analog pin connected to the gas sensor
#define GAS_SENSOR A0

// Define the analog pin connected to the temperature sensor
#define TEMP_SENSOR A1

// Define the analog pin connected to the third sensor (could be any other sensor)
#define SENSOR_3 A2

// Create a structure to store readings from all three sensors
struct SensorData {
    int gas;      // Value read from the gas sensor
    int temp;     // Value read from the temperature sensor
    int sensor3;  // Value read from the third sensor
};

// This function reads analog values from the sensors and returns them as a SensorData structure
SensorData readSensors() {
    SensorData data;  // Create a variable to hold the sensor readings

    // Read the analog value from the gas sensor and store it in the structure
    data.gas = analogRead(GAS_SENSOR);

    // Read the analog value from the temperature sensor and store it in the structure
    data.temp = analogRead(TEMP_SENSOR);

    // Read the analog value from the third sensor and store it in the structure
    data.sensor3 = analogRead(SENSOR_3);

    // Return the structure containing all sensor values
    return data;
}
