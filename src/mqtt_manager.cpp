#include "mqtt_manager.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "data.h"

const char* mqtt_server = "wd5de919.ala.asia-southeast1.emqxsl.com";
const int mqtt_port_secure = 8883;
const int mqtt_port_normal = 1883;
const char* mqtt_user = "Hansganteng";
const char* mqtt_pass = "12345678";
const char* mqtt_topic_sub = "binus/ayam";

WiFiClientSecure secureClient;
WiFiClient wifiClient;
EthernetClient ethClient;
PubSubClient mqttClient; // No client in constructor

static void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    JsonDocument doc;
    deserializeJson(doc, payload, length);

    if (strcmp(topic, mqtt_topic_sub) == 0) {
        float temp = doc["temperature"].isNull() ? -100.0f : doc["temperature"].as<float>();
        float lux = doc["lux"].isNull() ? -1.0f : doc["lux"].as<float>();
        int co2 = doc["co2"].isNull() ? -1 : doc["co2"].as<int>();
        
        data_lock(g_state);
        if (temp > -50.0f) {
            g_state.sensor.temp[0] = temp;
            g_state.sensor.sensor_error[0] = false;
        } else {
            g_state.sensor.temp[0] = -100.0f;
        }
        g_state.sensor.lux = lux;
        g_state.sensor.co2 = co2;
        g_state.last_data_ts = millis();
        g_state.ui_needs_update = true;
        data_unlock(g_state);
        
        Serial.printf("[MQTT] Data: T:%.1f L:%.0f C:%d\n", temp, lux, co2);
    }
}

void mqtt_init() {
    secureClient.setInsecure();
    mqttClient.setCallback(mqtt_callback);
    mqttClient.setBufferSize(512);
}

static void reconnect() {
    if (!mqttClient.connected()) {
        // Choose client and port based on interface
        if (g_state.net.net_priority == 1 && g_state.net.lan_connected) {
            mqttClient.setClient(ethClient);
            mqttClient.setServer(mqtt_server, mqtt_port_normal);
            Serial.print("[MQTT] Connecting via LAN...");
        } else if (g_state.net.wifi_connected) {
            mqttClient.setClient(secureClient);
            mqttClient.setServer(mqtt_server, mqtt_port_secure);
            Serial.print("[MQTT] Connecting via WiFi (SSL)...");
        } else {
            return; // No connection
        }

        String clientId = "MasterS3-" + String(random(0xffff), HEX);
        if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
            Serial.println("Success!");
            mqttClient.subscribe(mqtt_topic_sub);
            data_lock(g_state);
            g_state.net.mqtt_ok = true;
            data_unlock(g_state);
        } else {
            Serial.printf("Failed (rc=%d)\n", mqttClient.state());
            data_lock(g_state);
            g_state.net.mqtt_ok = false;
            data_unlock(g_state);
        }
    }
}

void mqtt_loop() {
    // Check if we have ANY connection
    bool has_net = g_state.net.wifi_connected || g_state.net.lan_connected;
    if (!has_net) return;

    if (!mqttClient.connected()) {
        static uint32_t last_reconnect = 0;
        if (millis() - last_reconnect > 5000) {
            last_reconnect = millis();
            reconnect();
        }
    } else {
        mqttClient.loop();
    }
}

bool is_mqtt_connected() {
    return mqttClient.connected();
}
