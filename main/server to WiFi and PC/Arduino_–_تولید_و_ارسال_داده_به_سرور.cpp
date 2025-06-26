// Create sample JSON data
  String sensorData = "{"value":" + String(random(20, 100)) + "}";
  webSocket.sendTXT(sensorData);

  delay(2000);   // Send every 2 seconds

}