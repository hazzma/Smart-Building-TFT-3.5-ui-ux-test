#include <Arduino.h>
#include "display.h"
#include "data.h"
#include "ui_screens.h"
#include "ui_widgets.h"
#include "touch.h"
#include <WiFi.h>
#include "mqtt_manager.h"
#include "lan_manager.h"
#include "wifi_manager.h"
#include "time_manager.h"

// ─────────────────────────────────────────────────────────────────
// Task_Net: Core 0 — Data & Logic Engine (Non-Visual)
// ─────────────────────────────────────────────────────────────────
void Task_Net(void *pvParameters) {
    uint32_t last_sim_update = 0;
    
    wifi_manager_init();
    wifi_manager_load_and_connect();
    time_manager_init();
    
    vTaskDelay(pdMS_TO_TICKS(5000)); // Wait for initial connect

    mqtt_init();

    for (;;) {
        uint32_t now = millis();
        static int last_prio = -1;

        // Unified state update
        data_lock(g_state);
        int current_prio = g_state.net.net_priority;
        g_state.net.wifi_connected = (WiFi.status() == WL_CONNECTED);

        // Data Timeout Check (10s)
        if (now - g_state.last_data_ts > 10000 && !g_state.use_dummy) {
            bool changed = (g_state.sensor.temp[0] != -100.0f);
            for(int i=0; i<4; i++) g_state.sensor.temp[i] = -100.0f;
            g_state.sensor.lux = -1.0f;
            g_state.sensor.co2 = -1;
            if (changed) g_state.ui_needs_update = true;
            g_state.last_data_ts = now; 
        }

        // Simulation update (if use_dummy is true)
        if (g_state.use_dummy && (now - last_sim_update > 500)) {
            last_sim_update = now;
            g_state.sensor.temp[0] += 0.01f;
            if (g_state.sensor.temp[0] > 35.0f) g_state.sensor.temp[0] = 20.0f;
            g_state.ui_needs_update = true;
        }
        data_unlock(g_state);

        if (current_prio != last_prio) {
            last_prio = current_prio;
            wifi_manager_set_power(current_prio == 0);
        }

        if (current_prio == 0) wifi_manager_loop();
        lan_loop();
        mqtt_loop();
        time_manager_update();
        
        static uint32_t last_hb = 0;
        if (now - last_hb > 5000) {
            last_hb = now;
            Serial.printf("[NET] Alive | Prio:%d | MQTT:%s\n", current_prio, is_mqtt_connected()?"OK":"FAIL");
        }

        vTaskDelay(pdMS_TO_TICKS(30)); 
    }
}

// ─────────────────────────────────────────────────────────────────
// Task_Touch: Core 1 — Independent Touch Polling (Low Priority)
// ─────────────────────────────────────────────────────────────────
void Task_Touch(void *pvParameters) {
    int tx, ty;
    for (;;) {
        if (touch_get_point(tx, ty)) {
            screens_handle_touch(g_state, tx, ty);
            data_lock(g_state);
            g_state.ui_needs_update = true; // Force UI to wake up
            data_unlock(g_state);
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

// ─────────────────────────────────────────────────────────────────
// Task_UI: Core 1 — Turbo Visual Engine (Rendering Only)
// ─────────────────────────────────────────────────────────────────
void Task_UI(void *pvParameters) {
    int frames = 0;
    int current_fps = 0;
    uint32_t last_time = millis();
    uint32_t last_frame_time = 0;
    
    uint32_t total_render_time = 0;
    uint32_t total_push_time = 0;
    
    const int target_frame_ms = 25; // 40 FPS
    
    for (;;) {
        uint32_t now = millis();
        
        bool needs_update = false;
        data_lock(g_state);
        needs_update = g_state.ui_needs_update;
        // Optional: Force update every 2 seconds for clock/stability
        static uint32_t last_force_refresh = 0;
        if (now - last_force_refresh > 2000) {
            needs_update = true;
            last_force_refresh = now;
        }
        data_unlock(g_state);

        if (needs_update && (now - last_frame_time >= target_frame_ms)) {
            last_frame_time = now;
            
            // Try to take the bus for rendering
            if (xSemaphoreTake(bus_mutex, pdMS_TO_TICKS(target_frame_ms)) == pdTRUE) {
                // 1. Measure Render Time
                uint32_t t_start = micros();
                screens_render(g_state, current_fps);
                total_render_time += (micros() - t_start);
                
                // 2. Measure Push Time
                t_start = micros();
                canvas.pushSprite(0, 0);
                total_push_time += (micros() - t_start);
                
                widgets_swap();
                
                xSemaphoreGive(bus_mutex);
                frames++;

                // Reset dirty flag
                data_lock(g_state);
                g_state.ui_needs_update = false;
                data_unlock(g_state);
            }
        }

        if (millis() - last_time >= 1000) {
            current_fps = frames;
            uint32_t avg_render = (frames > 0) ? (total_render_time / frames / 1000) : 0;
            uint32_t avg_push = (frames > 0) ? (total_push_time / frames / 1000) : 0;
            
            Serial.printf("[TURBO] FPS: %d | Render: %dms | Push: %dms\n", 
                          current_fps, avg_render, avg_push);
            
            frames = 0;
            total_render_time = 0;
            total_push_time = 0;
            last_time = millis();
        }
        
        vTaskDelay(1); 
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);

    display_init();
    data_init(g_state);
    screens_init();
    touch_init();
    lan_task_init();

    xTaskCreatePinnedToCore(Task_Net, "Task_Net", 8192, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(Task_Touch, "Task_Touch", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(Task_UI, "Task_UI", 16384, NULL, 4, NULL, 1);
}

void loop() {
    vTaskDelete(NULL);
}
