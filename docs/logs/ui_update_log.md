# UI Development Log - Master S3

## [2026-04-20 18:15] - Hamburger Menu & Settings UI
**Status**: Interactive UI Created (Placeholder logic)

### Changes Made:
- **Hamburger Icon**: Added a 3-line interactive icon in the `drawNotifBar` (Top-Left corner).
- **Settings Screen**: Created `SCREEN_SETTINGS` with options for WiFi and LAN configuration.
- **Navigation**:
    - Dashboard -> Settings (Clicking hamburger icon)
    - Settings -> Dashboard (Clicking BACK button)
- **Touch Areas**:
    - Hamburger: `(0, 0, 80, 50)` for easy access.
    - Settings Options: Full-width cards for better touch response.

### Future Works:
- **WiFi Config**: Implement a screen to scan networks and input passwords via on-screen keyboard.
- **LAN Config**: Implement static IP vs DHCP toggle.
- **Persistence**: Save selection to Preferences/NVS so it survives reboot.
- **Visual Polish**: Add transition animations between screens.
