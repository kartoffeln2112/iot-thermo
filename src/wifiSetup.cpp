/* Wifi Setup code for wifi related functions in the IoT Thermo project.
    Detailed info on functions can be found in wifiSetup.h.

    Functions:
        wific() - setup object
        initWifi() - setup wifi connection, NTP connection (for timestamps), SSL and MQTT connection
        checkMQTTConnect() - check MQTT connection is current, and ping MQTT
        checkWifiConnect() - check wifi connection is current
        publish(char*) - publish a message to MQTT
        getTimestamp() - get the current time from NTP
        psErr(int8_t) - pubsubclient error message print, message error codes found on: https://pubsubclient.knolleary.net/api
*/

#include "wifiSetup.h"

wific::wific(){return;}


long unsigned int wific::getTimestamp()
{
    struct tm timeinfo;
    // get local time from NTP
    if (!getLocalTime(&timeinfo))
    {
        M5.Lcd.printf("Failed to obtain time");
        return 0;
    }
    // print local time and return
    M5.Lcd.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    return mktime(&timeinfo);
}


void wific::initWifi()
{
    // start wifimanager auto connect, if no known wifi are in range, start AP to config new wifi connection
    wifiManager.autoConnect("ESP32_AP", "password123");

    //connect to NTP & test
    configTime(0, 0, "pool.ntp.org");
    M5.Lcd.printf("NTP Time: ");
    getTimestamp();
    
    // set up certificates
    m5client.setCACert(awsset.rootCA);
    m5client.setCertificate(awsset.certificate_pem_crt);
    m5client.setPrivateKey(awsset.private_pem_key);
    
    // check that m5client is able to connect
    M5.Lcd.printf("\nM5Client to AWS\n");
    if (!m5client.connect(awsset.endpoint, awsset.port))
    {
        char garbo[256];
        M5.Lcd.printf("Connection failed! Error %d", m5client.lastError(garbo,256));
    }
    else
        M5.Lcd.printf("Success\n");

    // set up mqtt server details
    psclient.setClient(m5client);
    psclient.setServer(awsset.endpoint, awsset.port);

    // connect to MQTT
    M5.Lcd.print("Connecting to MQTT...");
    while (!psclient.connected())
    {
        psErr(psclient.state());
        delay(500);
        M5.Lcd.print(".");
        psclient.connect(awsset.thingName);
        checkWifiConnect();
        Serial.printf("\nWifi status: %d", WiFi.status());
        Serial.printf("\nSSL status:  %s\n", m5client.connected()?"connected":"not connected");
    }
    if(psclient.connected())
    {
        M5.Lcd.print("Success");
    }
    
    // subscribe to topic
    psclient.subscribe(SUBTOPIC);
    if(!psclient.subscribe(SUBTOPIC))
    {
        M5.Lcd.printf("MQTT channel subscription failure. Error code = %s", psErr(psclient.state()));
    }
    delay(1000);

    return;
}


void wific::checkMQTTConnect()
{
    // check that pubsubclient is connected, reconnect if not
    while (!psclient.connected())
    {
        M5.Lcd.printf("\nReconnecting to MQTT...");
        if (psclient.connect(awsset.thingName))
        {
            M5.Lcd.print("success!");

            // resubscribe after reconnecting
            if(!psclient.subscribe(SUBTOPIC))
            {
                M5.Lcd.printf("MQTT channel subscription failure. Error code = %s", psErr(psclient.state()));
            }
        }
        else
        {
            // if MQTT reconnect fail, ensure that wifi is connected
            checkWifiConnect();
            M5.Lcd.printf("Failed to connect. Error code = %s", psErr(psclient.state()));
        }
    }

    // ping to keep connection current
    psclient.loop();

    return;
}


void wific::checkWifiConnect()
{
    // if wifi not connected, reconnect
    if (WiFi.status() != WL_CONNECTED)
    {
        WiFi.reconnect();
    }
    // if ssl not connected, reconnect
    if (!m5client.connected())
    {
        m5client.connect(awsset.endpoint, awsset.port);
    }
    return;
}


void wific::publish(const char* msg)
{
    // attempt to publish to MQTT
    bool success = psclient.publish(PUBTOPIC, msg);

    // print success or error message
    M5.Lcd.printf("Sending data to MQTT: ");
    if (success)
    {
        M5.Lcd.print("Complete");
    }
    else
    {
        M5.Lcd.printf("Failed. Error code = %s", psErr(psclient.state()));
    }
    return;
}


const char* wific::psErr(int8_t mqerr)
{
    if (mqerr == -4)
        return "Connection timeout";
    else if (mqerr == -3)
        return "Connection lost";
    else if (mqerr == -2)
        return"Connect failed";
    else if (mqerr == -1)
        return "Disconnected";
    else if (mqerr == 0)
        return "Connected";
    else if (mqerr == 1)
        return "Connect bad protocol";
    else if (mqerr == 2)
        return "Connect bad client id";
    else if (mqerr == 3)
        return "Connect unavailable";
    else if (mqerr == 4)
        return "Connect bad credentials";
    else if (mqerr == 5)
        return "Connect unauthorized";

    return;
}
