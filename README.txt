Built for the ESP32 Core2 system, using Arduino Framework on VSCode PlatformIO.
Libraries:
  Adafruit BMP280 Library v2.6.1
  Adafruit BusIO v1.11.1
  Adafruit Unified Sensor v1.1.4
  ArduinoJson v6.19.2
  M5Core2 v0.1.0
  PubSubClient v2.8
  UNIT_ENV v0.0.2
  
Uses ENV III Sensor (SHT30, QMP6988) to collect temperature, humidity, air pressure
data in the room and sends data to AWS MQTT in JSON format.

TO CONFIG - 
  Edit "ws.h.template" file and complete user data in order to run. 
  Save as "ws.h" in same folder (/include).
  
  Values:
    SUBTOPIC = AWS IOT Thing Subscribe Topic (this can be found in the Thing's Policy)
    PUBTOPIC = AWS IOT Thing Publish Topic (this can be found in the Thing's Policy)
    
    WIFI_SETTINGS
      ssid = wifi network name
      password = wifi network password
    
    AWS_SETTINGS
      thingName = AWS IOT Thing name
      endpoint = AWS IOT Endpoint (found in AWS IoT Core > Settings > Device data endpoint)
      rootCA = Amazon Root CA1 certificate
      certificate_pem_crt = certificate.pem.crt (downloaded when creating Thing)
      private_pem_key = private.pem.key (downloaded when creating Thing)
