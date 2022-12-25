#include "webserver.hpp"
#include "ESPAsyncWebServer.h"



#ifdef ESP32
#include "AsyncTCP.h"
#include <FS.h>
#include <SPIFFS.h>
#else
#include "ESPAsyncTCP.h"
#endif

#include "midi_player.hpp"

DNSServer dns;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // access at ws://[esp ip]/ws
AsyncEventSource events("/events"); // event source (Server-Sent events)

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

const char* PARAM_MESSAGE = "message"; 
const char* SONG_MESSAGE = "song"; 
bool shouldReboot = false;
void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void SetupWebServer(){
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("WiFi Failed!\n");
    return;
  } 
  dns.stop();
  dns.start(53, "*", WiFi.softAPIP());

  server.reset();
    server.serveStatic("/", SPIFFS, "/www/");
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/sugar_bear.html", "text/html");
  });

  // Send a GET request to <IP>/get?message=<message>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
      String message;
      if (request->hasParam(PARAM_MESSAGE)) {
          message = request->getParam(PARAM_MESSAGE)->value();
      } else {
          message = "No message sent";
      }
      request->send(200, "text/plain", "Hello, GET: " + message);
  });

  // Send a POST request to <IP>/post with a form field message set to <message>
  server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
      String message;
      if (request->hasParam(PARAM_MESSAGE, true)) {
          message = request->getParam(PARAM_MESSAGE, true)->value();
      } else {
          message = "No message sent";
      }
      request->send(200, "text/plain", "Hello, POST: " + message);
  });

  // Send a POST request to <IP>/post with a form field message set to <message>
  server.on("/music", HTTP_GET, [] (AsyncWebServerRequest *request) {
    //Play midi in another task

    Serial.println("Got music request...");
    String song;
      if (request->hasParam(SONG_MESSAGE, true)) {
          song = request->getParam(SONG_MESSAGE, true)->value();
      } else {
          song = "sugar_bear.mid";
      }

    xTaskCreatePinnedToCore(
      PlayMidi,   /* Function to implement the task */
      "musicTask", /* Name of the task */
      10000,      /* Stack size in words */
      NULL,       /* Task input parameter */
      0,          /* Priority of the task */
      NULL,       /* Task handle. */
      0);  /* Core where the task should run */

      Serial.println("Task created...");
      request->send(200, "text/plain", "Playing Music: " + song);
  });

  server.onNotFound(notFound);

  server.begin();
}
void LoopWebServer(){
  // if(shouldReboot){
  //   Serial.println("Rebooting...");
  //   delay(100);
  //   ESP.restart();
  // }
  // static char temp[128];
  // sprintf(temp, "Seconds since boot: %u", millis()/1000);
  // events.send(temp, "time"); //send event "time"
  dns.processNextRequest();
}