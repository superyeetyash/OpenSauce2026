// duck.scad — the target: classic carnival duck silhouette, printed flat.
// part = "duck" (3D) | "outline" (2D, e.g. if you'd rather laser-cut ducks)
//
// Render:  openscad -o duck.stl duck.scad
// Print flat on its side, 2 walls, 10-15% infill — the SG90 lifting it
// only has ~470 gf at the rack, so LIGHT is the whole game.
// Bolts to the rack head with 2x M3 through the base tab.

include <params.scad>

part = "duck";

// Silhouette drawn at a nominal 150 mm height, then scaled to duck_height.
// Duck faces -X (beak left). Base tab bottom at y = 0.
module duck_outline() {
  scale(duck_height / 150) difference() {
    union() {
      hull() {  // body
        translate([52, 52]) circle(34);
        translate([98, 56]) circle(28);
      }
      hull() {  // tail kick
        translate([98, 60]) circle(24);
        translate([138, 92]) circle(7);
      }
      hull() {  // chest -> neck
        translate([46, 56]) circle(24);
        translate([40, 112]) circle(21);
      }
      translate([40, 118]) circle(23);  // head
      hull() {  // beak
        translate([32, 116]) circle(9);
        translate([8, 108]) circle(4);
      }
      translate([30, 0]) square([70, 30]);  // base tab down to the rack
    }
    if (duck_eye_d > 0) translate([34, 124]) circle(d = duck_eye_d);
  }
}

module duck() {
  s = duck_height / 150;
  difference() {
    linear_extrude(duck_thickness) duck_outline();
    // M3 pair matching the rack head (spacing stays true, never scaled)
    for (sx = [-1, 1])
      translate([65 * s + sx * duck_mount_spacing / 2, 11 * s, -0.5])
        cylinder(d = m3_hole, h = duck_thickness + 1);
  }
}

if (part == "duck") duck();
if (part == "outline") duck_outline();
