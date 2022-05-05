/* The main run file for the M5 Stack device in the IoT Thermo project.
  This program will connect the M5 Stack to AWS MQTT on the configured certificates,
  read the sensors for desired values (humidity, temperature, moisture), and publish
  those values plus a timestamp onto MQTT. This runs off the Arduino Framework.

  Functions:
  setup() - standard startup function for M5Stack using Arduino Framework. runs once
  loop() - standard looping function for M5Stack using Arduion Framework. runs infinitely
  env_sensor(float*, float*) - retrieves temperature (C) and humidity (%) values from ENV III sensor
  earth_sensor() - retrieves moisture (%) value from EARTH sensor
*/

#include <M5Core2.h>
#include <Arduino.h>
#include <Wire.h>
#include <UNIT_ENV.h>
#include "wifiSetup.h"

SHT3X sht30;
QMP6988 qmp6988;
wific wifi;

const char* TAG = "MAIN";
const char* ROOM_NAME = "Office";
const int SEND_INTERVAL = 10000;
bool pauseConnect = false;
long unsigned int currentMillis;
int dryTime;

bool env_sensor(float*, float*);
float earth_sensor();


// initial setup loop in Arduino Framework. runs once on startup
void setup() {  
  // setup for terminal monitoring
  Serial.begin(CONFIG_MONITOR_BAUD);
  delay(500);

  // setup for M5stack
  M5.begin(true,false,true,true);
  M5.Lcd.setTextColor(BLUE);
  M5.Lcd.setTextSize(2);

  // initialize wifi/MQTT/NTP connections
  wifi.initWifi();

  // begin ENV III sensor
  qmp6988.init();

  // setup EARTH sensor
  pinMode(26, INPUT);
  dacWrite(25,0);

  currentMillis = millis();
  dryTime = 0;
}


// run loop for Arduino Framework, runs continuously while m5 is on
void loop() {
  // update button feedback at beginning of loop
  M5.update();

  // set up loop variables
  float temp, humidity, pressure, rh;
  StaticJsonDocument<JSON_OBJECT_SIZE(6)> jdata;
  String jout;

  // pause m5stack from sending sensor info
  if (M5.BtnC.wasPressed())
  {
    // resume sending if currently paused, pause sending if currently resumed
    pauseConnect = !pauseConnect;
    if (pauseConnect)
    {
      M5.Lcd.clear();
      M5.Lcd.setCursor(0,0);
      M5.Lcd.printf("Sensor reading and connection paused.\n");
    }
    else
    {
      M5.Lcd.printf("Resuming connection...");
    }
  }

  // if the time interval has passed, and device is sending, send data
  if (((millis() - currentMillis) >= SEND_INTERVAL) && !pauseConnect)
  {
    currentMillis = millis();
    // attempt to retrieve ENV III sensor info
    bool sensor_success = env_sensor(&temp, &humidity, &pressure);

    // retrieve earth_sensor info
    rh = earth_sensor();

    // reset or increment dryTime
    if (rh < 80)
    {
        dryTime++;
    }
    else 
    {
      dryTime = 0;
    }

    // skip loop if sensor information wasn't retrieved
    if (sensor_success)
    {
      // print status to screen
      M5.Lcd.clear();
      M5.Lcd.setCursor(0,0);
      long unsigned int timestamp = wifi.getTimestamp();
      M5.Lcd.printf("Temperature: %2.2f*C  \nHumidity: %0.2f%%  \nPressure: %0.2fPa\r\n", temp, humidity, pressure);
      M5.Lcd.printf("Moisture: %f\n", rh);

      // serialize data into JSON
      jdata["Time"].set(timestamp);
      jdata["Room"].set(ROOM_NAME);
      jdata["Temperature"].set(temp);
      jdata["Humidity"].set(humidity);
      jdata["Moisture"].set(rh);
      jdata["DryTime"].set(dryTime);

      serializeJson(jdata, jout); 

      // publish data to MQTT
      wifi.checkMQTTConnect();
      wifi.publish(jout.c_str());
    }
  }
  
  // if time interval has passed, and sending is paused, ping connection to keep connected to MQTT
  else if (((millis() - currentMillis) >= SEND_INTERVAL))
  {
    currentMillis = millis();
    wifi.checkMQTTConnect();
  }
}


// retrieves sensor data from env unit sensor: temperature, humidity
// INPUTS: temp - temperature variable to store temperature value
//         humidity - humidity variable to store humidity value
// OUTPUTS: temp, humidity
bool env_sensor(float *temp, float *humidity)
{
  // check if sht30 is working. print error if else.
  if (sht30.get() != 0) {
    M5.Lcd.printf("%s: Error - sht30.get() failed", TAG);
    qmp6988.softwareReset();
    return false;
  }

  // read temp and humidity values using UNIT_ENV library
  *temp = sht30.cTemp;
  *humidity = sht30.humidity; 

  return true;
}


// retrieves sensor data from EARTH sensor unit: moisture (relative humidity)
// INPUTS: N/A
// OUTPUT: percent (float) - a percent value of the moisture reading
float earth_sensor()
{
  // read in RH from sensor as analog value
  float percent;
  uint16_t anaRead = analogRead(36);
  percent = map(anaRead, 4095, 1495, 0, 100); 

  // if percent is calculated above 100% 
  // this can happen if in contact with highly conductive material, i.e. water, metals
  if (percent > 100)
  {
    percent = 100;
  }

  return percent;
}
