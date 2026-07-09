# Amber Project – Design Notes

> "Modern Technology. Classic BMW Soul."

This document describes the visual and interaction philosophy of the Amber Project.

It is not an implementation document.

It explains *how the project should feel*.

---

# 1. Design Philosophy

Amber is not a gadget.

Amber should feel like it always belonged in the car.

Every screen should appear calm, elegant and purposeful.

If a feature makes the interface more complicated than useful,
it should probably not exist.

Minimalism is preferred over decoration.

---

# 2. Clock

The clock is the heart of Amber.

The analog clock should immediately remind the user of a classic BMW dashboard,
without being a copy of the original design.

Goals:

- clean
- readable
- timeless
- elegant
- calm

Avoid visual noise.

---

# 3. Color Palette

Primary color

Amber

This is the identity of the project.

The amber color should resemble classic BMW dashboard illumination.

Background

Pure black.

The display should visually disappear inside the bezel.

Accent colors

Used only when required.

Examples:

- Bluetooth
- Navigation
- Warnings
- Notifications

The clock itself should remain monochromatic.

---

# 4. Clock Face

No numbers.

No date.

No unnecessary text.

Hour markers only.

The outer ring should be subtle.

The center hub should remain small.

The eye should naturally focus on the hands.

---

# 5. Clock Hands

Hour hand

- short
- thicker

Minute hand

- longer
- slightly thinner

Second hand

- very thin
- smooth movement

Hands should always be easy to distinguish.

---

# 6. Motion

Movement should never feel mechanical.

Animations should be smooth and quiet.

Fast animations should only be used for notifications.

The clock itself should feel alive,
not busy.

---

# 7. Boot Animation

The device should never appear to "boot".

It should wake up.

Preferred sequence:

1. Black screen
2. Center dot
3. Outer ring
4. Hour markers
5. Hands sweep to current time
6. Normal operation

---

# 8. User Interface

Every screen should answer one question.

Avoid dashboards full of information.

Information hierarchy:

1. Time
2. Driving-related information
3. Notifications
4. Settings

---

# 9. Features

Features must integrate naturally.

Examples:

✓ Phone status

✓ BLE

✓ Navigation

✓ Calendar

✓ Gate opener

✓ Music

A feature should never look like an application running on top of a clock.

It should look like it belongs there.

---

# 10. Engineering Principles

Readable code is preferred over clever code.

Architecture is preferred over shortcuts.

Every module should have a single responsibility.

Refactoring is encouraged.

Premature optimization is discouraged.

---

# 11. Guiding Principle

If someone unfamiliar with Amber looks at the display and asks:

    "Was this an original BMW option?"

then we are moving in the right direction.

# 12. The Amber Rule

Every new feature must satisfy three questions:

Does it improve the driving experience?

Does it respect the classic interior?

Can it disappear when it is not needed?

If the answer to any question is "No",

the feature should be reconsidered.