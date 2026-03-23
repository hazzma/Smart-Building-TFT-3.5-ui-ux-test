#include <Arduino.h>
#include "display.h"
#include "data.h"
#include "ui_screens.h"
#include "ui_widgets.h"

void Task_UI(void *pvParameters) {
    display_init();
    data_init(g_state);
    screens_init();
    
    int frames = 0;
    int current_fps = 0;
    uint32_t last_time = millis();
    
    for (;;) {
        // Update data simulation
        data_lock(g_state);
        g_state.sensor.temp[0] += 0.02f;
        if(g_state.sensor.temp[0] > 30.0f) g_state.sensor.temp[0] = 20.0f;
        g_state.sensor.lux_avg = 400.0f + random(0, 50);
        data_unlock(g_state);

        // Render UI to canvas sprite
        screens_render(g_state, current_fps);
        
        // Push to physical LCD (DMA)
        canvas.pushSprite(0, 0);
        
        frames++;
        if (millis() - last_time >= 1000) {
            current_fps = frames;
            frames = 0;
            last_time = millis();
        }
        
        // Near-zero delay to let other tasks breathe but prioritize UI
        vTaskDelay(pdMS_TO_TICKS(1)); 
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Smart Building Master - S3 Booting UI (High FPS Optimization)...");

    xTaskCreatePinnedToCore(
        Task_UI,
        "Task_UI",
        8192,
        NULL,
        3,
        NULL,
        1
    );
}

void loop() {
    vTaskDelete(NULL);
}
