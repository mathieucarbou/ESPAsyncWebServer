#include <Arduino.h>
#include <DNSServer.h>
#ifdef ESP32
#include <AsyncTCP.h>
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "StreamConcat.h"

DNSServer dnsServer;
AsyncWebServer server(80);

void setup() {

  Serial.begin(115200);
  WiFi.softAP("esp-captive");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    File header = LittleFS.open("/header.html", "r");
    File body = LittleFS.open("/body.html", "r");
    File footer = LittleFS.open("/footer.html", "r");
    StreamConcat s1(&header, &body);
    StreamConcat s2(&s1, &footer);
    request->send(s2, "text/html", s2.available());
    header.close();
    body.close();
    footer.close();
  });

  server.onNotFound([](AsyncWebServerRequest* request) {
    request->send(404, "text/plain", "Not found");
  });

  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
}