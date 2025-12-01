/*
 * Pro Micro Macro Keyboard
 * Version: 2.0
 * Author: MWDiss
 * Date: Dec 1, 2025
 * 
 * A 3-button macro pad with chorded inputs and toggleable modes.
 */

#include <HID-Project.h>

// --- USER CONFIGURATION ---

// Pin mappings
const int PIN_COPY  = 16;
const int PIN_PASTE = 10;
const int PIN_ALT   = 18; // Plain Paste / Select All

// Visual feedback (Set false if you don't want your keyboard flashing)
const bool USE_SCROLL_LOCK = true; 

// Timing settings (milliseconds)
// Recommended range: 400 - 800ms
const int HOLD_TIME = 600;      
const int DEBOUNCE  = 50; 

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
  Button(int p) : pin(p) {} 
};

Button bCopy(PIN_COPY);
Button bPaste(PIN_PASTE);
Button bAlt(PIN_ALT);

bool comboActive = false;

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

  // 1. Handle Combinations (Paste + Alt)
  if (downPaste && downAlt) {
    if (!comboActive) {
      Keyboard.press(KEY_LEFT_GUI); 
      Keyboard.write('v'); 
      Keyboard.releaseAll();
      
      // Stop individual button actions
      bPaste.ignoreRelease = true;
      bAlt.ignoreRelease = true;
      comboActive = true; 
    }
    bAlt.pressTime = now; // Prevent Alt hold-trigger while comboing
  } else {
    comboActive = false;
  }

  // 2. Handle Buttons
  processCopy(bCopy, downCopy, now);
  processPaste(bPaste, downPaste, now);
  processAlt(bAlt, downAlt, now);

  delay(2); // CPU nap
}

// --- LOGIC HANDLERS ---

void processCopy(Button &b, bool down, unsigned long now) {
  if (down && b.lastState == HIGH) { // Press
    b.pressTime = now;
    b.ignoreRelease = false;
    b.holdDone = false;
  }
  
  // Hold Logic (Toggle Media Mode)
  if (down && !b.holdDone && (now - b.pressTime > HOLD_TIME)) {
    currentMode = (currentMode == MODE_NORMAL) ? MODE_MEDIA : MODE_NORMAL;
    flashSignal();
    b.holdDone = true;
    b.ignoreRelease = true; 
  }

  if (!down && b.lastState == LOW) { // Release
    if (now - b.pressTime > DEBOUNCE && !b.ignoreRelease) {
      if (currentMode == MODE_MEDIA) Consumer.write(MEDIA_PLAY_PAUSE);
      else { Keyboard.press(KEY_LEFT_CTRL); Keyboard.write('c'); Keyboard.releaseAll(); }
    }
  }
  b.lastState = down ? LOW : HIGH;
}

void processPaste(Button &b, bool down, unsigned long now) {
  if (down && b.lastState == HIGH) {
    b.pressTime = now;
    b.ignoreRelease = false;
  }
  
  // Paste has no hold action
  if (!down && b.lastState == LOW) { 
    if (now - b.pressTime > DEBOUNCE && !b.ignoreRelease) {
      if (currentMode == MODE_MEDIA) Consumer.write(MEDIA_VOLUME_DOWN);
      else { Keyboard.press(KEY_LEFT_CTRL); Keyboard.write('v'); Keyboard.releaseAll(); }
    }
  }
  b.lastState = down ? LOW : HIGH;
}

void processAlt(Button &b, bool down, unsigned long now) {
  if (down && b.lastState == HIGH) {
    b.pressTime = now;
    b.ignoreRelease = false;
    b.holdDone = false;
  }

  // Hold Logic (Toggle Button Config)
  if (down && !b.holdDone && !b.ignoreRelease && (now - b.pressTime > HOLD_TIME)) {
    isSelectAllMode = !isSelectAllMode;
    flashSignal();
    b.holdDone = true;
    b.ignoreRelease = true;
  }

  if (!down && b.lastState == LOW) { 
    if (now - b.pressTime > DEBOUNCE && !b.ignoreRelease) {
      if (currentMode == MODE_MEDIA) {
        Consumer.write(MEDIA_VOLUME_UP);
      } else {
        // Toggle behavior check
        Keyboard.press(KEY_LEFT_CTRL);
        if (isSelectAllMode) {
          Keyboard.write('a');
        } else {
          Keyboard.press(KEY_LEFT_SHIFT); 
          Keyboard.write('v');
        }
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