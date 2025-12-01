![Version](https://img.shields.io/badge/version-2.0-blue) ![Platform](https://img.shields.io/badge/platform-Arduino-teal) ![License](https://img.shields.io/badge/license-MIT-green)

# Pro Micro Macro Board

An advanced 3-button macro keyboard script for the SparkFun Pro Micro (ATmega32U4). It features chorded inputs, togglable media controls, and mode persistence without blocking the CPU.

**Author:** MWDiss  
**Date:** Dec 1, 2025

## Features

*   **Zero Delay:** Uses non-blocking logic (`millis`) for instant response.
*   **Context Modes:** Switch between Productivity and Media modes.
*   **Chord Support:** Press multiple keys to unlock hidden functions (Clipboard history).
*   **Visual Feedback:** Flashes your main keyboard's Scroll Lock light when switching modes.

## Controls

### Normal Mode (Productivity)

| Button | Action | Hold (0.6s) |
| :--- | :--- | :--- |
| **Pin 16** | Copy `Ctrl+C` | Toggle **Media Mode** |
| **Pin 10** | Paste `Ctrl+V` | *(None)* |
| **Pin 18** | Plain Paste `Ctrl+Shift+V`* | Toggle key to **Select All** |

*\*Note: Pin 18 toggles between "Plain Paste" and "Select All" when held.*

### Media Mode

| Button | Action |
| :--- | :--- |
| **Pin 16** | Play / Pause |
| **Pin 10** | Volume Down |
| **Pin 18** | Volume Up |

### Global Combo

| Combination | Action |
| :--- | :--- |
| **Pin 10 + Pin 18** | Open Windows Clipboard `Win + V` |

## Configuration

You can easily adjust settings at the top of the `.ino` file

```cpp
// Change which pins your buttons use
const int PIN_COPY  = 16;

// Adjust how long to hold to switch modes (default 600ms)
const int HOLD_TIME = 600;

// Turn off scroll lock flashing if annoying
const bool USE_SCROLL_LOCK = true; 
```

## Setup & Installation

1.  **Install Arduino IDE:** [Download here](https://www.arduino.cc/en/software).
2.  **Add HID-Project:**
    *   Go to `Tools` > `Manage Libraries...`
    *   Search for **HID-Project** by NicoHood.
    *   Install it.
3.  **Select Board:**
    *   Tools > Board > SparkFun AVR Boards > **SparkFun Pro Micro**.
4.  **Upload:** Connect via USB and click the Upload arrow.

### Troubleshooting Uploads
If the upload fails or the board isn't found
1.  Double-tap the **GND** and **RST** pins with a wire.
2.  Select the new COM port that appears.
3.  Hit Upload immediately.
