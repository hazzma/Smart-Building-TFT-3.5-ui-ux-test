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
        // Handle WiFi Power based on Priority
        static int last_prio = -1;
        data_lock(g_state);
        int current_prio = g_state.net.net_priority;
        data_unlock(g_state);

        if (current_prio != last_prio) {
            last_prio = current_prio;
            wifi_manager_set_power(current_prio == 0); // ON if WiFi Prio
        }

        if (current_prio == 0) {
            wifi_manager_loop();
        }

        time_manager_update();
        lan_loop();
        mqtt_loop();
        
        data_lock(g_state);
        g_state.net.wifi_connected = (WiFi.status() == WL_CONNECTED);
        data_unlock(g_state);

        // Simulasi update data sensor setiap 500ms
        if (millis() - last_sim_update > 500) {
            last_sim_update = millis();
            
            data_lock(g_state);
            // Hanya simulasi jika data dari MQTT belum masuk (opsional)
            // g_state.sensor.temp[0] += 0.05f;
            // if (g_state.sensor.temp[0] > 35.0f) g_state.sensor.temp[0] = 20.0f;
            
            // g_state.sensor.lux = 300 + random(0, 100);
            // g_state.sensor.co2 = 600 + random(0, 200);
            
            // Trigger UI Refresh
            g_state.ui_needs_update = true;
            data_unlock(g_state);
        }
        
        vTaskDelay(pdMS_TO_TICKS(50)); 
    }
}

// ─────────────────────────────────────────────────────────────────
// Task_UI: Core 1 — Turbo Visual Engine (40 FPS Target)
// ─────────────────────────────────────────────────────────────────
void Task_UI(void *pvParameters) {
    int frames = 0;
    int current_fps = 0;
    uint32_t last_time = millis();
    uint32_t last_frame_time = 0;
    uint32_t last_touch_poll = 0;
    
    uint32_t total_render_time = 0;
    uint32_t total_push_time = 0;
    
    int tx, ty;
    const int target_frame_ms = 25; // 40 FPS
    
    for (;;) {
        if (millis() - last_touch_poll > 20) { 
            last_touch_poll = millis();
            if (touch_get_point(tx, ty)) {
                 screens_handle_touch(g_state, tx, ty);
            }
        }


        uint32_t now = millis();
        if (now - last_frame_time >= target_frame_ms) {
            last_frame_time = now;
            
            // 1. Measure Render Time
            uint32_t t_start = micros();
            screens_render(g_state, current_fps);
            total_render_time += (micros() - t_start);
            
            // 2. Measure Push Time
            t_start = micros();
            canvas.pushSprite(0, 0);
            total_push_time += (micros() - t_start);
            
            widgets_swap();
            frames++;
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
    xTaskCreatePinnedToCore(Task_UI, "Task_UI", 16384, NULL, 4, NULL, 1);
}

void loop() {
    vTaskDelete(NULL);
}
