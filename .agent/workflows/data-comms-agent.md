---
description: "Data & Communication Agent - Focuses on local state management and slave protocol abstraction"
---

# Data & Communication Agent

This agent is responsible for the core data architecture and the communication link between Master and Slave devices.

## Core Responsibilities
- Manage `data.cpp`, `data.h`, `comms.cpp`, and `comms.h`.
- Implement and maintain the `BuildingState` struct (Single Source of Truth).
- Abstract slave communication protocols (Modbus RTU, CAN Bus, or Dummy).
- Ensure thread-safe data access using RTOS Mutex.

## Development Workflow
1. **Data Architecture**:
   - Implement `data.cpp` with the `BuildingState` struct as per FSD 4.5.
   - Initialize the `mutex` for safe access between UI, Comms, and Firebase tasks.
2. **Protocol Abstraction**:
   - Build `comms.cpp` using the **Protocol Adapter** pattern (FSD 6.1).
   - Implement `comms_poll()` to gather data from Slaves and update `BuildingState`.
   - Provide clean setter functions: `comms_send_ac`, `comms_send_temp_target`, etc.
3. **Slave Handling**:
   - Manage the slave list and online/offline status (`slave_online[2]`).
   - Implement dummy data simulation for development when hardware is not present.
4. **Documentation (MANDATORY)**:
   - For every major feature or protocol change, create a markdown file in `docs/architecture/` (e.g., `modbus_mapping.md`).
   - Document the register/ID mapping for slaves to make future slave code development easier.

## Guidelines
- **Thread Safety**: Never touch `BuildingState` without `data_lock()`.
- **Modularity**: The `comms` layer must be swappable without affecting the UI.
- **Fail-Safe**: Implement timeouts for slave responses to prevent task hanging.
- **FSD Alignment**: Ensure `SensorData` fields match the FSD 4.5 specification exactly.
