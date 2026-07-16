# CLAUDE.md

Duck Hunt reaction arcade game for the OpenSauce 2026 hardware hackathon: 6 servo-lifted
ducks behind a wall, 6 RGB-lit buttons, color-match gameplay with a shrinking round timer,
HUB75 matrix scoreboard (Pico 2, future). Full description, BOM, pin map, and power plan:
`README.md`.

## Layout

- `ColorMatchGame/ColorMatchGame.ino` — the one Mega 2560 sketch (self-contained, no libraries)
- `cad/` — parametric OpenSCAD; all shared dimensions in `cad/params.scad`

## Source of truth

Pin assignments live in the tables at the top of `ColorMatchGame.ino` (`RGB_PINS`,
`BUTTON_PINS`, `SPEAKER_PIN`). If they change, update the README pin map to match.

## Mega timer budget (do not reassign)

| Timer | Used by | Why |
|---|---|---|
| Timer1 | Soft-PWM ISR in the sketch | Pins 22–27 (lights 1–2) have no hardware PWM; an 8 kHz ISR runs 32-step software PWM on all 18 LED channels |
| Timer2 | `tone()` (speaker, pin 34) | Default tone timer on the Mega |
| Timer5 | Reserved for Servo library | Mega Servo lib takes Timer5 for the first 12 servos (ducks, future) |

Consequences: no `analogWrite()` on pins 11/12 (Timer1), no other `tone()`-style use of
Timer2, and keep the Timer1 ISR lean — it runs 8000×/s.

## Sketch conventions

- Single self-contained `.ino`; no external libraries so far.
- `F()` around every string literal (RAM is 8 KB).
- Non-blocking `millis()` patterns in game flow — no `delay()` outside setup/diagnostics.
  The melody player and round timer are both advanced from `loop()`.
- Hardware-failure tolerance is a feature: stuck buttons get auto-disabled and the game
  continues (`checkButtonWiring`, `disableButton`); keep new features working when
  buttons are disabled (skip `buttonDisabled[i]` pairs, never make them the target).
- Wiring: common-cathode LEDs driven HIGH-on; buttons `INPUT_PULLUP`, pressed = LOW.
- `random()` is seeded from floating A0 — don't wire anything to A0.

## Compile check (no hardware needed)

```powershell
& "$env:LOCALAPPDATA\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe" `
  compile --fqbn arduino:avr:mega ColorMatchGame
```

arduino:avr core 1.8.8 is installed. Always compile after editing the sketch.

## CAD rendering

OpenSCAD is installed but not on PATH:

```powershell
& "C:\Program Files\OpenSCAD\openscad.exe" -o out\duck.stl cad\duck.scad          # 3D parts -> STL
& "C:\Program Files\OpenSCAD\openscad.exe" -o out\housing.dxf cad\housing.scad    # 2D panels -> DXF
```

Per-part instructions and print settings: `cad/README.md`.

## Fabrication decisions (locked in with the team)

- Servos: SG90/MG90S micro, ~180° sweep. MG90S preferred (metal gears).
- Ducks ~150 mm tall, ~120 mm pop-up travel → module-2, 38-tooth pinion (pitch Ø76 mm),
  since travel per 180° = π × pitch radius. Torque is the tight constraint: keep the
  duck+rack assembly light (10–15% infill).
- 3D print everything except housing walls; walls are laser-cut (thickness is a
  parameter in `cad/params.scad`).
