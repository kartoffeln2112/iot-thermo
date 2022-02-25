#include "Wire.h"
#include "UNIT_ENV.h"
#include "wifiSetup.h"
#include "M5Core2.h"

SHT3X sht30;
QMP6988 qmp6988;

const char* TAG = "MAIN";
const char* ROOM_NAME = "room1";
char draw_string[1024];

void env_sensor(float*, float*, float*);


void setup() {  
  M5.begin(true,false,true,true);
  M5.Lcd.setTextColor(BLUE);
  M5.Lcd.setTextSize(2);
  
  initWifi();

  //Wire.begin(21, 22);
  qmp6988.init();
}

void loop() {
  float temp, humidity, pressure;
  StaticJsonDocument<JSON_OBJECT_SIZE(4)> jdata;
  String jout;

  env_sensor(&temp, &humidity, &pressure);

  jdata["Room"].set(ROOM_NAME);
  jdata["Temperature"].set(temp);
  jdata["Humidity"].set(humidity);
  jdata["Pressure"].set(pressure);

  serializeJson(jdata, jout);

  M5.Lcd.clear();
  M5.Lcd.setCursor(0,0);
  M5.Lcd.printf("Temperature: %2.2f*C  \nHumidity: %0.2f%%  \nPressure: %0.2fPa\r\n", temp, humidity, pressure);
  vTaskDelay(5000);
}

void env_sensor(float *temp, float *humidity, float *pressure)
{
  if (sht30.get() != 0) {
    M5.Lcd.printf("%s: Error - sht30.get() failed", TAG);
    return;
  }

  *temp = sht30.cTemp;
  *humidity = sht30.humidity;
  *pressure = qmp6988.calcPressure();  

  return;
}
