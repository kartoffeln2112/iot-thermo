#include <M5Core2.h>
#include <Arduino.h>
#include <Wire.h>
#include <UNIT_ENV.h>

#include "wifiSetup.h"

SHT3X sht30;
QMP6988 qmp6988;
wific wifi;


const char* TAG = "MAIN";
const char* ROOM_NAME = "room1";
char draw_string[1024];

bool env_sensor(float*, float*, float*);
void earth_sensor(int*, uint16_t*);


void setup() {  
  // setup for terminal monitoring
  Serial.begin(CONFIG_MONITOR_BAUD);
  delay(500);

  // setup for M5stack
  M5.begin(true,false,true,true);
  M5.Lcd.setTextColor(BLUE);
  M5.Lcd.setTextSize(2);

  // initialize wifi/MQTT connections
  wifi.initWifi();

  // begin ENV III sensor
  qmp6988.init();

  // setup EARTH sensor
  pinMode(26, INPUT);
  dacWrite(25,0);
}

void loop() {
  uint16_t anaRead;
  int digiRead;
  float temp, humidity, pressure;
  StaticJsonDocument<JSON_OBJECT_SIZE(5)> jdata;
  String jout;
  
  // attempt to retrieve ENV III sensor info
  bool sensor_success = env_sensor(&temp, &humidity, &pressure);

  // retrieve earth_sensor info
  earth_sensor(&digiRead, &anaRead);

  // skip loop if sensor information wasn't retrieved
  if (sensor_success)
  {
    // serialize data into JSON
    jdata["Room"].set(ROOM_NAME);
    jdata["Temperature"].set(temp);
    jdata["Humidity"].set(humidity);
    jdata["Pressure"].set(pressure);
    jdata["Moisture"].set(anaRead);

    serializeJson(jdata, jout);

    // print status to screen
    M5.Lcd.clear();
    M5.Lcd.setCursor(0,0);
    M5.Lcd.printf("Temperature: %2.2f*C  \nHumidity: %0.2f%%  \nPressure: %0.2fPa\r\n", temp, humidity, pressure);
    M5.Lcd.printf("Analog Read: %d\nDigital Read: %d", anaRead, digiRead);

    wifi.checkMQTTConnect();
    wifi.publish(jout.c_str());

    delay(1000);
  }

}


bool env_sensor(float *temp, float *humidity, float *pressure)
{
  if (sht30.get() != 0) {
    M5.Lcd.printf("%s: Error - sht30.get() failed", TAG);
    qmp6988.softwareReset();
    return false;
  }

  *temp = sht30.cTemp;
  *humidity = sht30.humidity;
  *pressure = qmp6988.calcPressure();  

  return true;
}


void earth_sensor(int *digiRead, uint16_t *anaRead)
{
  *anaRead = analogRead(36);
  *digiRead = digitalRead(26);

  return;
}