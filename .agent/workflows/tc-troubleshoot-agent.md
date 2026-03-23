---
description: "Troubleshooting & Touchscreen (TC) Agent - Focuses on resistive touch, calibration, and hardware health"
---

# Troubleshooting & Touchscreen (TC) Agent

This agent is responsible for the touchscreen implementation, calibration, and overall system status management.

## Core Responsibilities
- Implement and maintain `touch.cpp` and `touch.h`.
- Address specific shared-pin hardware challenges (ADC reading vs. TFT bus).
- Manage error handling in `error_handler.cpp` and `error_handler.h`.
- Monitor system health (WiFi, Firebase, Sensor Timeouts).

## Development Workflow
1. **Critical Configuration**:
   - Ensure `touch.h` has the correct pin mapping (XP: 7, XM: 16, YP: 15, YM: 8) as per FSD 4.8.
   - **MANDATORY**: Implement the Bus Releasing logic: `lcd.endWrite()` -> `ts.getPoint()` -> `lcd.beginWrite()`.
2. **Touch Logic Development**:
   - Create `touch_init()` and `touch_get_point()`.
   - Implement coordinate mapping from raw ADC [0-4095] to screen [480x320].
   - Implement `touch_run_calibration()` with interactive on-screen points.
3. **Troubleshooting System**:
   - Build `error_handler.cpp` to track `ErrorCode` for sensor timeouts, slave offline, and touch uncalibrated.
   - Synchronize with `ui_widgets.cpp` to render the notification bar (NotifBar) correctly.
4. **Integration Testing**:
   - Verify touch reliability in high-load scenarios (during Firebase push or Modbus polling).
   - Use `Serial.print` for real-time touch debugging when needed.
5. **NVS Persistent Storage**:
   - Implement `settings.cpp` to save and load calibration data (min/max X, min/max Y).
6. **Documentation (MANDATORY)**:
   - For every hardware-related fix or calibration logic update, create a markdown file in `docs/hardware/` (e.g., `touch_calibration.md`, `pin_mapping_troubleshooting.md`).
   - Document any specific GPIO constraints encountered during development.

## Guidelines
- **Bus Integrity**: Never read touch without releasing the TFT bus first.
- **Accuracy**: Calibration is key for resistive screens. Use 3-point or 4-point calibration.
- **Reporting**: Always report errors when touch ADC values are consistently invalid.
- **FSD Alignment**: Refer to Section 4.8 for the mandatory bus-sharing procedure.
