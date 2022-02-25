#include "wifiSetup.h"

void initWifi()
{    
    WIFI_SETTINGS wifi_settings;

    M5.Lcd.printf("\nConnecting to %s", wifi_settings.ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_settings.ssid, wifi_settings.password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        M5.Lcd.print(".");
    }
    M5.Lcd.printf("Success\n");
    delay(1000);

    return;
}