#include "time_manager.h"
#include "data.h"

static EthernetUDP udp;
static const char* ntpServer = "pool.ntp.org";
static const int NTP_PACKET_SIZE = 48;
static byte packetBuffer[NTP_PACKET_SIZE];

void time_manager_init() {
    configTime(7 * 3600, 0, ntpServer);
}

static void sendNTPpacket(IPAddress& address) {
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;
    udp.beginPacket(address, 123);
    udp.write(packetBuffer, NTP_PACKET_SIZE);
    udp.endPacket();
}

#include <Dns.h>

void sync_time_lan() {
    if (Ethernet.linkStatus() != LinkON) return;
    
    DNSClient dns;
    dns.begin(Ethernet.dnsServerIP());
    IPAddress ntpIP;
    
    if (dns.getHostByName(ntpServer, ntpIP) == 1) {
        udp.begin(8888);
        sendNTPpacket(ntpIP);
        delay(1000);
        if (udp.parsePacket()) {
            udp.read(packetBuffer, NTP_PACKET_SIZE);
            unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
            unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
            unsigned long secsSince1900 = highWord << 16 | lowWord;
            const unsigned long seventyYears = 2208988800UL;
            unsigned long epoch = secsSince1900 - seventyYears;
            
            // Update system time
            time_t now = epoch + (7 * 3600); // GMT+7
            struct timeval tv = { .tv_sec = now };
            settimeofday(&tv, NULL);
            Serial.println("[TIME] LAN NTP Sync Success");
        }
    }
    udp.stop();
}

void time_manager_update() {
    static uint32_t last_sync = 0;
    
    // Check which interface to use
    if (WiFi.status() == WL_CONNECTED) {
        // configTime already handles this via LWIP
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            data_lock(g_state);
            strftime(g_state.net.time_str, 16, "%H:%M", &timeinfo);
            data_unlock(g_state);
        }
    } else if (Ethernet.linkStatus() == LinkON) {
        // Manual Sync for LAN every 10 minutes
        if (millis() - last_sync > 600000 || last_sync == 0) {
            last_sync = millis();
            sync_time_lan();
        }
        
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            data_lock(g_state);
            strftime(g_state.net.time_str, 16, "%H:%M", &timeinfo);
            data_unlock(g_state);
        }
    }
}
