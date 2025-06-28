#define GAS_SENSOR A0
#define TEMP_SENSOR A1
#define SENSOR_3 A2

const int NUM_SAMPLES = 20; // Increased samples for better noise reduction

// --- Calibration and Validation Constants ---
// For MQ-x Gas Sensor (adjust based on your specific sensor and calibration)
const float GAS_VCC = 5.0;            // Operating voltage for sensor
const float GAS_RL = 10.0;            // Load resistance (KOhms), typically 10K for MQ series
const float GAS_RO_IN_AIR = 10.0;     // Calibrated Ro in clean air (KOhms) - MEASURE THIS FOR ACCURACY!
                                      // This is Rs in clean air for your specific sensor module.
const float GAS_CURVE_PPM_AT_1 = 20.0; // PPM value when Rs/Ro = 1 (approx)
const float GAS_CURVE_SLOPE = -0.3;   // Slope of the log(Rs/Ro) vs log(PPM) curve (from datasheet)

// For LM35 Temperature Sensor
// No specific calibration needed beyond standard conversion if 5V reference is accurate

// For Soil Moisture Sensor (Resistive type - adjust based on your sensor and calibration)
const int SOIL_DRY_VALUE = 800;   // Analog value when sensor is completely dry (e.g., in air)
const int SOIL_WET_VALUE = 300;   // Analog value when sensor is in water or completely saturated

// --- Raw ADC Value Validation Ranges (for detecting disconnected/faulty sensors more robustly) ---
// These ranges should be determined by testing your actual sensors when connected and disconnected.
const int GAS_MIN_VALID_RAW = 50;  // Minimum expected raw value when sensor is connected and working
const int GAS_MAX_VALID_RAW = 950; // Maximum expected raw value

const int TEMP_MIN_VALID_RAW = 100; // Minimum expected raw value (e.g., 10C)
const int TEMP_MAX_VALID_RAW = 600; // Maximum expected raw value (e.g., 60C)

const int SENSOR3_MIN_VALID_RAW = 10;  // Minimum expected raw value (e.g., sensor in water)
const int SENSOR3_MAX_VALID_RAW = 990; // Maximum expected raw value (e.g., sensor in dry air)


// Structure to hold sensor readings
struct SensorData {
    float gasPPM;   // Gas concentration in PPM (parts per million)
    float tempC;    // Temperature in Celsius
    float soilMoisturePercent; // Soil moisture in percentage (0-100%)
    bool gasConnected; // Indicates if gas sensor is likely connected and working
    bool tempConnected; // Indicates if temp sensor is likely connected and working
    bool sensor3Connected; // Indicates if sensor3 is likely connected and working
};

// Function to read and average multiple analog readings with value validation
int averageAnalogRead(int pin) {
    long sum = 0;
    int validReadings = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        int value = analogRead(pin);
        // Only sum valid readings within the standard ADC range
        if (value >= 0 && value <= 1023) {
            sum += value;
            validReadings++;
        }
        delay(2); // Small delay for stable readings (reduced from 5ms for faster reads)
    }
    if (validReadings > 0) {
        return sum / validReadings; // Return average of valid readings
    } else {
        return -1; // Indicate no valid readings (e.g., pin disconnected)
    }
}

// Function to calculate sensor resistance from ADC value for MQ sensors
float calculateRs(int rawADC) {
    if (rawADC == 0) return 10000000.0; // Avoid division by zero, return very high resistance
    float VRL = (float)rawADC * (GAS_VCC / 1023.0); // Voltage across the load resistor
    float Rs = ((GAS_VCC - VRL) / VRL) * GAS_RL; // Sensor resistance (KOhms)
    return Rs;
}

// Function to read all sensors with enhanced validation and calibration
SensorData readSensors() {
    SensorData data;

    // --- Read and validate Gas Sensor ---
    int gasRaw = averageAnalogRead(GAS_SENSOR);
    if (gasRaw != -1 && gasRaw >= GAS_MIN_VALID_RAW && gasRaw <= GAS_MAX_VALID_RAW) {
        data.gasConnected = true;
        float Rs = calculateRs(gasRaw);
        float Rs_Ro_ratio = Rs / GAS_RO_IN_AIR;

        // Using a simplified log-log model: log(PPM) = m * log(Rs/Ro) + b
        // b = log(PPM when Rs/Ro = 1)
        // For example, if PPM = 20 when Rs/Ro = 1, then log(20) = log_base_10(20) = 1.30
        // float b = log10(GAS_CURVE_PPM_AT_1);
        // data.gasPPM = pow(10, (GAS_CURVE_SLOPE * log10(Rs_Ro_ratio) + b));

        // A simpler inverse relationship for demonstration, adjust for actual MQ sensor curve
        // This is a highly simplified model and needs refinement based on actual datasheet curve.
        // For example, MQ-2 for LPG/Propane: ratio of 0.6 -> 200 PPM, 1.0 -> 1000 PPM, 2.0 -> 10000 PPM
        if (Rs_Ro_ratio > 0) {
             data.gasPPM = pow(10, (log10(Rs_Ro_ratio) * GAS_CURVE_SLOPE) + log10(GAS_CURVE_PPM_AT_1));
        } else {
            data.gasPPM = 0; // Or a specific error value
        }


    } else {
        data.gasConnected = false;
        data.gasPPM = -1.0; // Invalid/Not Connected
    }

    // --- Read and convert Temperature Sensor value ---
    int tempRaw = averageAnalogRead(TEMP_SENSOR);
    if (tempRaw != -1 && tempRaw >= TEMP_MIN_VALID_RAW && tempRaw <= TEMP_MAX_VALID_RAW) {
        data.tempConnected = true;
        // LM35 (10mV per °C), and 5V reference (1024 steps -> 5V/1024 steps/bit)
        data.tempC = tempRaw * (500.0 / 1024.0); // 500.0 because 5V = 5000mV, and 10mV/C, so 5000/10 = 500
    } else {
        data.tempConnected = false;
        data.tempC = -1000.0; // Invalid/Not Connected flag
    }

    // --- Read and validate Third Sensor (Soil Moisture) ---
    int sensor3Raw = averageAnalogRead(SENSOR_3);
    if (sensor3Raw != -1 && sensor3Raw >= SENSOR3_MIN_VALID_RAW && sensor3Raw <= SENSOR3_MAX_VALID_RAW) {
        data.sensor3Connected = true;
        // Map the raw sensor value to a percentage (0-100%)
        // Note: For resistive soil moisture sensors, higher ADC value usually means dryer soil.
        // The map function handles this correctly by reversing the min/max if needed.
        data.soilMoisturePercent = map(sensor3Raw, SOIL_DRY_VALUE, SOIL_WET_VALUE, 0, 100);

        // Clamp values to ensure they stay within 0-100% range
        if (data.soilMoisturePercent < 0) data.soilMoisturePercent = 0;
        if (data.soilMoisturePercent > 100) data.soilMoisturePercent = 100;

    } else {
        data.sensor3Connected = false;
        data.soilMoisturePercent = -1.0; // Invalid/Not Connected
    }

    return data;
}

// Arduino setup
void setup() {
    Serial.begin(9600);
}

// Arduino loop
void loop() {
    SensorData readings = readSensors();

    // Print Gas sensor values
    if (readings.gasConnected)
        Serial.print("Gas: "), Serial.print(readings.gasPPM, 2), Serial.print(" PPM");
    else
        Serial.print("Gas: NOT CONNECTED / ERROR");

    Serial.print(" | ");

    // Print Temperature sensor values
    if (readings.tempConnected)
        Serial.print("Temp (C): "), Serial.print(readings.tempC, 2);
    else
        Serial.print("Temp: NOT CONNECTED / ERROR");

    Serial.print(" | ");

    // Print Sensor3 (Soil Moisture) values
    if (readings.sensor3Connected)
        Serial.print("Soil Moisture: "), Serial.print(readings.soilMoisturePercent, 2), Serial.println("%");
    else
        Serial.println("Soil Moisture: NOT CONNECTED / ERROR");

    delay(2000); // Delay between readings (increased for clarity)
}
