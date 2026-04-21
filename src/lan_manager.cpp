#include "lan_manager.h"
#include <SPI.h>
#include <Ethernet.h>
#include <Preferences.h>
#include <Dns.h>
#include "data.h"

// LAN Pinout from PINOUTlan.md
#define LAN_MOSI 11
#define LAN_MISO 13
#define LAN_SCK  12
#define LAN_CS   10
#define LAN_RST  18

SPIClass lanSPI(2); // Using SPI2_HOST on S3

void lan_manager_load_config() {
    Preferences prefs;
    prefs.begin("lan_config", true);
    data_lock(g_state);
    g_state.net.lan_use_dhcp = prefs.getBool("dhcp", true);
    String ip = prefs.getString("static_ip", "192.168.1.177");
    String gw = prefs.getString("gateway", "192.168.1.1");
    String sn = prefs.getString("subnet", "255.255.255.0");
    strncpy(g_state.net.lan_static_ip, ip.c_str(), 16);
    strncpy(g_state.net.lan_gateway, gw.c_str(), 16);
    strncpy(g_state.net.lan_subnet, sn.c_str(), 16);
    data_unlock(g_state);
    prefs.end();
}

void lan_manager_save_config() {
    Preferences prefs;
    prefs.begin("lan_config", false);
    data_lock(g_state);
    prefs.putBool("dhcp", g_state.net.lan_use_dhcp);
    prefs.putString("static_ip", g_state.net.lan_static_ip);
    prefs.putString("gateway", g_state.net.lan_gateway);
    prefs.putString("subnet", g_state.net.lan_subnet);
    data_unlock(g_state);
    prefs.end();
    Serial.println("[LAN] Config Saved. Restarting Ethernet...");
    lan_init(); // Re-init with new settings
}

void lan_check_internet() {
    if (Ethernet.linkStatus() != LinkON) return;
    
    DNSClient dns;
    dns.begin(Ethernet.dnsServerIP());
    IPAddress remote_ip;
    bool has_internet = (dns.getHostByName("google.com", remote_ip) == 1);
    
    data_lock(g_state);
    if (has_internet) {
        strcpy(g_state.net.lan_status_detail, "Internet Access OK");
    } else {
        strcpy(g_state.net.lan_status_detail, "Local Only (No Internet)");
    }
    data_unlock(g_state);
}

void lan_init() {
    Serial.println("[LAN] Initializing Ethernet...");
    
    lan_manager_load_config();
    
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    mac[5] ^= 0x01; 
    
    pinMode(LAN_RST, OUTPUT);
    digitalWrite(LAN_RST, LOW);
    delay(100);
    digitalWrite(LAN_RST, HIGH);
    delay(200);

    lanSPI.begin(LAN_SCK, LAN_MISO, LAN_MOSI, LAN_CS);
    Ethernet.init(LAN_CS); 
    
    bool success = false;
    data_lock(g_state);
    bool use_dhcp = g_state.net.lan_use_dhcp;
    data_unlock(g_state);

    if (use_dhcp) {
        Serial.println("[LAN] Trying DHCP...");
        if (Ethernet.begin(mac, 10000, 4000) != 0) { // 10s timeout
            success = true;
        } else {
            Serial.println("[LAN] DHCP Failed.");
        }
    }

    if (!success) {
        Serial.println("[LAN] Using Static IP...");
        IPAddress ip, gw, sn, dns_addr(8, 8, 8, 8);
        data_lock(g_state);
        ip.fromString(g_state.net.lan_static_ip);
        gw.fromString(g_state.net.lan_gateway);
        sn.fromString(g_state.net.lan_subnet);
        data_unlock(g_state);
        
        Ethernet.begin(mac, ip, dns_addr, gw, sn);
    }

    IPAddress ip_res = Ethernet.localIP();
    data_lock(g_state);
    g_state.net.lan_connected = (Ethernet.linkStatus() == LinkON);
    snprintf(g_state.net.lan_ip, 16, "%d.%d.%d.%d", ip_res[0], ip_res[1], ip_res[2], ip_res[3]);
    data_unlock(g_state);
    
    Serial.print("[LAN] Ready. IP: ");
    Serial.println(ip_res);
    lan_check_internet();
}

void lan_loop() {
    static uint32_t last_link_check = 0;
    static uint32_t last_internet_check = 0;
    
    if (millis() - last_link_check > 2000) {
        last_link_check = millis();
        bool link = (Ethernet.linkStatus() == LinkON);
        
        data_lock(g_state);
        if (g_state.net.lan_connected != link) {
            g_state.net.lan_connected = link;
            g_state.ui_needs_update = true;
            Serial.printf("[LAN] Link Status Changed: %s\n", link ? "CONNECTED" : "DISCONNECTED");
        }
        data_unlock(g_state);
    }

    if (millis() - last_internet_check > 30000) { // Every 30s
        last_internet_check = millis();
        lan_check_internet();
    }
}

bool is_lan_connected() {
    return Ethernet.linkStatus() == LinkON;
}

// Non-blocking Task for Core 0
void Task_LAN(void *pvParameters) {
    Serial.println("[LAN] Task started on Core 0");
    
    // Initial delay to let power stabilize
    vTaskDelay(pdMS_TO_TICKS(500));
    
    lan_init();
    
    for(;;) {
        lan_loop();
        // Sleep to be non-blocking
        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}

void lan_task_init() {
    xTaskCreatePinnedToCore(
        Task_LAN,
        "LAN_Task",
        4096,
        NULL,
        1,
        NULL,
        0 // Pinned to Core 0
    );
}
