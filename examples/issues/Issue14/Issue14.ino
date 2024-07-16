#include "esp_wifi.h"
#include <WiFi.h>

#include <ESPAsyncWebServer.h>
#include <queue>

const char appWebPage[] PROGMEM = R"rawliteral(
<body>
<button id="button1" onclick="fetch('/button1');">Relay1</button>
<script>

  const evtSource = new EventSource("/events");

  button1 = document.getElementById("button1");


  evtSource.addEventListener('state', (e) => {
      const data = JSON.parse(e.data);
      console.log('Event Source data: ', data);

      if (data.button1) {
          button1.style.backgroundColor = "green";
      }
      else {
          button1.style.backgroundColor = "red";
      }

  });


</script>
</body>
)rawliteral";

void sendState();

// const char* ssid = "test_esp32";
// const char* password = "123456789";

//==========================================================

// Настройки IP адреса
// IPAddress local_IP(192, 168, 111, 111);
// IPAddress gateway(192, 168, 111, 1);
// IPAddress subnet(255, 255, 255, 0);

// AsyncWebServer server(22222);
AsyncWebServer server(80);

AsyncEventSource events("/events");

unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
const uint16_t interval = 1000;

const int button1Pin = 4;
bool button1 = false;

char json[512];

void prepareJson() {
  snprintf(json, 511, "{\"button1\":%d,\"1234567890abcdefghij1234567890abcdefghij1234567890abcdefghij1234567890abcdefghij1234567890abcdefghij1234567890abcdefghij\":%ld}", button1, random(0, 999999999));
}

void handleRoot(AsyncWebServerRequest* request) {
  request->send(200, "text/html", appWebPage);
}

void button1off() {
  button1 = false;
  digitalWrite(button1Pin, HIGH);
}

void button1on() {
  digitalWrite(button1Pin, LOW);
  button1 = true;
}

void sendState() {
  prepareJson();
  events.send(json, "state", millis());
}

//=========================================================
void setup() {

  Serial.begin(115200);
  delay(100);
  randomSeed(micros());

  pinMode(button1Pin, OUTPUT);
  button1off();

  // WiFi.softAPConfig(local_IP, gateway, subnet);
  // WiFi.softAP(ssid, password, 11);
  WiFi.softAP("esp-captive");

  // WiFi.setTxPower(WIFI_POWER_21dBm);
  // esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_11B);

  server.on("/", HTTP_GET, handleRoot);

  server.on("/button1", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", "OK");
    if (button1) {
      button1off();
    } else {
      button1on();
    }
    sendState();
  });

  events.onConnect([](AsyncEventSourceClient* client) {
    prepareJson();
    client->send(json, "state", millis(), 5000);
  });

  server.addHandler(&events);
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

  server.begin();
}

void loop() {

  currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    sendState();
  }
}