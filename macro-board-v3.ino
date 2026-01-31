/*
 * Pro Micro Macro Keyboard
 * Version: 3.4 (Repeater Logic Update)
 * Author: MWDiss
 * Date: 2026
 * 
 * CHANGES V3.4:
 * - Increased auto-repeat initial delay to 600ms (Reduced spam).
 * - Implemented software auto-repeat for Arrow Keys in Media Mode (Ctrl/Shift).
 * - Consistent repeater timing across Volume and Arrows.
 */

#include <HID-Project.h>

// --- PIN MAPPINGS ---
const int PIN_COPY  = 14; 
const int PIN_PASTE = 16; 
const int PIN_ALT   = 18; 
const int PIN_ENTER = 20;
const int PIN_CTRL  = 7;
const int PIN_SHIFT = 5;

// --- CONFIGURATION ---
const int HOLD_TIME_MACRO = 600; // Hold Macro key to swap layer
const int HOLD_TIME_ENTER = 800; // Hold Enter for Win+Tab

// Repeater Settings
const int REPEAT_DELAY = 600;    // Wait 0.6s before spamming starts
const int REPEAT_RATE  = 100;    // Spam speed (ms between clicks)

const int MIN_PRESS_TIME  = 30;  // Noise Filter
const bool USE_SCROLL_LOCK = true; 

enum Mode { MODE_NORMAL, MODE_MEDIA };
Mode currentMode = MODE_NORMAL;
bool isSelectAllMode = false;

// --- STRUCTURES ---

struct MacroButton {
  int pin;
  bool state = HIGH;          
  unsigned long pressTime = 0;
  bool ignoreRelease = false; 
  bool holdDone = false;
  unsigned long lastRepeat = 0; 
  MacroButton(int p) : pin(p) {} 
};

// Fast key states now track time for repeaters
struct FastState {
  bool active = false; 
  bool isRepeating = false; // Is this key currently in "Software Repeat" mode?
  unsigned long pressTime = 0;
  unsigned long lastRepeat = 0;
  KeyboardKeycode activeKey = KEY_RESERVED; 
};

struct EnterState {
  bool state = HIGH;
  unsigned long pressTime = 0;
  bool holdTriggered = false; 
  int activeModeKey = 0;      
};

MacroButton bCopy(PIN_COPY);
MacroButton bPaste(PIN_PASTE);
MacroButton bAlt(PIN_ALT);

FastState sCtrl;
FastState sShift;
EnterState sEnter;


void setup() {
  pinMode(PIN_COPY, INPUT_PULLUP);
  pinMode(PIN_PASTE, INPUT_PULLUP);
  pinMode(PIN_ALT, INPUT_PULLUP);
  pinMode(PIN_ENTER, INPUT_PULLUP);
  pinMode(PIN_CTRL, INPUT_PULLUP);
  pinMode(PIN_SHIFT, INPUT_PULLUP);
  
  Keyboard.begin();
  Consumer.begin();
}

void loop() {
  unsigned long now = millis();

  // 1. FAST KEYS
  processEnterLogic(now);
  processSmartModifier(PIN_CTRL, sCtrl, KEY_LEFT_CTRL, KEY_DOWN_ARROW, now);
  processSmartModifier(PIN_SHIFT, sShift, KEY_LEFT_SHIFT, KEY_UP_ARROW, now);

  // 2. MACRO KEYS
  processMacroCopy(bCopy, now);
  processMacroPaste(bPaste, now);
  processMacroAlt(bAlt, now);
}

// --------------------------------------------------------
// SMART MODIFIER (Mode Aware Repeater)
// --------------------------------------------------------

void processSmartModifier(int pin, FastState &s, KeyboardKeycode normKey, KeyboardKeycode medKey, unsigned long now) {
  bool isPressed = !digitalRead(pin);

  // ON PRESS START
  if (isPressed && !s.active) {
    s.pressTime = now;
    s.lastRepeat = now; // Initialize repeater logic
    s.active = true;

    // Determine Mode
    if (currentMode == MODE_MEDIA) {
      s.isRepeating = true;
      s.activeKey = medKey;
      Keyboard.write(s.activeKey); // Tap once immediately
    } else {
      s.isRepeating = false;
      s.activeKey = normKey;
      Keyboard.press(s.activeKey); // Hold normally
    }
  }

  // WHILE HELD
  if (isPressed && s.active) {
    // If in repeating mode (Media Arrow), handle the spam timing
    if (s.isRepeating) {
      if (now - s.pressTime > REPEAT_DELAY) {       // Initial Delay (600ms)
        if (now - s.lastRepeat > REPEAT_RATE) {     // Rate (100ms)
          Keyboard.write(s.activeKey);
          s.lastRepeat = now;
        }
      }
    }
  }
  
  // ON RELEASE
  if (!isPressed && s.active) {
    // Only send release signal if it was a Normal hold-key
    // (Repeating keys use .write() which is press+release, so no cleanup needed)
    if (!s.isRepeating) {
      Keyboard.release(s.activeKey);
    }
    s.active = false;
  }
}

// --------------------------------------------------------
// ENTER LOGIC
// --------------------------------------------------------
void processEnterLogic(unsigned long now) {
  bool isPressed = !digitalRead(PIN_ENTER);

  if (isPressed && sEnter.state == HIGH) {
    sEnter.pressTime = now;
    sEnter.holdTriggered = false;
    if (currentMode == MODE_MEDIA) {
      Consumer.press(MEDIA_VOLUME_MUTE);
      sEnter.activeModeKey = 1; 
    } else {
      Keyboard.press(KEY_RETURN);
      sEnter.activeModeKey = 0; 
    }
  }

  if (isPressed && !sEnter.holdTriggered) {
    if (now - sEnter.pressTime > HOLD_TIME_ENTER) {
      if (sEnter.activeModeKey == 1) Consumer.release(MEDIA_VOLUME_MUTE);
      else Keyboard.release(KEY_RETURN);

      Keyboard.press(KEY_LEFT_GUI); Keyboard.press(KEY_TAB);
      delay(10); 
      Keyboard.releaseAll();
      sEnter.holdTriggered = true; 
    }
  }

  if (!isPressed && sEnter.state == LOW) {
    if (!sEnter.holdTriggered) {
      if (sEnter.activeModeKey == 1) Consumer.release(MEDIA_VOLUME_MUTE);
      else Keyboard.release(KEY_RETURN);
    }
  }
  sEnter.state = isPressed ? LOW : HIGH;
}


// --------------------------------------------------------
// MACRO LOGIC
// --------------------------------------------------------

void processMacroCopy(MacroButton &b, unsigned long now) {
  bool down = !digitalRead(b.pin); 
  if (down && b.state == HIGH) { b.pressTime = now; b.holdDone = false; b.ignoreRelease = false; }
  
  if (down && !b.holdDone && (now - b.pressTime > HOLD_TIME_MACRO)) {
     currentMode = (currentMode == MODE_NORMAL) ? MODE_MEDIA : MODE_NORMAL;
     flashSignal();
     b.holdDone = true; b.ignoreRelease = true;
  }
  
  if (!down && b.state == LOW) {
    if (!b.ignoreRelease && (now - b.pressTime > MIN_PRESS_TIME)) {
      if (currentMode == MODE_MEDIA) Consumer.write(MEDIA_PLAY_PAUSE);
      else { Keyboard.press(KEY_LEFT_CTRL); Keyboard.write('c'); Keyboard.releaseAll(); }
    }
  }
  b.state = down ? LOW : HIGH;
}


void processMacroPaste(MacroButton &b, unsigned long now) {
  bool down = !digitalRead(b.pin);
  
  // Press
  if (down && b.state == HIGH) {
    b.pressTime = now; b.holdDone = false; b.ignoreRelease = false;
    if (currentMode == MODE_MEDIA) { Consumer.write(MEDIA_VOLUME_DOWN); b.lastRepeat = now; }
  }

  // Hold / Repeat
  if (down) {
    if (currentMode == MODE_MEDIA) {
       // Using new REPEAT constants
       if ((now - b.pressTime > REPEAT_DELAY) && (now - b.lastRepeat > REPEAT_RATE)) {
         Consumer.write(MEDIA_VOLUME_DOWN); 
         b.lastRepeat = now;
       }
    } else if (!b.holdDone && (now - b.pressTime > HOLD_TIME_MACRO)) {
       Keyboard.press(KEY_LEFT_GUI); Keyboard.write('v'); Keyboard.releaseAll();
       b.holdDone = true; b.ignoreRelease = true;
    }
  }

  if (!down && b.state == LOW) {
    if (!b.ignoreRelease && currentMode == MODE_NORMAL) {
       if (now - b.pressTime > MIN_PRESS_TIME) {
          Keyboard.press(KEY_LEFT_CTRL); Keyboard.write('v'); Keyboard.releaseAll();
       }
    }
  }
  b.state = down ? LOW : HIGH;
}


void processMacroAlt(MacroButton &b, unsigned long now) {
  bool down = !digitalRead(b.pin);
  
  if (down && b.state == HIGH) {
    b.pressTime = now; b.holdDone = false; b.ignoreRelease = false;
    if (currentMode == MODE_MEDIA) { Consumer.write(MEDIA_VOLUME_UP); b.lastRepeat = now; }
  }

  if (down) {
    if (currentMode == MODE_MEDIA) {
       if ((now - b.pressTime > REPEAT_DELAY) && (now - b.lastRepeat > REPEAT_RATE)) {
         Consumer.write(MEDIA_VOLUME_UP); 
         b.lastRepeat = now;
       }
    } else if (!b.holdDone && (now - b.pressTime > HOLD_TIME_MACRO)) {
       isSelectAllMode = !isSelectAllMode;
       flashSignal();
       b.holdDone = true; b.ignoreRelease = true;
    }
  }

  if (!down && b.state == LOW) {
    if (!b.ignoreRelease && currentMode == MODE_NORMAL) {
       if (now - b.pressTime > MIN_PRESS_TIME) {
           Keyboard.press(KEY_LEFT_CTRL);
           if (isSelectAllMode) Keyboard.write('a');
           else { Keyboard.press(KEY_LEFT_SHIFT); Keyboard.write('v'); }
           Keyboard.releaseAll();
       }
    }
  }
  b.state = down ? LOW : HIGH;
}


void flashSignal() {
  if (!USE_SCROLL_LOCK) return;
  Keyboard.press(KEY_SCROLL_LOCK); Keyboard.releaseAll();
  delay(50);
  Keyboard.press(KEY_SCROLL_LOCK); Keyboard.releaseAll();
}