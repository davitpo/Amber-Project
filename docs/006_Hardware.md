# 🟠 Amber Project - Hardware Configuration

## Baseline Hardware

The current development platform is built around the highly versatile **ESP32-2424S012C-I** module. This is a compact, reliable core that fits neatly into classic round gauges or custom-molded dashboard button panels.

* **Key Specifications:**
  * **Processor:** ESP32-S3 (Dual-core CPU, integrated Wi-Fi + BLE 5.0).
  * **Display:** 1.2-inch circular IPS display with vibrant color rendering.
  * **Touch Layer:** Integrated capacitive touch interface for tactile inputs.
  * **Power Supply:** Onboard lithium battery storage management circuit.
  * **Data Interface:** Single USB-C port for power delivery, debugging terminal, and flashing firmware.

---

## 1. Electrical Integration

* Consoles provide 12V automotive power lines, requiring robust filtering and step-down converters to 5V/3.3V.
* **Fully Reversible Warning:** Installers should use piggyback fuse-taps or split standard harnesses rather than splicing original main lines.

---

## 2. Hardware Independence

Although the project starts with the ESP32-2424S012C-I, the underlying software and UX layout must remain strictly decoupled from specific hardware platforms.
* **Portability:** Keep screen layouts relative to percentage-based layouts rather than static pixel targets where possible, or structure abstract drivers.
* **Modularity:** Future PCB iterations or different circular screen form-factors (different sizes/resolutions) must run with minimal adjustments to the core firmware code.
