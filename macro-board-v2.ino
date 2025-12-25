/*
 * Pro Micro Macro Keyboard
 * Version: 2.1
 * Author: MWDiss
 * Date: Dec 2025
 * 
 * Update 2.1 Changes:
 * - Reduced debounce time (fixed "missed clicks" on fast taps).
 * - Clipboard triggers via "Hold Paste". No more combos.
 * - Added auto-repeat for Volume controls in Media Mode.
 */

#include <HID-Project.h>

// --- USER CONFIGURATION ---

// Pin mappings
const int PIN_COPY  = 16;
const int PIN_PASTE = 10;
const int PIN_ALT   = 18; // Plain Paste / Select All

// Visual feedback
const bool USE_SCROLL_LOCK = true; 

// Timing settings (milliseconds)
const int HOLD_TIME = 600;      // Time to trigger Hold Actions (e.g., Clipboard)
const int DEBOUNCE  = 10;       // Reduced to 10ms for highly responsive inputs
const int VOL_DELAY = 400;      // Time before volume starts repeating
const int VOL_RATE  = 100;      // Speed of volume repetition

// --- END CONFIGURATION ---

enum Mode { MODE_NORMAL, MODE_MEDIA };
Mode currentMode = MODE_NORMAL;
bool isSelectAllMode = false; // Toggles PIN_ALT function

struct Button {
  int pin;
  bool lastState = HIGH;
  unsigned long pressTime = 0;
  bool ignoreRelease = false; 
  bool holdDone = false;
  // Volume repeat tracking
  bool repeating = false;
  unsigned long lastRepeatTime = 0;
  
  Button(int p) : pin(p) {} 
};

Button bCopy(PIN_COPY);
Button bPaste(PIN_PASTE);
Button bAlt(PIN_ALT);

void setup() {
  pinMode(PIN_COPY, INPUT_PULLUP);
  pinMode(PIN_PASTE, INPUT_PULLUP);
  pinMode(PIN_ALT, INPUT_PULLUP);
  
  Keyboard.begin();
  Consumer.begin();
}

void loop() {
  bool downCopy = !digitalRead(PIN_COPY);
  bool downPaste = !digitalRead(PIN_PASTE);
  bool downAlt = !digitalRead(PIN_ALT);
  unsigned long now = millis();

  // Process Buttons
  processCopy(bCopy, downCopy, now);
  processPaste(bPaste, downPaste, now);
  processAlt(bAlt, downAlt, now);

  delay(2); // Short stabilization
}

// --- LOGIC HANDLERS ---

void processCopy(Button &b, bool down, unsigned long now) {
  // 1. PRESS
  if (down && b.lastState == HIGH) { 
    b.pressTime = now;
    b.ignoreRelease = false;
    b.holdDone = false;
  }
  
  // 2. HOLD (Normal: Toggle Mode / Media: Nothing)
  // Media play/pause is usually on release to be safe, so we stick to Hold logic for Toggle.
  if (down && !b.holdDone && (now - b.pressTime > HOLD_TIME)) {
    // Only toggle mode if in Normal, or toggle back if in Media
    // If you prefer to allow media controls without toggle risk, we could separate logic.
    // For now: Long Press on Copy always toggles mode.
    currentMode = (currentMode == MODE_NORMAL) ? MODE_MEDIA : MODE_NORMAL;
    flashSignal();
    b.holdDone = true;
    b.ignoreRelease = true; 
  }

  // 3. RELEASE
  if (!down && b.lastState == LOW) { 
    if (now - b.pressTime > DEBOUNCE && !b.ignoreRelease) {
      if (currentMode == MODE_MEDIA) Consumer.write(MEDIA_PLAY_PAUSE);
      else { Keyboard.press(KEY_LEFT_CTRL); Keyboard.write('c'); Keyboard.releaseAll(); }
    }
  }
  b.lastState = down ? LOW : HIGH;
}

void processPaste(Button &b, bool down, unsigned long now) {
  // 1. PRESS
  if (down && b.lastState == HIGH) {
    b.pressTime = now;
    b.ignoreRelease = false;
    b.holdDone = false;
    b.repeating = false; // Reset repeating
    
    // MEDIA MODE IMMEDIATE ACTION (For smooth feel)
    if (currentMode == MODE_MEDIA) {
       Consumer.write(MEDIA_VOLUME_DOWN);
       b.lastRepeatTime = now;
    }
  }

  // 2. HOLD
  if (down) {
    // A. MEDIA MODE REPEATER
    if (currentMode == MODE_MEDIA) {
      unsigned long heldFor = now - b.pressTime;
      // If held long enough to start repeating
      if (heldFor > VOL_DELAY) {
        if (now - b.lastRepeatTime > VOL_RATE) {
          Consumer.write(MEDIA_VOLUME_DOWN);
          b.lastRepeatTime = now;
        }
      }
    } 
    // B. NORMAL MODE CLIPBOARD TRIGGER
    else if (!b.holdDone && (now - b.pressTime > HOLD_TIME)) {
      // Trigger Clipboard
      Keyboard.press(KEY_LEFT_GUI); Keyboard.write('v'); Keyboard.releaseAll();
      b.holdDone = true;
      b.ignoreRelease = true; // Don't paste on release
    }
  }

  // 3. RELEASE
  if (!down && b.lastState == LOW) { 
    if (now - b.pressTime > DEBOUNCE && !b.ignoreRelease) {
      // If NOT in Media mode (since Media acts on press/hold), do Paste
      if (currentMode == MODE_NORMAL) {
        Keyboard.press(KEY_LEFT_CTRL); Keyboard.write('v'); Keyboard.releaseAll(); 
      }
    }
  }
  b.lastState = down ? LOW : HIGH;
}

void processAlt(Button &b, bool down, unsigned long now) {
  // 1. PRESS
  if (down && b.lastState == HIGH) {
    b.pressTime = now;
    b.ignoreRelease = false;
    b.holdDone = false;
    
    // MEDIA MODE IMMEDIATE ACTION
    if (currentMode == MODE_MEDIA) {
       Consumer.write(MEDIA_VOLUME_UP);
       b.lastRepeatTime = now;
    }
  }

  // 2. HOLD
  if (down) {
    // A. MEDIA MODE REPEATER
    if (currentMode == MODE_MEDIA) {
      if (now - b.pressTime > VOL_DELAY) {
        if (now - b.lastRepeatTime > VOL_RATE) {
          Consumer.write(MEDIA_VOLUME_UP);
          b.lastRepeatTime = now;
        }
      }
    }
    // B. NORMAL MODE TOGGLE CONFIG
    else if (!b.holdDone && (now - b.pressTime > HOLD_TIME)) {
      isSelectAllMode = !isSelectAllMode;
      flashSignal();
      b.holdDone = true;
      b.ignoreRelease = true;
    }
  }

  // 3. RELEASE
  if (!down && b.lastState == LOW) { 
    if (now - b.pressTime > DEBOUNCE && !b.ignoreRelease) {
       if (currentMode == MODE_NORMAL) {
        Keyboard.press(KEY_LEFT_CTRL);
        if (isSelectAllMode) Keyboard.write('a');
        else { Keyboard.press(KEY_LEFT_SHIFT); Keyboard.write('v'); }
        Keyboard.releaseAll();
      }
    }
  }
  b.lastState = down ? LOW : HIGH;
}

void flashSignal() {
  if (!USE_SCROLL_LOCK) return;
  Keyboard.press(KEY_SCROLL_LOCK); Keyboard.releaseAll();
  delay(50);
  Keyboard.press(KEY_SCROLL_LOCK); Keyboard.releaseAll();
}
