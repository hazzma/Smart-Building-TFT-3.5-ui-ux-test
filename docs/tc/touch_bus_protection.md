# Troubleshooting & Touchscreen (TC) Documentation
**Project:** Smart Building Master S3
**Feature:** Display Protection & Touch Coordination

## 1. Problem Description
Pada hardware ESP32-S3 dengan Parallel Bus 8-bit, pin touchscreen (`XP, XM, YP, YM`) berbagi GPIO yang sama dengan jalur data dan kontrol LCD. Masalah muncul saat "spam click", di mana pembacaan ADC touch menginterupsi siklus penulisan data ke LCD, menyebabkan sinkronisasi ILI9488 pecah (gambar bergeser atau penuh noise).

## 2. Solution: Bus Locking
Saya menambahkan "pagar" di level driver menggunakan `tft.endWrite()` dan `tft.startWrite()`.

### Code Snippet: `src/touch.cpp`
```cpp
bool touch_get_point(int &tx, int &ty) {
    // 1. Paksa driver TFT melepaskan Bus (CS set ke HIGH)
    tft.endWrite(); 
    
    // 2. Lakukan pembacaan ADC Touch (Pin shared dalam mode input/analog)
    TouchPoint p = touchAgent.getPoint(480, 320, 1);
    
    // 3. Kembalikan kontrol Bus ke driver TFT
    tft.startWrite(); 
    
    if (p.pressed) {
        tx = p.x; ty = p.y;
        return true;
    }
    return false;
}
```

## 3. Solution: Sequential Tasking
Mengatur agar pergerakan data di `Task_UI` tidak saling tabrak.

### Code Snippet: `src/main.cpp`
```cpp
for (;;) {
    // TAHAP 1: Render visual ke buffer (CPU only)
    screens_render(g_state, current_fps);
    
    // TAHAP 2: Pengiriman data ke hardware LCD (Bus busy)
    canvas.pushSprite(0, 0);
    
    // Linker swap
    widgets_swap();

    // TAHAP 3: Polling Touch (Hanya setelah pengiriman LCD selesai)
    if (millis() - last_touch_time > 20) {
        last_touch_time = millis();
        // Memanggil fungsi yang sudah di-protect di atas
        if (touch_get_point(tx, ty)) {
            screens_handle_touch(g_state, tx, ty);
        }
    }
    ...
}
```

## 4. Result
- **Stability**: Masalah korupsi gambar hilang total meskipun layar ditekan secara agresif.
- **Compatibility**: Solusi ini bekerja sempurna dengan `TFT_eSPI` karena kita menggunakan API internal driver untuk mengatur state `CS`.
- **UI Performance**: Tetap menjaga FPS tinggi karena pembacaan touch dilakukan di "idle time" antar frame.

---
*Dokumentasi ini dibuat secara otomatis oleh TC Agent.*
