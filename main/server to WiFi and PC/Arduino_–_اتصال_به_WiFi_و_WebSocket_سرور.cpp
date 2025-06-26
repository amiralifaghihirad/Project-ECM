  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected!");

// Enter the WebSocket server address
  webSocket.begin("192.168.1.100", 8080, "/");
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();