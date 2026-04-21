#include "wifi_manager.h"
#include "data.h"

static Preferences prefs;

void wifi_manager_init() {
    prefs.begin("wifi_cfg", false);
}

void wifi_manager_set_power(bool on) {
    if (on) {
        WiFi.mode(WIFI_STA);
        Serial.println("[WIFI] Power ON");
    } else {
        WiFi.mode(WIFI_OFF);
        Serial.println("[WIFI] Power OFF");
        data_lock(g_state);
        strcpy(g_state.net.connected_wifi_ssid, "-");
        data_unlock(g_state);
    }
}

void wifi_manager_connect(const char* ssid, const char* pass) {
    if (!ssid || strlen(ssid) == 0) return;

    Serial.printf("[WIFI] Connecting to: %s\n", ssid);
    
    // Save to NVS
    prefs.putString("ssid", ssid);
    prefs.putString("pass", pass);

    WiFi.disconnect();
    WiFi.begin(ssid, pass);
}

void wifi_manager_load_and_connect() {
    String ssid = prefs.getString("ssid", "han");
    String pass = prefs.getString("pass", "hanhanhan");

    if (ssid.length() > 0) {
        Serial.printf("[WIFI] Auto-connecting to SSID: %s\n", ssid.c_str());
        WiFi.begin(ssid.c_str(), pass.c_str());
    } else {
        Serial.println("[WIFI] No saved credentials found.");
    }
}

void wifi_manager_reconnect() {
    String ssid = prefs.getString("ssid", "han");
    String pass = prefs.getString("pass", "hanhanhan");
    if (ssid.length() > 0) {
        Serial.printf("[WIFI] Manual Reconnect to: %s\n", ssid.c_str());
        WiFi.mode(WIFI_STA); // Ensure radio is ON
        WiFi.disconnect();
        WiFi.begin(ssid.c_str(), pass.c_str());
    }
}

void wifi_manager_loop() {
    static uint32_t last_check = 0;
    if (millis() - last_check > 2000) {
        last_check = millis();
        wl_status_t status = WiFi.status();
        
        data_lock(g_state);
        if (status == WL_CONNECTED) {
            strncpy(g_state.net.connected_wifi_ssid, WiFi.SSID().c_str(), 32);
            strcpy(g_state.net.wifi_status_detail, "SUCCESS: Connected");
        } else if (status == WL_IDLE_STATUS || status == WL_DISCONNECTED) {
            strcpy(g_state.net.wifi_status_detail, "IDLE: Disconnected");
        } else if (status == WL_CONNECT_FAILED) {
            strcpy(g_state.net.wifi_status_detail, "FAILED: Wrong Credentials?");
        } else if (status == WL_NO_SSID_AVAIL) {
            strcpy(g_state.net.wifi_status_detail, "FAILED: SSID Not Found");
        } else {
            strcpy(g_state.net.wifi_status_detail, "CONNECTING...");
        }
        data_unlock(g_state);
    }
}
