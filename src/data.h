#ifndef DATA_H
#define DATA_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

struct SensorData {
  float    temp[4];
  float    temp_target;
  float    lux_avg;
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
  char  room_name[32];
  char  slave_name[2][32];
};

struct BuildingState {
  SensorData          sensor;
  NetworkState        net;
  bool                use_dummy;
  SemaphoreHandle_t   mutex;
};

extern BuildingState g_state;

void data_init(BuildingState& state);
void data_load_dummy(BuildingState& state);
void data_lock(BuildingState& state);
void data_unlock(BuildingState& state);

#endif
