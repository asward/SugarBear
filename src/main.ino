#include <Arduino.h>
#include "webserver.hpp"
#include "wifi.hpp"
#include "midi_player.hpp"
#include <SPIFFS.h>

void setup() {
  Serial.begin(115200);   Serial.println("Running");
  
  #ifdef ESP32
    if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
  #else
    if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
  #endif

  SetupWiFi();
  SetupWebServer();
  SetupMidiPlayer();
}

void loop() {
  LoopWiFi();
  LoopWebServer();
}
