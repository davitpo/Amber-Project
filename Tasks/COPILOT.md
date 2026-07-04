# 🟠 Amber Project

Welcome to the Amber Project.

You are joining an engineering project whose goal is much larger than writing code.

Amber Project is an open-source platform inspired by the timeless amber instrument illumination of classic BMW vehicles.

Our mission is simple:

> Modern Technology. Classic BMW Soul.

---

## Philosophy

This project is NOT about making an old BMW look modern.

It is about building the features BMW engineers might have designed in the late 1990s if today's technology had existed.

Every design decision should respect the original vehicle.

If something looks flashy or aftermarket, it probably does not belong here.

---

## Project Principles

1. OEM First
   Every screen, animation and interaction should feel like factory equipment.

2. Fully Reversible
   Nothing should require permanent modifications to the vehicle.

3. Open Source
   Everything should be documented and understandable.

4. Modular
   Features should be independent whenever possible.

5. Offline First
   Core functionality must not depend on Internet connectivity.

6. Quality over Quantity
   One perfectly implemented feature is preferred over ten unfinished ones.

7. Simplicity
   Elegant solutions are preferred over clever ones.

---

## Architecture

Repository layout

firmware/
    ESP32 firmware
    PlatformIO
    LVGL
    BLE

android/
    Kotlin Android application
    BLE communication
    Yandex Maps integration
    Car launcher

docs/
    Documentation
    Design decisions
    BLE protocol
    Roadmap

hardware/
    PCB
    Mechanical parts
    3D models

assets/
    Fonts
    Icons
    Clock faces
    Images

---

## Coding Style

Prefer readable code.

Prefer descriptive names.

Avoid giant classes.

Avoid giant functions.

Document WHY instead of WHAT.

Comments should explain decisions, not syntax.

Every module should have a single responsibility.

---

## UI Style

The UI should resemble classic BMW dashboards.

Primary colors

Black
Amber
White

Avoid

RGB effects

Large gradients

Glass effects

Neon colors

Overly modern UI elements

Animations should be subtle and purposeful.

---

## BLE

BLE communication should be event driven.

Avoid polling whenever possible.

Protocols should be versioned.

Document every characteristic.

---

## Documentation

Every significant design decision should be documented.

Future contributors should understand why something exists.

---

## The Team

David
Project Creator
Hardware
Electronics
Testing
Vision

ChatGPT
Architecture
Firmware Design
Android Architecture
Documentation
Design Reviews

GitHub Copilot
Implementation
Boilerplate
Refactoring
Testing assistance

---

## Motto

Modern Technology.

Classic BMW Soul.

---

## One final rule

Before implementing any feature ask yourself:

"Would a BMW engineer from 1998 smile if they saw this in 2026?"

If the answer is yes,
you're probably on the right path.