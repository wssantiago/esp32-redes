#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_log.h>
#include <esp_task_wdt.h>

using namespace std;

static const char *TAG = "BS";

const int ANALOG_READ_PIN = 36; // or A0
const int RESOLUTION = 12; // Could be 9-12

const int THRESHOLD = 1000; //TODO: avaliar limite

// AP credentials
const char *ssid = "BASESTATION";
const char *password = "redes-renato";
const char *serverName = "http://192.168.4.1/monitor"; // TODO: mudar IP?

AsyncWebServer server(80);


///////////////// SETUPS /////////////////
void baseStationSetup() {
  WiFi.softAP(ssid, password);
  Serial.println(WiFi.macAddress());

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/monitor", HTTP_POST, [](AsyncWebServerRequest *request) {
    AsyncWebParameter *p = request->getParam("light", true);
    String light = p->value();

    if (light.toInt() > THRESHOLD) {
      request->send_P(200, "text/plain", "off");
    }
    else {
      request->send_P(200, "text/plain", "on");
    }    
  });

  // Start server
  server.begin();
}

void poleSetup() {
  WiFi.begin(ssid, password);
  Serial.println("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}
///////////////// SETUPS /////////////////


int getLight() {
  analogReadResolution(RESOLUTION);
  
  return analogRead(ANALOG_READ_PIN);
}


///////////////// TASKS /////////////////
void sendPoleRequests(void *param) {
  esp_task_wdt_init(10, false);

  while (true) {
    WiFiClient client;
    HTTPClient http;

    http.begin(client, serverName);

    // Send HTTP POST request
    int httpResponseCode = http.POST(static_cast<String>(getLight()));

    if (httpResponseCode == HTTP_CODE_OK) {
      String payload = http.getString();
      
      if (payload == "on") {
        // TODO: set LED on
      }
      else if (payload == "off") {
        // TODO: set LED off
      }
    }

    // Free resources
    http.end();
  }
}
///////////////// TASKS /////////////////


///////////////// LOOPS /////////////////
void baseStationLoop() {}

void poleLoop() {
  xTaskCreate(sendPoleRequests, "send requests", 8192, NULL, 1, NULL);

}
///////////////// LOOPS /////////////////

void setup() {
  Serial.begin(115200);

  // Setup
  if (strcmp(TAG, "BS") == 0) {
    baseStationSetup();
  } else {
    poleSetup();
  }
}

void loop() {
  if (strcmp(TAG, "BS") == 0) {
    baseStationLoop();
  } else {
    poleLoop();
  }
}