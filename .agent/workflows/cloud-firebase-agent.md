---
description: "Cloud & Firebase Agent - Focuses on Firebase Realtime DB, push/pull data, and cloud sync"
---

# Cloud & Firebase Agent

This agent is responsible for the bridge between the Master device and the Flutter app via Firebase.

## Core Responsibilities
- Manage `firebase.cpp`, `firebase.h`, and `Task_Firebase` (Core 0, Priority 1).
- Synchronize `BuildingState` (SensorData) to the cloud.
- Process incoming commands from the Flutter app via the `/commands/` path.
- Handle Firebase authentication and connectivity.

## Development Workflow
1. **Cloud Sync Logic**:
   - Implement `firebase_init()` with API keys and database URL.
   - Build `firebase_push()` to upload `SensorData` to `/buildings/{room_name}/sensors/` (FSD 4.18).
2. **Command Processing**:
   - Implement `firebase_pull_commands()` to read commands from `/commands/` (FSD 4.30).
   - Once a command is processed (via `comms.cpp`), **delete the command node** to prevent re-execution.
3. **Connectivity Management**:
   - Monitor `firebase_ok` status and report to the `error_handler` if sync fails.
   - Ensure `Task_Firebase` doesn't starve other tasks (it should have lower priority).
4. **Documentation (MANDATORY)**:
   - Create and maintain `docs/cloud/firebase_schema.json` to document the database structure.
   - Document how the Flutter app should write to the `/commands/` path.

## Guidelines
- **Zero Interruption**: Firebase sync should not affect the UI or Slave polling.
- **Resource Management**: Only push data when changes occur or at a fixed interval (e.g., every 5-10s).
- **Security**: Hardcode only non-sensitive demo keys; use `settings.cpp` for user-defined configuration.
- **FSD Alignment**: Follow Section 4.7 for JSON path structure.
