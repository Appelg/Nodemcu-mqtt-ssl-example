/*
    Pubsubclient -> cloudmqtt.com SSL

    https://github.com/knolleary/pubsubclient

*/

#include <time.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <CACert.h>

/* =================== EDIT CONFIG HERE ============================ */
//Either create & populate or remove "secrets.h" and edit this file
#include "secrets.h"
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;
const char* mqtt_server = CLOUDMQTT_server;
const int mqtt_ssl_port = CLOUDMQTT_ssl_port;
const char* mqtt_user = CLOUDMQTT_user;
const char* mqtt_password = CLOUDMQTT_pass;
/* =================== EDIT CONFIG HERE ============================ */

// Mqtt
extern const char cloudmqttRootCert[] PROGMEM;
BearSSL::WiFiClientSecure secureWifiClient;
PubSubClient pubSubClient(secureWifiClient);
BearSSL::X509List cert(cacert);

// Set time via NTP, as required for x.509 validation
void setClock() {
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}


void setup() {
  // Basic setup & wifi-connect
  Serial.begin(115200);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Mqtt server & Load cert
  pubSubClient.setServer(mqtt_server, mqtt_ssl_port);
  setClock();
  secureWifiClient.setTrustAnchors(&cert);
}

void reconnectMqtt() {
  while (!pubSubClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (pubSubClient.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      pubSubClient.publish("connect", clientId.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(pubSubClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  // Connect MQTT
  if (!pubSubClient.connected()) {
    reconnectMqtt();
  }

  // Do MQTT
  pubSubClient.loop();
  Serial.print("Sending message to mqtt... ");
  pubSubClient.publish("debug", "YEAH SSL!!!");
  Serial.println("Done");

  delay(5000);
}