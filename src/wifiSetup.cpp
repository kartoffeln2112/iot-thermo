#include "wifiSetup.h"

wific::wific(){return;}
void msgReceive(char* topic, uint8_t* payload, unsigned int length);

void wific::initWifi()
{    
    M5.Lcd.printf("\nConnecting to %s", wifi_settings.ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_settings.ssid, wifi_settings.password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        M5.Lcd.print(".");
    }
    M5.Lcd.printf("Success\n");
    
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
    M5.Lcd.printf("Success\n");

    // set up mqtt server details
    psclient.setClient(m5client);
    psclient.setServer(awsset.endpoint, awsset.port);
    psclient.setCallback(msgReceive);

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
        M5.Lcd.print("success");
    }
    
    psclient.subscribe(SUBTOPIC);
    delay(1000);

    return;
}

void msgReceive(char* topic, uint8_t* payload, unsigned int length)
{
    M5.Lcd.printf("\nMessage received on %s: ", topic);
    for (int i = 0; i < length; i++)
    {
        M5.Lcd.print((char)payload[i]);
    }
    return;
}

void wific::checkMQTTConnect()
{
    while (!psclient.connected())
    {
        M5.Lcd.printf("\nReconnecting to MQTT...");
        if (psclient.connect(awsset.thingName))
        {
            M5.Lcd.print("success!");
            if(!psclient.subscribe(SUBTOPIC))
            {
                psErr(psclient.state());
            }
        }
        else
        {
            checkWifiConnect();
            M5.Lcd.print("Failed to connect. Error code = ");
            psErr(psclient.state());
        }
    }

    psclient.loop();

    return;
}

void wific::checkWifiConnect()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        WiFi.reconnect();
    }
    if (!m5client.connected())
    {
        m5client.connect(awsset.endpoint, awsset.port);
    }
    return;
}

void wific::publish(const char* msg)
{
    bool success = psclient.publish(PUBTOPIC, msg);
    M5.Lcd.printf("Sending data to MQTT: ");
    if (success)
    {
        M5.Lcd.print("Complete");
    }
    else
    {
        M5.Lcd.print("Failed. Error code = ");
        psErr(psclient.state());
    }
    return;
}

void wific::psErr(int8_t mqerr)
{
    if (mqerr == MQTT_CONNECTION_TIMEOUT)
        Serial.print("Connection timeout");
    else if (mqerr == MQTT_CONNECTION_LOST)
        Serial.print("Connection lost");
    else if (mqerr == MQTT_CONNECT_FAILED)
        Serial.print("Connect failed");
    else if (mqerr == MQTT_DISCONNECTED)
        Serial.print("Disconnected");
    else if (mqerr == MQTT_CONNECTED)
        Serial.print("Connected");
    else if (mqerr == MQTT_CONNECT_BAD_PROTOCOL)
        Serial.print("Connect bad protocol");
    else if (mqerr == MQTT_CONNECT_BAD_CLIENT_ID)
        Serial.print("Connect bad Client-ID");
    else if (mqerr == MQTT_CONNECT_UNAVAILABLE)
        Serial.print("Connect unavailable");
    else if (mqerr == MQTT_CONNECT_BAD_CREDENTIALS)
        Serial.print("Connect bad credentials");
    else if (mqerr == MQTT_CONNECT_UNAUTHORIZED)
        Serial.print("Connect unauthorized");

    return;
}