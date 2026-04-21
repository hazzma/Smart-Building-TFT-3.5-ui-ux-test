#ifndef DATA_H
#define DATA_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

struct SensorData {
  float    temp[4];
  float    temp_target;
  float    lux;
  int      co2;
  bool     ac_on;
  bool     projector_on;
  bool     light_on;
  bool     human_presence;
  bool     sensor_error[4];
  uint8_t  slave_count;
  bool     slave_online[2];
};

struct NetworkState {
  bool  wifi_connected;
  bool  lan_connected;
  bool  firebase_ok;
  bool  mqtt_ok;
  int   net_priority; // 0=WiFi, 1=LAN
  bool  lan_use_dhcp;  // true = DHCP, false = Static
  char  lan_ip[16];     // Current IP
  char  connected_wifi_ssid[32]; // Connected SSID name
  char  lan_static_ip[16];
  char  lan_gateway[16];
  char  lan_subnet[16];
  char  time_str[16];   // Waktu dari NTP
  char  room_name[32];
  char  slave_name[2][32];
  char  conn_status[32];
  char  lan_status_detail[64]; // e.g., "DHCP OK", "Internet Access"
  char  wifi_status_detail[64]; // e.g., "Connecting...", "Failed"
};

struct BuildingState {
  SensorData          sensor;
  NetworkState        net;
  int                 dashboard_page;  // 0 = Page 1, 1 = Page 2
  bool                use_dummy;       // true = gunakan data dummy
  bool                ui_needs_update; // Dirty flag untuk optimasi render
  uint32_t            last_data_ts;    // Timestamp data terakhir masuk
  SemaphoreHandle_t   mutex;           // RTOS mutex untuk akses aman
};

extern BuildingState g_state;

void data_init(BuildingState& state);
void data_load_dummy(BuildingState& state);
void data_lock(BuildingState& state);
void data_unlock(BuildingState& state);

#endif
