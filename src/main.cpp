#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_log.h>
#include <esp_task_wdt.h>

using namespace std;

static const char *TAG = "POSTE";

const int READ_PIN = 34;
const int LED_PIN = 16;
const int RESOLUTION = 12; 

const int THRESHOLD = 2000;

// AP credentials
const char *ssid = "redes-renato";
const char *password = "12345678";
const char *serverName = "http://192.168.4.1/monitor";

AsyncWebServer server(80);


void baseStationSetup() {
  WiFi.softAP(ssid, password);
  Serial.println(WiFi.macAddress());

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/monitor", HTTP_POST, [](AsyncWebServerRequest *request) {
    AsyncWebParameter *p = request->getParam("light", true);
    String light = p->value();
    Serial.println(light);

    if (light.toInt() < THRESHOLD) {
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

  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
}


int getLight() {
  analogReadResolution(RESOLUTION);
  
  int light = analogRead(READ_PIN);
  Serial.println("light: " + String(light));
  return light;
}


void baseStationLoop() {
  Serial.println(WiFi.macAddress());

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  delay(5000);
}

void poleLoop() {
  while (true) {
    WiFiClient client;
    HTTPClient http;

    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Send HTTP POST request
    Serial.println("Sending POST request");
    int httpResponseCode = http.POST("light=" + String(getLight()));
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);

    if (payload == "on") {
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }


    // Free resources
    http.end();
    delay(500);
  }

}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(READ_PIN, INPUT);
  Serial.begin(9600);
  delay(2000);

  if (strcmp(TAG, "BS") == 0) {
    Serial.println("setup base");
    baseStationSetup();
  } else {
    Serial.println("setup pole");
    poleSetup();
  }
}

void loop() {
  if (strcmp(TAG, "BS") == 0) {
    Serial.println("loop base");
    baseStationLoop();
  } else {
    Serial.println("loop pole");
    poleLoop();
  }
}