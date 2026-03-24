# Smart Building TFT 3.5 ui/ux test

Project Smart Building Master berbasis **ESP32-S3 (N16R8)** dengan display **3.5" TFT (ILI9488)** menggunakan library **LovyanGFX**.

## 📊 Performance Matrix
*   **Resolution:** 480x320 (Landscape)
*   **Bus Interface:** 8-bit Parallel I80
*   **Bus Frequency:** 30MHz (Optimized)
*   **Frame Rate:** **88 FPS** (Measured Peak Performance)
*   **Strategy:** PSRAM Double Buffering + DMA Overlapping (Non-blocking render)
*   **Stability:** High (Verified at 30MHz Bus Frequency)

## 🛠️ Kendala & Solusi (History)
1.  **Display Orientation:** Awalnya muncul portrait dan terpotong. Diatasi dengan koreksi native panel offset dan `setRotation(1)` yang presisi.
2.  **Rendering Artifacts:** Sempat terjadi flickering dan gambar pecah. Diatasi dengan implementasi Double Buffering menggunakan `LGFX_Sprite` yang dialokasikan di **PSRAM**.
3.  **Touchscreen Conflict:** Upaya implementasi *Shared Pins* menggunakan library `Adafruit_TouchScreen` menyebabkan crash (0 FPS) akibat korupsi bus Parallel.
4.  **Optimasi FPS:** Frekuensi bus ditingkatkan dari 20MHz ke 30MHz, dan task UI dioptimalkan dengan delay minimum untuk mencapai throughput maksimal hardware.

## 🚀 Status Implementasi
- [x] High-Speed Parallel Bus Setup
- [x] Dashboard UI (Modern Dark Theme)
- [x] Real-time Sensor Data Simulation
- [x] PSRAM DMA Sprite Buffering
- [ ] Resistive Touch Integration (Pending next phase)

---
*Developed for Smart Building Infrastructure Project.*
