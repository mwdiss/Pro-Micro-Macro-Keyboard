![Version](https://img.shields.io/badge/version-2.1-blue) ![Platform](https://img.shields.io/badge/platform-Arduino-teal) ![License](https://img.shields.io/badge/license-MIT-green)

# Pro Micro Macro Board

An advanced 3-button macro keyboard script for the SparkFun Pro Micro (ATmega32U4). V2.1 features hyper-responsive inputs, repeating volume keys, and context-aware layering.

**Author:** MWDiss  
**Date:** Dec 1, 2025

## Features

*   **Fast Response:** Optimized with minimal debounce for fast key taps.
*   **Media Mode:** Hold a key to switch layer, granting access to music/volume controls.
*   **Auto-Repeat:** Volume keys repeat when held (just like a standard keyboard).
*   **Zero CPU Load:** Non-blocking code ensures no PC slowdowns.
*   **Visual Feedback:** Flashes `Scroll Lock` on mode change.

## Controls

### Normal Mode (Productivity)

| Button | Pin | Single Tap | Hold (0.6s) |
| :--- | :--- | :--- | :--- |
| **Btn 1** | 16 | Copy `Ctrl+C` | Toggle **Media Mode** |
| **Btn 2** | 10 | Paste `Ctrl+V` | Open **Clipboard** `Win+V` |
| **Btn 3** | 18 | Plain Paste* | Toggle Function `Plain/SelectAll` |

*\*Note: Button 3 can be toggled to perform `Ctrl+A` (Select All) by holding it for 0.6s.*

### Media Mode

| Button | Action | Behavior |
| :--- | :--- | :--- |
| **Btn 1** | Play / Pause | Action on Release |
| **Btn 2** | Volume Down | **Repeat on Hold** |
| **Btn 3** | Volume Up | **Repeat on Hold** |

## Configuration

Adjust settings at the top of `macro_board_v2_1.ino`

```cpp
const int HOLD_TIME = 600;      // Time to hold before special action triggers
const int DEBOUNCE  = 10;       // Lower = more responsive, Higher = safe from noise
const bool USE_SCROLL_LOCK = true; // Set false to disable light flashing
```

## Setup & Troubleshooting

1.  **Requirement:** Install **HID-Project** library by NicoHood in Arduino IDE (`Tools > Manage Libraries`).
2.  **Upload Failed?** Double-tap `GND` + `RST` pins on the board to reset the bootloader, then quickly upload.
3.  **Buttons reversed?** Update `const int PIN_xxx` variables at the top of the file.
