# Duck Hunt Reaction Arcade — OpenSauce 2026

An interactive arcade-style reaction game built for the OpenSauce 2026 hardware hackathon
(prompt: **make something interactive**). Players test speed, accuracy, and reflexes by
hitting the correct illuminated button before time runs out.

A modern, customizable take on the carnival duck-shooting game — shooting replaced by
fast-paced color recognition. Designed to be visually impressive, instantly understandable
to spectators, and playable by a crowd.

## How it plays

- Six mechanical **ducks** (two rows of three) hide behind a wall, each on a servo-driven
  rack-and-pinion lift. Each duck has a fixed color.
- Six **arcade buttons** in the same 2×3 layout each contain an RGB LED. Every round the
  button colors **shuffle**.
- A duck pops up → find the button showing the duck's color → press it before the timer
  runs out.
  - **Correct** → duck drops, *ding*, +1 point, next round with **half the time**
    (floors at 1.5 s).
  - **Wrong** → error sound, game over.
  - **Time up** → time-up sound, game over.
- A **HUB75 64×32 LED matrix** is the arcade marquee: title, score, countdown, round
  info, animations.

### Main features

- 🎯 Reaction-based gameplay
- 🦆 Six servo-powered rising duck targets
- 🌈 RGB button color-matching system
- ⏱️ Progressively faster rounds (time halves each round)
- 🏆 Score tracking and high scores
- 🖥️ Large LED matrix arcade display
- 🔊 Arcade-style sound effects
- 🎮 Physical interactive controls

## System architecture

```
                 P5 HUB75 LED Matrix
                         |
                  Raspberry Pi Pico 2
                         |
                       UART
                         |
                  Arduino Mega 2560
                  /       |        \
                 /        |         \
            Servos     Buttons    RGB LEDs
              |           |          |
           Ducks     Player Input  Colors
```

The Mega 2560 owns all game logic, inputs, lights, servos, and sound. The Pico 2 is a
dedicated display driver for the HUB75 panel (HUB75 needs constant high-speed refresh —
too much for the Mega on top of everything else) and receives score/timer/state over UART.

## Repo layout

| Path | What it is |
|---|---|
| `ColorMatchGame/ColorMatchGame.ino` | Current Mega sketch: buttons + RGB lights + round timer + speaker (the color-match core of the game, playable today via Serial Monitor) |
| `cad/` | Parametric OpenSCAD models: ducks, rack & pinion, servo lift modules, laser-cut housing panels — see `cad/README.md` |
| `CLAUDE.md` | Working notes/conventions for AI-assisted development |

## Bill of materials

| Qty | Part | Notes |
|---|---|---|
| 1 | Elegoo/Arduino **Mega 2560** | Main controller |
| 6 | 4-pin tactile buttons | One leg to signal pin, other to GND |
| 6 | 4-pin RGB LEDs, **common cathode** | Common (longest) pin to GND |
| 18 | ~220 Ω resistors | One per LED color channel (R, G, B × 6) |
| 6 | **SG90 / MG90S** micro servos | MG90S (metal gear) recommended — see `cad/README.md` |
| 1 | Passive speaker or passive buzzer | On pin 34 (~100 Ω series resistor for a bare speaker) |
| 1 | Raspberry Pi **Pico 2** | HUB75 display driver (future phase) |
| 1 | **P5 HUB75 64×32** RGB matrix | Arcade display (future phase) |
| 1 | 5 V power supply | See “Power” below |

## Pin map (Mega 2560)

Wiring style: **LED common pins and button second legs all go to GND.**
So LED channels are driven HIGH-on, and buttons use `INPUT_PULLUP` (pressed = LOW).

| Pin | Function | | Pin | Function |
|---|---|---|---|---|
| 2 | Light 3 — R | | 22 | Light 1 — R |
| 3 | Light 3 — G | | 23 | Light 1 — G |
| 4 | Light 3 — B | | 24 | Light 1 — B |
| 5 | Light 4 — R | | 25 | Light 2 — R |
| 6 | Light 4 — G | | 26 | Light 2 — G |
| 7 | Light 4 — B | | 27 | Light 2 — B |
| 8 | Light 5 — R | | 28 | Button 1 |
| 9 | Light 5 — G | | 29 | Button 2 |
| 10 | Light 5 — B | | 30 | Button 3 |
| 11 | Light 6 — R | | 31 | Button 4 |
| 12 | Light 6 — G | | 32 | Button 5 |
| 13 | Light 6 — B | | 33 | Button 6 |
| | | | 34 | Speaker (+) |

The authoritative pin tables live at the top of `ColorMatchGame/ColorMatchGame.ino`
(`RGB_PINS`, `BUTTON_PINS`, `SPEAKER_PIN`) — if you rewire, change them there.

Notes:

- Every LED channel needs its ~220 Ω resistor. Until they're all in, drop `BRIGHTNESS`
  in the sketch to ~8 so total current can't brown-out the board.
- Pin 13 (Light 6 — B) shares the Mega's onboard LED — the tiny board LED blinking with
  it is normal.
- The sketch expects each light's wires in R, G, B order per `RGB_PINS`; the built-in
  `t` test and `c` calibration commands find swapped wires/buttons for you.

## Power

One duck moves at a time, but budget for spikes. **Never power servos or the matrix from
the Mega's 5 V pin** — it can only source a few hundred mA.

| Load | Budget | Feed from |
|---|---|---|
| Mega 2560 | ~0.2 A | USB (dev) or 7–12 V barrel jack (demo) |
| 6× SG90/MG90S | 2 A (each can spike ~0.65 A at stall) | Dedicated 5 V rail |
| HUB75 64×32 | 2–3 A worst case (full white) | Dedicated 5 V rail |
| LEDs + logic | ~0.3 A | Mega |

**Recommended:** one **5 V 10 A** supply feeding a small distribution block → servo rail +
matrix rail (Pico 2 can take 5 V on VSYS). Or two supplies: 5 V/3 A for servos, 5 V/4 A
for the matrix. Either way:

- **All grounds common** — servo rail GND, matrix GND, and Mega GND tied together.
- Put a **470–1000 µF electrolytic** across the servo rail near the servos; servo start-up
  spikes otherwise reset microcontrollers.
- Signal wires (servo PWM, HUB75 data, UART) carry no power — only these plus ground go
  to the boards.

## Build & upload

**Arduino IDE:** open `ColorMatchGame/ColorMatchGame.ino`, board = *Arduino Mega or Mega
2560*, upload. Serial Monitor at **9600 baud**.

**CLI** (arduino-cli, also bundled inside Arduino IDE 2.x):

```
arduino-cli compile --fqbn arduino:avr:mega ColorMatchGame
arduino-cli upload  --fqbn arduino:avr:mega -p COM3 ColorMatchGame   # your COM port
```

### What you should see/hear

1. Serial Monitor prints the banner.
2. If any button reads stuck-pressed, its LED lights red and a cross-short scan tells you
   exactly which wires are touching. After 10 s the game starts without the bad buttons.
3. Each round: all six lights show different colors, Serial prints
   `Find and press: PURPLE  (8.0 s)`. The last 3 seconds tick audibly.
4. Serial commands: `c` = re-map button order interactively, `t` = LED channel test.

## CAD & fabrication

Everything mechanical is parametric OpenSCAD in [`cad/`](cad/README.md):
3D-printed ducks (~150 mm), rack-and-pinion lift (~120 mm travel per 180° servo sweep),
six identical servo lift modules, and laser-cut housing panels (DXF export).
Dimensions, gear module, travel, and material thickness are all knobs in `cad/params.scad`.

## Roadmap

- [x] Buttons + RGB color-match game with round timer and sound (playable via Serial)
- [x] CAD: duck, rack & pinion, lift module, housing panels
- [ ] Servo control on the Mega (Servo lib uses Timer5 — reserved, nothing to free up)
- [ ] Full duck-hunt logic: duck pops up, its color is the target (replaces Serial prompt)
- [ ] Pico 2 + HUB75 marquee (score/timer/animations), UART protocol Mega → Pico
- [ ] High-score persistence (Mega EEPROM)
- [ ] Wire speaker → real arcade sounds are already stubbed in

## Pictures
<img width="3024" height="3024" alt="IMG_8560" src="https://github.com/user-attachments/assets/a1f3dd56-f906-4985-8864-4094a30a135f" />
<img width="3024" height="4032" alt="IMG_8562" src="https://github.com/user-attachments/assets/51169030-0185-404a-ae03-f9743e1fcf3f" />
<img width="3024" height="4032" alt="IMG_8565" src="https://github.com/user-attachments/assets/e07b2a31-da50-45e2-a050-8f1881bb390f" />
<img width="3024" height="4032" alt="IMG_8564" src="https://github.com/user-attachments/assets/190b87f1-5b0f-4bc2-9918-0c6a42ec60d2" />
<img width="3024" height="4032" alt="IMG_8563" src="https://github.com/user-attachments/assets/107aa109-ab1b-4236-9435-2b1e92b25231" />

