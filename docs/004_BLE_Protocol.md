# 🟠 Amber Project BLE Protocol Draft

## Message Philosophy

* **Event-Driven:** Data is only transmitted when state changes. Polling is strictly prohibited to optimize power consumption and bandwidth.
* **Minimal Payload:** Keep structures packed, clear, and tiny. Prefer raw byte values over bloated strings or JSON objects.
* **Robust Fail-Safe:** If connection drops, the hardware falls back immediately to offline mode without interrupting the screen or locking up input controls.

---

## Versioning Strategy

To guarantee backwards and forwards compatibility as our software evolves:
1. Every connection cycle starts with a handshake where both devices declare their protocol version.
2. The version byte format uses simple major/minor schema (`0x[Major][Minor]` - e.g., `0x10` for `v1.0`).
3. If versions do not match or aren't compatible, the ESP32 degrades gracefully to legacy modes or reports an incompatibility symbol on-screen.

---

## Reserved UUID Ranges

The Amber Project reserves custom UUID allocations to distinct services and characteristics.

* **Primary Custom Service Base UUID:** `E36B2026-XXXX-499B-89E6-D983DE62CDFA`

| Local UUID Range (16-bit space) | Category | Description / Purpose |
| :--- | :--- | :--- |
| `0x1000` - `0x10FF` | **System & Sync** | Version Handshakes, RTC Time Sync, Settings |
| `0x2000` - `0x20FF` | **Telemetry** | Speed, RPM, Coolant temperature, and fuel logs |
| `0x3000` - `0x30FF` | **Navigation** | Turn direction, distance instructions, arrival time |
| `0x4000` - `0x40FF` | **Media & Audio** | Track name, artists, play state, active notifications |

*Specific characteristic registers and bit-wise packet configurations will be defined during Phase 2.*

---

# Amber Command Protocol

Version 1

## Command Syntax
All messages sent via the Amber Command Protocol (ACP) v1 must be formatted in clear ASCII text, with trailing or leading spaces and carriage-return/linefeed (CR/LF) indicators fully trimmed on receipt. Commands are case-insensitive.

Format structure: `COMMAND` or `COMMAND=parameter_value` up to 64 bytes.

---

## Validation Rules
*   **PING**: Zero parameter format validation. Response payload is `PONG`.
*   **VERSION**: Zero parameter format validation. Response payload is version text.
*   **GETTIME**: Not implemented yet.
*   **STATUS**: Not implemented yet.
*   **GETBRIGHTNESS**: Not implemented yet.
*   **SETTIME**: Parameter format `hh:mm:ss` (strictly 8 characters, with colons separating fields). Hour must fall in standard range `0..23`, minute and second fields are checked between bounds `0..59`. Execution is marked NOT IMPLEMENTED yet.
*   **BRIGHTNESS**: Parameter format is a numerical integer between bounds `0..100`. Fractional, negative, or format strings with alphabetic descriptors are rejected. Execution is marked NOT IMPLEMENTED yet.

---

## Response Format
*   `OK`: Action processed successfully.
*   `PONG`: Reply validation response.
*   `ERROR=UNKNOWN_COMMAND`: The inputted command keyword does not exist or matches nothing.
*   `ERROR=INVALID_FORMAT`: Argument formats or character descriptors mismatch the specified rules.
*   `ERROR=OUT_OF_RANGE`: Integer inputs exceed validation boundaries.

---

## Examples
*   `PING` -> `PONG`
*   `SETTIME=12:36:45` -> `OK` (Execution: NOT_IMPLEMENTED)
*   `SETTIME=25:00:00` -> `ERROR=OUT_OF_RANGE`
*   `BRIGHTNESS=80` -> `OK` (Execution: NOT_IMPLEMENTED)
*   `BRIGHTNESS=-5` -> `ERROR=INVALID_FORMAT`
