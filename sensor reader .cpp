#define GAS_SENSOR A0
#define TEMP_SENSOR A1
#define SENSOR_3 A2

struct SensorData {
    int gas;
    int temp;
    int sensor3;
};

SensorData readSensors() {
    SensorData data;
    data.gas = analogRead(GAS_SENSOR);
    data.temp = analogRead(TEMP_SENSOR);
    data.sensor3 = analogRead(SENSOR_3);
    return data;
}