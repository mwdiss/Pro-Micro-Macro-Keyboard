# Pro Micro Macro Board

![Version](https://img.shields.io/badge/version-3.4-blue) ![Platform](https://img.shields.io/badge/platform-Arduino-teal) ![License](https://img.shields.io/badge/license-MIT-green)

A professional, low-latency 3-button macro keyboard designed for the **SparkFun Pro Micro (ATmega32U4)**. Featuring distinct handling for **instantaneous modifiers** (Ctrl/Shift) and **intelligent macro keys** to ensure zero ghosting. V3.4 introduces optimized repeater logic for list navigation and volume control.

**Author:** MWDiss  
**Date:** Jan 2026

---

## Key Features

*   **Zero-Latency Modifiers:** Normal Layer Ctrl/Shift operate via direct pass-through for gaming/shortcuts.
*   **Intelligent Auto-Repeat:** 
    *   Media Volume and Arrow keys tap once on press, then wait 0.6s before repeating, allowing for both precise clicks and fast scrubbing.
*   **Dual-Layer Mode:** Toggle between `Productivity` and `Media` layers.
*   **Mode-Aware Keys:** Ctrl/Shift morph into repeating Arrow keys when in Media Mode.
*   **Win+Tab Interrupter:** Hold Enter (0.8s) to trigger Task View.

---

## Layout Mappings

### Default Layer (Productivity)

| Button | Pin | Tap Behavior (Release) | Hold (0.6s) |
| :--- | :--- | :--- | :--- |
| **Copy** | 14 | `Ctrl + C` | Toggle Layer (**Media**) |
| **Paste** | 16 | `Ctrl + V` | Open **Clipboard** (`Win+V`) |
| **Alt** | 18 | Plain Paste / Select All* | Toggle Config (`Select All`) |
| **Enter** | 20 | `Enter` (Instant) | `Win + Tab` (Task View) |
| **Ctrl** | 7 | `Ctrl` (Holdable) | - |
| **Shift** | 5 | `Shift` (Holdable) | - |

*\*Note: Hold Button 3 to toggle between Plain Paste (`Ctrl+Shift+V`) and Select All (`Ctrl+A`).*

### Media Layer

| Button | Action | Hold Behavior (0.6s delay) |
| :--- | :--- | :--- |
| **Copy** | Play / Pause | Action on Release |
| **Paste** | Volume Down | **Auto-Repeats** |
| **Alt** | Volume Up | **Auto-Repeats** |
| **Enter** | Mute | Action on Release |
| **Ctrl** | `Down Arrow` | **Auto-Repeats** |
| **Shift** | `Up Arrow` | **Auto-Repeats** |

---

## Configuration

Adjust behavior in `macro_board_v3_4.ino`:

```cpp
// Timings
const int HOLD_TIME_MACRO = 600;  // Time to hold to swap layers
const int REPEAT_DELAY    = 600;  // Delay before Volume/Arrow keys start spamming
const int REPEAT_RATE     = 100;  // Speed of spamming (lower is faster)

// Visuals
const bool USE_SCROLL_LOCK = true; 
```

## Hardware Hookup

Connect each switch between the designated **Pin** and the **GND** pin.

| Button | Pins | Note |
| :--- | :--- | :--- |
| **Macro Copy** | Pin 14 | Digital Pin |
| **Macro Paste** | Pin 16 | Digital Pin |
| **Macro Alt** | Pin 18 | Analog A0 |
| **Fast Enter** | Pin 20 | Analog A2 |
| **Fast Ctrl** | Pin 7 | Digital Pin |
| **Fast Shift** | Pin 5 | Digital Pin |
