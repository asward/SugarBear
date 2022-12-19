#ifndef _SUGARBEAR_WEBSERVER_h
#define _SUGARBEAR_WEBSERVER_h

#include <DNSServer.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include "ESPAsyncWebServer.h"

void SetupWebServer();
void LoopWebServer();

#endif
