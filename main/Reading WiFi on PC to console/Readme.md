@@ -1,16 +1,18 @@
#include <iostream>
#include <cstdio>

void displayFormatted(char* data) {
#void (displayFormatted)(char* data) 
{
    // Assume data is like: GAS:123,TEMP:30,S3:500
    printf("[Sensor Readings]\n------------------\n%s\n\n", data);
}

int main() {
#int main() {
    // Sample sensor data
    char sensorData[] = "GAS:123,TEMP:30,S3:500";
    #char sensorData[] = "GAS:123,TEMP:30,S3:500";

    displayFormatted(sensorData);
    displayFormatted(sensorData);}
{

    return 0;
}
