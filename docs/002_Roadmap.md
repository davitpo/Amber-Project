# 🟠 Amber Project Roadmap

## Phase 0: Foundation
* **Goal:** Establish core documentation, code style, repository structure, and early prototyping targets.
* **Key Tasks:**
  * Define UI styling principles (Classic BMW typography, color palettes).
  * Select base development hardware and establish PlatformIO boilerplate environment.
  * Define initial BLE protocol scope and message structures.

## Phase 1: Clock
* **Goal:** Implement a highly polished, OEM-looking standalone digital/analog clock module.
* **Key Tasks:**
  * Design classic BMW clock faces using LVGL (Analog and Digital styles).
  * Leverage offline timekeeping using hardware RTC or internal timers.
  * Create smooth 1990s-style transitions and subtle glowing animations.

## Phase 2: BLE (Bluetooth Low Energy)
* **Goal:** Establish standard event-driven communication standard.
* **Key Tasks:**
  * Refine and implement the BLE protocol draft.
  * Enable time-synchronization over BLE.
  * Develop lightweight, event-driven service structure for state changes without polling.

## Phase 3: Android Companion
* **Goal:** Develop a seamless, companion Android app that integrates cleanly with the hardware module.
* **Key Tasks:**
  * Establish Android Kotlin project structure.
  * Integrate BLE communication framework.
  * Add navigation routing telemetry placeholder (Yandex Maps integration) to send basic cues to the dash screen.

## Phase 4: Vehicle Integration
* **Goal:** Connect directly to car telemetry and physical interfaces.
* **Key Tasks:**
  * Research physical mounting and integration into E36 dashboard slots (e.g. replacing analog clock or pocket).
  * Design optional vehicle bus connection (e.g., K-Line, I-Bus, or CAN-bus integrations) for real-time diagnostics and status.

## Phase 5: Future Ideas
* **Goal:** Long-term visions and community-suggested enhancements.
* **Key Tasks:**
  * Multi-sensor aux telemetry dashboard (oil temp, pressure, battery voltage).
  * Support for external expansion accessories (physical buttons, rotary dials).
  * Custom retro-fitting enclosures for other classic BMW chassis models.
