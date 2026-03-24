#include <Arduino.h>
#include "display.h"
#include "data.h"
#include "ui_screens.h"
#include "ui_widgets.h"

// ─────────────────────────────────────────────────────────────────
// Task_Net: Core 0, Low Priority — urusan network & slave
// ─────────────────────────────────────────────────────────────────
void Task_Net(void *pvParameters) {
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(500)); 
    }
}

// ─────────────────────────────────────────────────────────────────
// Task_UI: Core 1, High Priority — rendering display
// ─────────────────────────────────────────────────────────────────
void Task_UI(void *pvParameters) {
    int frames = 0;
    int current_fps = 0;
    uint32_t last_time = millis();
    
    for (;;) {
        // Update dummy sensor data
        data_lock(g_state);
        g_state.sensor.temp[0] += 0.02f;
        if (g_state.sensor.temp[0] > 30.0f) g_state.sensor.temp[0] = 20.0f;
        g_state.sensor.lux_avg = 400.0f + random(0, 50);
        data_unlock(g_state);

        // 1. Render UI to the CURRENT inactive canvas
        screens_render(g_state, current_fps);
        
        // 2. Perform the buffer swap and push the RENDERED buffer
        // Note: LovyanGFX pushSprite will automatically wait for any 
        // previous DMA transfer from the OTHER buffer to finish.
        canvas.pushSprite(0, 0);
        
        // 3. Swap to the other buffer for the next frame's drawing
        widgets_swap();
        
        // FPS counter
        frames++;
        if (millis() - last_time >= 1000) {
            current_fps = frames;
            frames = 0;
            last_time = millis();
            Serial.printf("[UI] FPS: %d\n", current_fps);
        }
        
        // Use vTaskDelay(0) or no delay if we want absolute MAX speed
        // but 1ms delay is safer to avoid WDT triggers on Core 1 if other system tasks exist.
        vTaskDelay(pdMS_TO_TICKS(1)); 
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("Smart Building Master - S3 Booting...");

    // 1. Initialize hardware and state (Must be BEFORE tasks start)
    display_init();
    data_init(g_state);
    screens_init();

    // 2. Start RTOS tasks
    xTaskCreatePinnedToCore(
        Task_Net, "Task_Net", 4096, NULL, 1, NULL, 0
    );

    xTaskCreatePinnedToCore(
        Task_UI, "Task_UI", 16384, NULL, 4, NULL, 1
    );
}

void loop() {
    vTaskDelete(NULL);
}
