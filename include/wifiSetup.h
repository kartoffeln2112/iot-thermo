#include <stdbool.h>
#include <M5Core2.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <esp_wpa2.h>
#include <WiFiManager.h>
#include "time.h"

#include "ws.h"

class wific {
    public:
        
        wific();
        void initWifi();
        void checkMQTTConnect();
        void checkWifiConnect();
        void publish(const char* msg);
        long unsigned int getTimestamp();
    private:
        WIFI_SETTINGS wifi_settings;
        AWS_SETTINGS awsset;
        WiFiManager wifiManager;
        WiFiClientSecure m5client;
        PubSubClient psclient;

        //bool homeConnect();
        bool utdConnect();
        bool connTimeout();
        void psErr(int8_t mqerr);
};
