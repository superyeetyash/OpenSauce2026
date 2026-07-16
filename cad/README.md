# Duck Hunt CAD

Parametric OpenSCAD models for the duck lift mechanism and the laser-cut
housing. **Every shared dimension lives in `params.scad`** — change it there
and all parts update together.

| File | Part | Make | Qty |
|---|---|---|---|
| `duck.scad` | Duck silhouette (~150 mm) | 3D print | 6 |
| `rack_pinion.scad` | Pinion gear / toothed rack | 3D print | 6 + 6 |
| `lift_module.scad` | Servo lift carriage | 3D print | 6 |
| `housing.scad` | Cabinet panels (2D) | Laser cut | see file |

Preview renders are in `preview/`.

## How the mechanism works

The SG90's horn carries a **module-2, 38-tooth pinion** (pitch Ø76 mm).
180° of servo sweep rolls **π × 38 mm ≈ 119 mm of rack travel** — that's the
duck's pop-up height, in ~0.3–0.5 s.

The rack slides vertically in the lift module's C-channel and rises **up
through the module**. The duck hangs off the **bottom** of the rack on a
bridge plate that is raised above the channel wall, so the bridge (and the
duck in front of it) glide past the module's guides for the whole stroke.
Gear separating force presses the rack into the channel wall, so the guide
stays engaged under load.

The lift module screws to a laser-cut mount wall through its 4 corner
holes; the servo drops in from the front, its body passing through both the
printed plate and the matching window in the wall behind it.

## ⚠ Torque budget (read before making ducks heavier)

An SG90 stalls at ~1.8 kg·cm. At the 38 mm pitch radius that is ~470 gf at
the rack — realistically **~200 g of usable lift**. So:

- Print ducks at **10–15% infill, 2 walls** (a 150 mm duck lands ~50–70 g).
- Use **MG90S** (metal gear, identical footprint) if you can — SG90 nylon
  gears strip when a duck gets slapped.
- If lifting is marginal: shrink `pinion_teeth` (e.g. 30 → Ø60 mm, ~94 mm
  travel, 27% more force) — everything re-derives automatically.

## Rendering / exporting

GUI: open a `.scad`, set `part = "..."` at the top, F6, then File → Export.

CLI (OpenSCAD isn't on PATH — use the full path; `openscad.com` prints
console output, `.exe` is silent):

```powershell
$scad = "C:\Program Files\OpenSCAD\openscad.com"
cd cad
& $scad -o duck.stl duck.scad
& $scad -o pinion.stl -D 'part="pinion"' rack_pinion.scad
& $scad -o rack.stl   -D 'part="rack"'   rack_pinion.scad
& $scad -o lift_module.stl -D 'part="module"' lift_module.scad
# housing panels, one DXF per part for the laser:
foreach ($p in "front","back","side","base","mount","deck") {
  & $scad -o "$p.dxf" -D "part=`"$p`"" housing.scad
}
```

Sanity previews: `part="mesh"` in `rack_pinion.scad` shows the gear
engagement; `part="assembly"` in `lift_module.scad` shows the whole lift;
`part="layout"` in `housing.scad` shows all panels labeled (preview only —
**don't cut the layout sheet**, the labels are text outlines).

## Print settings

| Part | Orientation | Infill | Supports |
|---|---|---|---|
| Duck | Flat on its side | 10–15%, 2 walls | No |
| Pinion | Horn pocket down | 25% | No |
| Rack | Teeth flat, back face down | 25% | Only under the bridge wings |
| Lift module | Back plate down | 20% | No (lip is 45° chamfered) |

PETG or PLA both fine. A drop of dry PTFE lube in the channel helps.

## Assembly (one lift, ×6)

1. Screw a cross/2-arm horn into the pinion's back pocket (M2 self-tappers
   through the 4 holes; the center channel gives screwdriver access).
2. Drop the servo into the module from the front, body through the window;
   screw the tabs to the plate.
3. Slide the rack into the channel, teeth toward the servo axis.
4. With the servo at 0°, hold the rack at "duck hidden" height and press
   the pinion+horn onto the servo spline, meshing the teeth; fit the horn
   screw. (This sets the travel phase — no code calibration needed.)
5. Bolt a duck to the bridge plate: 2× M3 from behind.
6. Screw the module to the mount wall; the wall's servo window must line up
   (both come from the same `params.scad`, so they do).

## Housing notes

- Panels assume **6 mm stock** (`mat_t`); tabs/slots derive from it, and
  `laser_kerf` (default 0.15) makes joints snug — set it for your machine,
  or 0 if your laser software compensates kerf.
- Biggest panels are 540 × 300 mm (+ 6 mm tabs) — needs a ~600 × 400 bed.
- The two mount walls are identical; they tab into the base at
  `mount_wall_d1/d2` and should get corner brackets or hot glue to the
  sides for stiffness.
- `button_hole_d` is a guess (12 mm) — **measure your actual buttons/caps**
  before cutting the deck.
- Rack tips rise above the mount walls at full pop — the 300 mm back wall
  covers them from the audience.
