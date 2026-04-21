#include "data.h"
#include <string.h>

BuildingState g_state;

void data_init(BuildingState& state) {
    state.mutex = xSemaphoreCreateMutex();
    state.use_dummy = true;
    data_load_dummy(state);
}

void data_load_dummy(BuildingState& state) {
    if (xSemaphoreTake(state.mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        state.sensor.temp[0] = -100.0f;
        state.sensor.temp[1] = -100.0f;
        state.sensor.temp[2] = -100.0f;
        state.sensor.temp[3] = -100.0f;
        state.sensor.temp_target = 23.0f;
        state.sensor.lux = -1.0f;
        state.sensor.co2 = -1;
        state.sensor.ac_on = true;
        state.sensor.projector_on = false;
        state.sensor.light_on = true;
        state.sensor.human_presence = true;
        memset(state.sensor.sensor_error, 0, sizeof(state.sensor.sensor_error)); 
        state.sensor.slave_count = 2;
        state.sensor.slave_online[0] = true;
        state.sensor.slave_online[1] = true;

        state.net.wifi_connected = true;
        state.net.lan_connected = false;
        state.net.firebase_ok = true;
        state.dashboard_page = 0;
        strcpy(state.net.time_str, "--:--");
        strcpy(state.net.room_name, "Meeting Room A");
        strcpy(state.net.slave_name[0], "Slave 1");
        strcpy(state.net.slave_name[1], "Slave 2");
        xSemaphoreGive(state.mutex);
    }
}

void data_lock(BuildingState& state) {
    xSemaphoreTake(state.mutex, portMAX_DELAY);
}

void data_unlock(BuildingState& state) {
    xSemaphoreGive(state.mutex);
}
