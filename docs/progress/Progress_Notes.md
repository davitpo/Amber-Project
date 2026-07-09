2026-07-10

Amber transitioned from direct TFT rendering to a framebuffer-based
rendering pipeline using LovyanGFX sprites.

Rendering now operates at approximately 30 FPS while the logical
clock continues ticking at 1 Hz.

Runtime diagnostics were introduced, including render mode,
free heap, minimum heap and frame rate monitoring.