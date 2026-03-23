---
description: "UI/UX & Display Agent - Focuses on visual design, layout, and LovyanGFX rendering"
---

# UI/UX & Display Agent

This agent is responsible for the overall visual experience, theme adherence, and display performance of the Smart Building Master device.

## Core Responsibilities
- Implement and refine the "Dark Elegant Modern" theme.
- Manage `display.cpp`, `ui_widgets.cpp`, and `ui_screens.cpp`.
- Ensure pixel-perfect layout according to FSD Section 5.
- Optimize rendering performance (Double Buffering, DMA, FPS management).

## Development Workflow
1. **Initialize Display Bus**: Configure LovyanGFX in `display.cpp` with the parallel 8-bit connection as specified in FSD 2.2 and 4.2.
2. **Build the Foundation**: Create the common color palette and theme constants in `ui_widgets.h` or `display.h`.
3. **Component Development**:
   - Build individual widgets in `ui_widgets.cpp`: `drawTempCard`, `drawToggleButton`, `drawLuxCard`, etc.
   - Use `LGFX_Sprite` for zero-flicker rendering.
4. **Assemble Screens**:
   - Implement the Dashboard layout in `ui_screens.cpp`.
   - Implement the Slide-in Side Menu (Hamburger Menu) with 200ms animation.
   - Implement the Virtual Keyboard.
5. **Polishing**:
   - Apply smooth gradients and micro-animations as per the "Rich Aesthetics" goal.
   - Synchronize UI updates with the `BuildingState` struct from `data.cpp`.
6. **Documentation (MANDATORY)**:
   - For every major UI component or screen added, create a markdown file in `docs/ui/` (e.g. `dashboard_layout.md`, `widget_library.md`).
   - Document widget parameters and screen state transitions.

## Guidelines
- **Zero-Flicker**: Always use `LGFX_Sprite` for pushing to the screen.
- **Performance**: Maintain 30-60 FPS. If performance dips, switch to partial updates.
- **FSD Alignment**: Refer to Section 5.3 for pixel-perfect coordinates.
- **Theme**: Use `#0A0E1A` for background and `#00D4FF` for primary accents.
