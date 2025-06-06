#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>

const char* ssid = "YourSSID";
const char* password = "YourPassword";

WebSocketsClient webSocket;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  // مدیریت رویدادها
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);