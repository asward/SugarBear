#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#endif

#include <ESPAsyncWebServer.h>     //Local WebServer used to serve the configuration portal
#include <ESPAsyncWiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include "wifi.hpp"


void SetupWiFi(){ 
    
    AsyncWiFiManager wifiManager(&server,&dns);
    //reset saved settings
    // wifiManager.resetSettings();
    //set custom ip for portal
    //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    wifiManager.autoConnect("SugarBearAP");
    //or use this for auto generated name ESP + ChipID
    //wifiManager.autoConnect();
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
}

void LoopWiFi() {
}
