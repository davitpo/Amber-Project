# 🟠 Amber Project Journal

## Entry 001: The Genesis of Amber
**Date:** March 2026  
**Author:** David

The idea for the Amber Project arrived late one evening behind the wheel of an E36. 

Looking at the dashboard, bathed in that soothing, timeless amber glow, I realized how perfectly BMW had designed the cabin ergonomics of the nineties. It is a driver-centric sanctuary. Yet, the passing of three decades has left certain gaps. Modern devices are distracting; mounting a modern smartphone or a glossy aftermarket screen breaks the design harmony of the cockpit.

That's when it clicked. Why not build an instrument that bridges this gap? Modern technology with a classic BMW soul. 

### Why "00:36"?
When the first prototype screen booted up, displaying a classic, minimalist analog clock with an amber sweep second hand, the time was set to 00:36. A subtle nod to the E36 chassis that inspired it. It represents the bridge between old and new—the precise moment where classic analog aesthetics meet modern digital control.

### The Vision
The first repository was initialized with a clear target: build a hardware module that drops perfectly into place, perhaps replacing an analog clock or an empty storage cubby, providing customizable dashboard extensions, Bluetooth connectivity, and offline-first telemetry while looking 100% factory original.

---

## [Future Entries Space]
*Journal entries will continue here as milestones are reached and the architecture matures.*

## 2026-07-15 — Android Toolchain Note

Interesting Android ecosystem observation:

AndroidX libraries may require a newer compileSdk than the current
Android Gradle Plugin officially supports.

Example encountered:

androidx.activity:activity:1.13.0
→ requires compileSdk >= 36

androidx.core:core-ktx:1.19.0
→ requires compileSdk >= 37
→ also requires Android Gradle Plugin 9.1+

Current Amber baseline:

AGP          8.10.1
compileSdk   35
targetSdk    35
minSdk       24

Decision:
Pin AndroidX dependencies to stable versions compatible with the current
toolchain instead of upgrading the entire Android SDK/AGP stack during M9.

Lesson learned:
A newer library version is not always the better choice for a stable project baseline.