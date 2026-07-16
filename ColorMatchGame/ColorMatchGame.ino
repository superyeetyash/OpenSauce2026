// Color Match Game — Elegoo Mega 2560   (build 4: round timer + speaker)
//
// 6 common-cathode RGB LEDs + 6 tactile buttons + passive speaker.
// Every round, all lights show a different color and the Serial Monitor
// prints a target color. Press the button under the matching light BEFORE
// the round timer runs out:
//   correct  -> ding, score +1, new round with HALF the time (1.5 s floor)
//   wrong    -> error sound, lights off, game over
//   time up  -> time-up sound, lights off, game over
// Press any button after game over to restart with the full 8 seconds.
//
// Serial Monitor commands (9600 baud):
//   c  calibrate button order (fixes "pressed blue, game said green")
//   t  LED channel test (finds lights with swapped color wires)
//
// Wiring: LED common pins and button second legs go to GND, so LED channels
// are driven HIGH-on and buttons use INPUT_PULLUP (pressed = LOW).
// Speaker: + leg to pin 34 (through ~100 ohm resistor for a bare speaker;
// a passive buzzer module can connect directly), - leg to GND.
//
// Colors like ORANGE and PURPLE need a partially-dimmed channel, and pins
// 22-27 (lights 1 and 2) have no hardware PWM on the Mega. So a Timer1
// interrupt runs 32-step software PWM on all 18 channels — every light can
// show every color identically. tone() uses Timer2, so sound and the soft
// PWM never fight. Timer5 stays free for the Servo library (ducks) later.
//
// A button that reads "pressed" for 4+ seconds is treated as broken wiring
// and disabled — the game keeps running on the remaining buttons. Press the
// Arduino RESET button after fixing it to bring it back.

const byte NUM_LIGHTS = 6;

// {R, G, B} pins per light. If the 't' test shows a light rendering the
// wrong colors, that light's wires are in the wrong order - swap its pin
// numbers here to match reality.
const byte RGB_PINS[NUM_LIGHTS][3] = {
  {22, 24, 23},  // Light 1
  {25, 27, 26},  // Light 2
  { 2,  4,  3},  // Light 3
  { 5,  7,  6},  // Light 4
  { 8, 10,  9},  // Light 5
  {11, 13, 12},  // Light 6
};

// Button pins in the same physical order as the lights (button under light 1
// first, and so on). If a round blames the wrong color, run 'c' calibration,
// then copy the printed order here to make it permanent.
byte BUTTON_PINS[NUM_LIGHTS] = {28, 29, 30, 31, 32, 33};

// ---- Speaker ---------------------------------------------------------
// Set SOUND_ENABLED to false to mute everything without rewiring.
const byte SPEAKER_PIN = 34;
const bool SOUND_ENABLED = true;

// Melodies are flat {frequency Hz, duration ms, ...} pairs. Frequency 0 is
// a rest. They play in the background from loop() — nothing blocks.
const unsigned int SOUND_STARTUP[] = {523, 90, 659, 90, 784, 90, 1047, 160};
const unsigned int SOUND_DING[]    = {880, 70, 1320, 130};
const unsigned int SOUND_WRONG[]   = {165, 200, 131, 200, 98, 400};
const unsigned int SOUND_TIMEUP[]  = {440, 140, 330, 140, 220, 380};
const unsigned int SOUND_TICK[]    = {1760, 30};
#define PLAY_SOUND(m) playMelody((m), sizeof(m) / sizeof((m)[0]) / 2)

// ---- Round timer -----------------------------------------------------
// Each round must be answered within roundTimeMs. Every correct press
// multiplies the time by ROUND_TIME_DECAY_PERCENT (50 = halves, per the
// game spec) down to ROUND_TIME_MIN_MS. The last 3 seconds tick out loud.
const unsigned long ROUND_TIME_START_MS = 8000;
const unsigned long ROUND_TIME_MIN_MS = 1500;
const byte ROUND_TIME_DECAY_PERCENT = 50;

// Palette: channel brightness 0..32 (32 = fully on).
// 6 colors, 6 lights — each round every color appears on exactly one light.
// Tuning: if ORANGE reads too red/yellow, adjust its green value (3-8);
// if PURPLE reads too blue/pink, adjust its red value (6-14).
const byte NUM_COLORS = 6;
const byte COLOR_DUTY[NUM_COLORS][3] = {
  {32,  0,  0},  // RED
  {32,  5,  0},  // ORANGE
  { 0,  0, 32},  // BLUE
  { 0, 32,  0},  // GREEN
  {10,  0, 32},  // PURPLE
  {32, 32, 32},  // WHITE
};
const char* const COLOR_NAMES[NUM_COLORS] = {
  "RED", "ORANGE", "BLUE", "GREEN", "PURPLE", "WHITE",
};

// After a press fires, the pin must read released for this long before the
// button can fire again. Filters contact bounce without adding any lag.
const unsigned long RELEASE_MS = 60;

// A button "held" this long is assumed stuck (shorted wiring) and disabled.
const unsigned long STUCK_MS = 4000;

// The startup wiring check gives up after this long and starts the game
// without the stuck buttons instead of waiting forever.
const unsigned long WIRING_WAIT_MS = 10000;

// Prints every button press/release to the Serial Monitor. Great while
// wiring/debugging; set to false for the final demo.
const bool TRACE_BUTTONS = true;

// Global brightness, 1-32. Lower values also cut total current draw — if the
// board resets when all lights come on (banner reprints in Serial Monitor),
// drop this to ~8 until every LED channel has its current-limiting resistor.
const byte BRIGHTNESS = 32;

const byte PWM_STEPS = 32;
volatile byte duty[NUM_LIGHTS][3];  // live brightness, read by the Timer1 ISR

byte lightColor[NUM_LIGHTS];  // palette index shown on each light
byte targetLight;             // which light shows the target color
int score = 0;
bool gameOver = false;

// Round timer state.
unsigned long roundTimeMs = ROUND_TIME_START_MS;  // time allowed this round
unsigned long roundDeadlineMs = 0;                // millis() when time is up
byte lastCountdownSecs = 255;                     // last whole second printed

// Background melody player state (advanced by updateSound() in loop()).
const unsigned int* melody = nullptr;
byte melodyPairs = 0;
byte melodyIdx = 0;
unsigned long noteEndMs = 0;

// Button state: a press fires on the first LOW sample, then the button is
// disarmed until the pin reads released for RELEASE_MS straight.
bool armed[NUM_LIGHTS];
unsigned long lastLowMs[NUM_LIGHTS];
unsigned long pressStartMs[NUM_LIGHTS];
bool buttonDisabled[NUM_LIGHTS];

// Software PWM: fires at 8 kHz -> full 32-step cycle at 250 Hz (flicker-free).
ISR(TIMER1_COMPA_vect) {
  static byte tick = 0;
  for (byte i = 0; i < NUM_LIGHTS; i++) {
    for (byte ch = 0; ch < 3; ch++) {
      byte d = duty[i][ch];
      if (tick == 0) {
        digitalWrite(RGB_PINS[i][ch], d > 0 ? HIGH : LOW);
      } else if (tick == d) {
        digitalWrite(RGB_PINS[i][ch], LOW);
      }
    }
  }
  tick = (tick + 1) & (PWM_STEPS - 1);
}

void setup() {
  Serial.begin(9600);  // matches the Serial Monitor's default baud
  randomSeed(analogRead(A0));  // A0 left floating for noise

  for (byte i = 0; i < NUM_LIGHTS; i++) {
    for (byte ch = 0; ch < 3; ch++) {
      pinMode(RGB_PINS[i][ch], OUTPUT);
      digitalWrite(RGB_PINS[i][ch], LOW);
      duty[i][ch] = 0;
    }
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
    armed[i] = true;
    lastLowMs[i] = 0;
    pressStartMs[i] = 0;
    buttonDisabled[i] = false;
  }
  pinMode(SPEAKER_PIN, OUTPUT);

  // Timer1 in CTC mode: 16 MHz / 8 / (249+1) = 8 kHz interrupt rate.
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = (1 << WGM12) | (1 << CS11);  // CTC, prescaler 8
  OCR1A = 249;
  TIMSK1 = (1 << OCIE1A);
  interrupts();

  Serial.println(F("=== COLOR MATCH GAME (build 4) ==="));
  Serial.println(F("Commands: 'c' = calibrate button order, 't' = LED color test"));
  checkButtonWiring();
  PLAY_SOUND(SOUND_STARTUP);
  newRound();
}

void loop() {
  if (Serial.available()) {
    char ch = Serial.read();
    if (ch == 'c' || ch == 'C') calibrateButtons();
    if (ch == 't' || ch == 'T') testChannels();
  }

  updateSound();
  if (!gameOver) updateRoundTimer();

  for (byte i = 0; i < NUM_LIGHTS; i++) {
    if (buttonDisabled[i]) continue;
    if (buttonJustPressed(i)) {
      handlePress(i);
    }
  }
}

// Counts the round down: prints/ticks the last 3 seconds, ends the game at 0.
void updateRoundTimer() {
  long remain = (long)(roundDeadlineMs - millis());
  if (remain <= 0) {
    allLightsOff();
    gameOver = true;
    PLAY_SOUND(SOUND_TIMEUP);
    Serial.print(F("TIME'S UP! The target was "));
    Serial.println(COLOR_NAMES[lightColor[targetLight]]);
    Serial.print(F("GAME OVER - final score: "));
    Serial.println(score);
    Serial.println(F("Press any button to play again."));
    return;
  }

  byte secs = (remain + 999) / 1000;  // whole seconds left, rounded up
  if (secs != lastCountdownSecs) {
    lastCountdownSecs = secs;
    if (secs <= 3) {
      Serial.print(F("  "));
      Serial.print(secs);
      Serial.println(F("..."));
      if (!soundPlaying()) PLAY_SOUND(SOUND_TICK);
    }
  }
}

void handlePress(byte button) {
  if (gameOver) {
    // Any button restarts a fresh game at full time.
    gameOver = false;
    score = 0;
    roundTimeMs = ROUND_TIME_START_MS;
    PLAY_SOUND(SOUND_STARTUP);
    Serial.println();
    Serial.println(F("=== NEW GAME ==="));
    newRound();
    return;
  }

  if (button == targetLight) {
    score++;
    PLAY_SOUND(SOUND_DING);
    // Next round gets less time (halves by default, floors at the minimum).
    roundTimeMs = roundTimeMs * ROUND_TIME_DECAY_PERCENT / 100;
    if (roundTimeMs < ROUND_TIME_MIN_MS) roundTimeMs = ROUND_TIME_MIN_MS;
    Serial.print(F("Correct! Score: "));
    Serial.println(score);
    newRound();
  } else {
    allLightsOff();
    gameOver = true;
    PLAY_SOUND(SOUND_WRONG);
    Serial.print(F("WRONG! You pressed button "));
    Serial.print(button + 1);
    Serial.print(F(" whose light was "));
    Serial.println(COLOR_NAMES[lightColor[button]]);
    Serial.print(F("GAME OVER - final score: "));
    Serial.println(score);
    Serial.println(F("Press any button to play again."));
  }
}

void newRound() {
  byte enabledCount = 0;
  for (byte i = 0; i < NUM_LIGHTS; i++) {
    if (!buttonDisabled[i]) enabledCount++;
  }
  if (enabledCount == 0) {
    allLightsOff();
    gameOver = true;
    Serial.println(F("No working buttons left - fix wiring and press RESET."));
    return;
  }

  // Fisher-Yates shuffle, then deal colors to the still-enabled lights.
  byte deck[NUM_COLORS];
  for (byte i = 0; i < NUM_COLORS; i++) deck[i] = i;
  for (byte i = NUM_COLORS - 1; i > 0; i--) {
    byte j = random(i + 1);
    byte tmp = deck[i];
    deck[i] = deck[j];
    deck[j] = tmp;
  }

  byte slot = 0;
  for (byte i = 0; i < NUM_LIGHTS; i++) {
    if (buttonDisabled[i]) {
      lightOff(i);  // disabled pairs stay dark and are never the target
      continue;
    }
    lightColor[i] = deck[slot++];
    setLight(i, lightColor[i]);
  }

  do {
    targetLight = random(NUM_LIGHTS);
  } while (buttonDisabled[targetLight]);

  roundDeadlineMs = millis() + roundTimeMs;
  lastCountdownSecs = 255;

  Serial.print(F("Find and press: "));
  Serial.print(COLOR_NAMES[lightColor[targetLight]]);
  Serial.print(F("  ("));
  printSeconds(roundTimeMs);
  Serial.println(F(" s)"));
}

// Prints milliseconds as seconds with one decimal, e.g. 1500 -> "1.5".
void printSeconds(unsigned long ms) {
  Serial.print(ms / 1000);
  Serial.print('.');
  Serial.print((ms % 1000) / 100);
}

void setLight(byte light, byte color) {
  for (byte ch = 0; ch < 3; ch++) {
    byte scaled = ((unsigned int)COLOR_DUTY[color][ch] * BRIGHTNESS + PWM_STEPS / 2) / PWM_STEPS;
    if (COLOR_DUTY[color][ch] > 0 && scaled == 0) scaled = 1;  // keep dim channels alive
    duty[light][ch] = scaled;
  }
}

void lightOff(byte light) {
  for (byte ch = 0; ch < 3; ch++) {
    duty[light][ch] = 0;
  }
}

void allLightsOff() {
  for (byte i = 0; i < NUM_LIGHTS; i++) {
    lightOff(i);
  }
}

// A healthy button reads HIGH (released) at power-up. Any button reading LOW
// is flagged (its LED lit solid RED) and located with the cross-short scan.
// After WIRING_WAIT_MS the stuck buttons are disabled and the game starts
// without them, so one bad wire can't block the whole demo.
void checkButtonWiring() {
  delay(50);  // let the pullups settle
  unsigned long t0 = millis();
  unsigned long lastStatus = millis();
  bool warned = false;

  while (true) {
    byte stuckCount = 0;
    for (byte i = 0; i < NUM_LIGHTS; i++) {
      if (digitalRead(BUTTON_PINS[i]) == LOW) {
        stuckCount++;
        setLight(i, 0);  // RED = this button reads as pressed
      } else {
        lightOff(i);
      }
    }
    if (stuckCount == 0) break;

    if (!warned && millis() - t0 > 1000) {
      warned = true;
      Serial.println(F("WARNING: these buttons read as PRESSED with nothing touching them:"));
      for (byte i = 0; i < NUM_LIGHTS; i++) {
        if (digitalRead(BUTTON_PINS[i]) == LOW) {
          Serial.print(F("  Button "));
          Serial.print(i + 1);
          Serial.print(F(" (pin "));
          Serial.print(BUTTON_PINS[i]);
          Serial.println(F(") - its LED is lit RED"));
        }
      }
      Serial.println(F("Running cross-short scan to locate the problem..."));
      for (byte i = 0; i < NUM_LIGHTS; i++) {
        if (digitalRead(BUTTON_PINS[i]) == LOW) findCrossShort(i);
      }
      Serial.println(F("Red LED off = fixed. In 10s the game starts without stuck buttons."));
    }

    if (warned && millis() - lastStatus > 3000) {
      lastStatus = millis();
      Serial.print(F("Still stuck: "));
      for (byte i = 0; i < NUM_LIGHTS; i++) {
        if (digitalRead(BUTTON_PINS[i]) == LOW) {
          Serial.print(i + 1);
          Serial.print(' ');
        }
      }
      Serial.println();
    }

    if (millis() - t0 > WIRING_WAIT_MS) {
      for (byte i = 0; i < NUM_LIGHTS; i++) {
        if (digitalRead(BUTTON_PINS[i]) == LOW) {
          buttonDisabled[i] = true;
          Serial.print(F("Button "));
          Serial.print(i + 1);
          Serial.println(F(" still stuck - DISABLED. Fix it and press RESET to re-enable."));
        }
      }
      break;
    }
  }

  allLightsOff();
  if (warned) Serial.println(F("Starting game."));
}

// Locates what a stuck button's wire is actually shorted to. Every LED
// channel that is "off" is driven to 0 V (same as ground), so a button wire
// touching any LED wire reads as pressed. Driving each LED channel HIGH one
// at a time and re-reading the button pinpoints the exact wire it touches.
void findCrossShort(byte b) {
  TIMSK1 = 0;  // pause soft PWM so it can't fight the test writes
  for (byte i = 0; i < NUM_LIGHTS; i++) {
    for (byte ch = 0; ch < 3; ch++) digitalWrite(RGB_PINS[i][ch], LOW);
  }

  bool found = false;
  for (byte i = 0; i < NUM_LIGHTS && !found; i++) {
    for (byte ch = 0; ch < 3 && !found; ch++) {
      digitalWrite(RGB_PINS[i][ch], HIGH);
      delay(2);
      if (digitalRead(BUTTON_PINS[b]) == HIGH) {  // reads released only now?
        found = true;
        const char* chName = (ch == 0) ? "RED" : (ch == 1) ? "GREEN" : "BLUE";
        Serial.print(F("FOUND IT: Button "));
        Serial.print(b + 1);
        Serial.print(F("'s signal wire is shorted to LED "));
        Serial.print(i + 1);
        Serial.print(F("'s "));
        Serial.print(chName);
        Serial.print(F(" wire (pin "));
        Serial.print(RGB_PINS[i][ch]);
        Serial.println(F(") - they share a breadboard row or are touching."));
      }
      digitalWrite(RGB_PINS[i][ch], LOW);
      delay(2);
    }
  }

  if (!found) {
    Serial.print(F("Button "));
    Serial.print(b + 1);
    Serial.println(F(" is shorted straight to ground, not to an LED wire."));
    Serial.println(F("-> jammed/rotated switch, wire in a ground row, or dead pin."));
  }

  TIMSK1 = (1 << OCIE1A);  // resume soft PWM
}

// Interactive fix for "I pressed the button under BLUE but the game counted
// a different light". Lights each LED white in turn; press the button that
// physically sits under it. Send 's' to skip a light with no working button
// (that pair is disabled). Prints the corrected pin order when done.
void calibrateButtons() {
  Serial.println(F("=== BUTTON CALIBRATION ==="));
  Serial.println(F("One light turns WHITE at a time - press the button directly"));
  Serial.println(F("under it. Send 's' to skip a light with no working button."));
  allLightsOff();
  delay(300);

  bool usable[NUM_LIGHTS];
  byte newPins[NUM_LIGHTS];
  for (byte k = 0; k < NUM_LIGHTS; k++) {
    usable[k] = (digitalRead(BUTTON_PINS[k]) == HIGH);
    if (!usable[k]) {
      Serial.print(F("(pin "));
      Serial.print(BUTTON_PINS[k]);
      Serial.println(F(" reads stuck-pressed and is excluded)"));
    }
    buttonDisabled[k] = false;
  }

  for (byte L = 0; L < NUM_LIGHTS; L++) {
    setLight(L, 5);  // WHITE
    Serial.print(F("Light "));
    Serial.print(L + 1);
    Serial.println(F(" is WHITE - press its button (or send 's' to skip)"));

    byte hit = 255;
    bool skipped = false;
    while (hit == 255 && !skipped) {
      if (Serial.available()) {
        char c = Serial.read();
        if (c == 's' || c == 'S') skipped = true;
      }
      for (byte k = 0; k < NUM_LIGHTS && hit == 255; k++) {
        if (usable[k] && digitalRead(BUTTON_PINS[k]) == LOW) hit = k;
      }
    }
    lightOff(L);

    if (skipped) {
      newPins[L] = BUTTON_PINS[L];
      buttonDisabled[L] = true;
      Serial.println(F("  skipped - this light/button pair is disabled until RESET."));
      continue;
    }

    newPins[L] = BUTTON_PINS[hit];
    usable[hit] = false;  // each button can only claim one light
    Serial.print(F("  mapped to pin "));
    Serial.println(newPins[L]);

    // Wait for a clean release before moving to the next light.
    unsigned long releasedAt = millis();
    while (millis() - releasedAt < 150) {
      if (digitalRead(newPins[L]) == LOW) releasedAt = millis();
    }
  }

  for (byte L = 0; L < NUM_LIGHTS; L++) BUTTON_PINS[L] = newPins[L];

  Serial.print(F("Done! New button order (pins): {"));
  for (byte L = 0; L < NUM_LIGHTS; L++) {
    Serial.print(BUTTON_PINS[L]);
    if (L < NUM_LIGHTS - 1) Serial.print(F(", "));
  }
  Serial.println(F("}"));
  Serial.println(F("Copy this into the BUTTON_PINS line of the sketch to keep it forever."));

  for (byte i = 0; i < NUM_LIGHTS; i++) {
    armed[i] = true;
    lastLowMs[i] = millis();
    pressStartMs[i] = millis();
  }
  gameOver = false;
  score = 0;
  roundTimeMs = ROUND_TIME_START_MS;
  Serial.println(F("=== NEW GAME ==="));
  newRound();
}

// Shows all lights in pure RED, then GREEN, then BLUE (twice). Any light
// displaying the wrong color has its wires in the wrong order - swap that
// light's pin numbers in RGB_PINS (or swap the jumpers) to fix it.
void testChannels() {
  Serial.println(F("=== LED CHANNEL TEST ==="));
  const byte pure[3] = {0, 3, 2};  // palette indices of RED, GREEN, BLUE
  const char* names[3] = {"RED", "GREEN", "BLUE"};
  for (byte rep = 0; rep < 2; rep++) {
    for (byte c = 0; c < 3; c++) {
      Serial.print(F("ALL lights should now be "));
      Serial.println(names[c]);
      for (byte i = 0; i < NUM_LIGHTS; i++) setLight(i, pure[c]);
      delay(2500);
    }
  }
  allLightsOff();
  Serial.println(F("Any light that showed the wrong color has swapped wires -"));
  Serial.println(F("fix its pin order in RGB_PINS at the top of the sketch."));
  Serial.println(F("=== NEW GAME ==="));
  gameOver = false;
  score = 0;
  roundTimeMs = ROUND_TIME_START_MS;
  newRound();
}

// Returns true exactly once per press, the instant the pin first reads
// pressed. The button then stays disarmed until the pin has read released
// for RELEASE_MS straight, so scratchy/bouncy contacts can't double-fire.
// A button that stays pressed for STUCK_MS is disabled as broken wiring.
bool buttonJustPressed(byte i) {
  bool raw = (digitalRead(BUTTON_PINS[i]) == LOW);

  if (armed[i]) {
    if (raw) {
      armed[i] = false;
      lastLowMs[i] = millis();
      pressStartMs[i] = millis();
      if (TRACE_BUTTONS) {
        Serial.print(F("[button "));
        Serial.print(i + 1);
        Serial.println(F(" pressed]"));
      }
      return true;
    }
    return false;
  }

  if (raw) {
    lastLowMs[i] = millis();  // still held (or bouncing) - stay disarmed
    if (millis() - pressStartMs[i] >= STUCK_MS) {
      disableButton(i);
    }
  } else if (millis() - lastLowMs[i] >= RELEASE_MS) {
    armed[i] = true;
    if (TRACE_BUTTONS) {
      Serial.print(F("[button "));
      Serial.print(i + 1);
      Serial.println(F(" released]"));
    }
  }
  return false;
}

// ---- Background melody player -----------------------------------------
// tone() runs the note itself on Timer2; these functions only decide when
// to start the next note, so nothing here ever blocks the game loop.

// Starts a melody (flat {freq, ms} pair array). Cuts off whatever was
// playing — the newest sound is always the most important one.
void playMelody(const unsigned int* notes, byte pairs) {
  if (!SOUND_ENABLED) return;
  melody = notes;
  melodyPairs = pairs;
  melodyIdx = 0;
  startNote();
}

// Plays the current note of the active melody (frequency 0 = rest).
void startNote() {
  if (melodyIdx >= melodyPairs) {
    melody = nullptr;
    return;
  }
  unsigned int freq = melody[melodyIdx * 2];
  unsigned int dur = melody[melodyIdx * 2 + 1];
  if (freq > 0) tone(SPEAKER_PIN, freq, dur);
  else noTone(SPEAKER_PIN);
  noteEndMs = millis() + dur + 20;  // small gap so notes don't slur
}

// Call from loop(): advances to the next note when the current one ends.
void updateSound() {
  if (melody == nullptr) return;
  if ((long)(millis() - noteEndMs) >= 0) {
    melodyIdx++;
    startNote();
  }
}

bool soundPlaying() {
  return melody != nullptr;
}

// Takes a misbehaving button (and its light) out of the game until RESET.
void disableButton(byte i) {
  buttonDisabled[i] = true;
  lightOff(i);
  Serial.print(F("Button "));
  Serial.print(i + 1);
  Serial.println(F(" has read pressed for 4+ seconds - DISABLED (stuck wiring)."));
  Serial.println(F("The game continues without it. Fix it and press RESET."));
  if (!gameOver) newRound();
}
