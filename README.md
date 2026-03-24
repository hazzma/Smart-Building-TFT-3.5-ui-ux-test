# Smart Building TFT 3.5 UI/UX Optimized 🚀

Project Smart Building Master berbasis **ESP32-S3 (N16R8)** dengan display **3.5" TFT (ILI9488)** menggunakan library **LovyanGFX**. Project ini fokus pada performa HMI yang ultra-smooth dan estetika premium.

## 📊 Performance Matrix (Peak Optimized)
*   **Resolution:** 480x320 (Landscape)
*   **Bus Interface:** 8-bit Parallel I80
*   **Bus Frequency:** 30MHz (Stable via Jumper Wires)
*   **Frame Rate (Actual):** **79 FPS (Avg)** / **88 FPS (Peak)**
*   **Strategy:** Dual PSRAM Sprite DMA Overlapping

## 🚀 How we hit 80+ FPS at only 30MHz?
Peningkatan performa drastis dari 53 FPS ke 88 FPS dicapai melalui optimasi arsitektur perangkat lunak:

1.  **Dual-Core RTOS Architecture:** 
    - **Core 1 (Task_UI):** Dikhususkan penuh untuk rendering dengan prioritas tertinggi (Priority 4).
    - **Core 0 (Task_Net):** Menangani proses background (WiFi/Slave) agar tidak menginterupsi proses drawing.
2.  **PSRAM Double Buffering (615KB):** 
    - Mengalokasikan dua buah sprite buffer 16-bit (`canvas0` & `canvas1`) di PSRAM. 
    - Ukuran per buffer: 480 x 320 x 2 bytes = 307.2 KB.
3.  **DMA Render-While-Push Overlapping:**
    - Menggunakan teknik pengerjaan pararel: Saat `canvas0` sedang dikirim ke LCD oleh hardware DMA (Latar belakang), CPU langsung me-render frame berikutnya ke `canvas1`.
    - Ini menghilangkan *Transfer Latency* (10.2ms) dari total waktu siklus frame.
4.  **Optimized Bus Timing:** 
    - Frekuensi 30MHz dipilih sebagai "Sweet Spot" untuk menjaga kejernihan sinyal pada kabel jumper tanpa korupsi data, namun tetap memberikan bandwidth yang cukup untuk 80+ FPS.

## 🛠️ Kendala & Solusi (History)
1.  **Display Orientation:** Native panel offset dikoreksi untuk resolusi 480x320.
2.  **Touchscreen Conflict:** Resolusi konflik IOMUX pada shared pins antara Parallel Bus dan Resistive Touch.
3.  **Stability:** Implementasi stack 16KB pada Task UI untuk mencegah crash saat render widget kompleks.

---
*Developed for Smart Building Infrastructure Project.*
