/* Header file for wifiSetup.cpp which contains the functions related to
    wifi connection and setup. Function info is described in the wific object.
*/

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

        // initialize the wifi, NTP, SSL, and MQTT connections
        // INPUTS: N/A
        // OUTPUT: N/A
        void initWifi();

        // ping MQTT to ensure that connection is current
        // INPUTS: N/A
        // OUTPUT: N/A
        void checkMQTTConnect();

        // check that wifi connection is current
        // INPUTS: N/A
        // OUTPUT: N/A
        void checkWifiConnect();

        // publish a message to MQTT
        // INPUTS: msg (char*) - containting json formatted message
        // OUTPUT: N/A
        void publish(const char* msg);

        // get the current time from NTP
        // INPUTS: N/A
        // OUTPUT: timeinfo (long uns int) - contains UTC formatted timestamp
        long unsigned int getTimestamp();

    private:
        AWS_SETTINGS awsset;
        WiFiManager wifiManager;
        WiFiClientSecure m5client;
        PubSubClient psclient;

        // pubsubclient error message print
        // message error codes found on: https://pubsubclient.knolleary.net/api
        // INPUTS: mqerr (int) - error code from psclient
        // OUPUT: error (char*) - worded error definition
        const char* psErr(int8_t mqerr);
};
