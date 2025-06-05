  // ساخت داده JSON به صورت نمونه
  String sensorData = "{"value":" + String(random(20, 100)) + "}";
  webSocket.sendTXT(sensorData);

  delay(2000); // ارسال هر 2 ثانیه
}