// rack_pinion.scad — the drive: servo pinion + duck rack.
// part = "pinion" | "rack" | "mesh" (preview of both engaged)
//
// Render:  openscad -o pinion.stl -D part="\"pinion\"" rack_pinion.scad
// Both parts print flat, no supports. The pinion's back face has a pocket
// for the stock SG90 cross/2-arm horn (printed splines never grip) — screw
// the gear to the horn with M2 self-tappers, then the horn to the servo.

include <params.scad>

part = "mesh";

// ---- 2D tooth profiles --------------------------------------------------
// Straight-flank (trapezoid) teeth at the pressure angle. Exact for the
// rack; a close approximation for a 38-tooth pinion, and plenty for a
// printed, lightly loaded, low-speed drive.

module pinion_2d() {
  rp = pinion_pitch_r;
  ra = pinion_tip_r;
  rr = pinion_root_r;
  // angular half-widths of a tooth (degrees) at pitch, tip, root
  half_p = ((PI * gear_module / 2 - gear_backlash) / 2) / rp * 180 / PI;
  d_tip = (gear_module * tan(pressure_angle)) / rp * 180 / PI;
  d_root = (1.25 * gear_module * tan(pressure_angle)) / rp * 180 / PI;
  union() {
    circle(r = rr);
    for (i = [0:pinion_teeth - 1])
      rotate(i * 360 / pinion_teeth) polygon([
        [rr * cos(-(half_p + d_root)), rr * sin(-(half_p + d_root))],
        [ra * cos(-(half_p - d_tip)), ra * sin(-(half_p - d_tip))],
        [ra * cos(half_p - d_tip), ra * sin(half_p - d_tip)],
        [rr * cos(half_p + d_root), rr * sin(half_p + d_root)],
      ]);
  }
}

// Rack teeth along +Y, pitch line at x = 0, teeth pointing +X.
module rack_teeth_2d(len) {
  p = PI * gear_module;                    // linear pitch
  half_p = (p / 2 - gear_backlash) / 2;    // half tooth width at pitch line
  dx_a = rack_tooth_h_add * tan(pressure_angle);
  dx_r = rack_tooth_h_ded * tan(pressure_angle);
  n = floor(len / p);
  intersection() {
    for (i = [0:n]) translate([0, i * p]) polygon([
      [-rack_tooth_h_ded, -(half_p + dx_r)],
      [rack_tooth_h_add, -(half_p - dx_a)],
      [rack_tooth_h_add, half_p - dx_a],
      [-rack_tooth_h_ded, half_p + dx_r],
    ]);
    translate([-rack_tooth_h_ded - 1, 0])
      square([rack_tooth_h_add + rack_tooth_h_ded + 2, len]);
  }
}

// ---- Pinion --------------------------------------------------------------

module pinion() {
  difference() {
    linear_extrude(gear_thickness) pinion_2d();
    // horn pocket in the back face (z = 0, faces the servo)
    translate([0, 0, -0.01]) {
      cylinder(d = horn_hub_d, h = horn_pocket_depth);
      for (a = [0, 90, 180, 270]) rotate(a)
        translate([0, -horn_arm_w / 2, 0])
          cube([horn_arm_r, horn_arm_w, horn_pocket_depth]);
    }
    // clearance for the horn's center screw + screwdriver
    translate([0, 0, -0.01])
      cylinder(d = horn_screw_clear_d, h = gear_thickness + 1);
    // M2 self-tappers through the gear into the horn arm holes
    for (a = [0, 90, 180, 270]) rotate(a)
      translate([horn_screw_r, 0, -0.01])
        cylinder(d = m2_selftap_hole, h = gear_thickness + 1);
  }
}

// ---- Rack ----------------------------------------------------------------
// Vertical bar: spine occupies x [0, rack_spine_w], teeth grow out the +x
// side and run UP through the lift module. The duck hangs off the BOTTOM
// (y < 0) on a bridge plate raised above the channel wall/lip height, so
// the bridge and duck sail past the module's guides as the rack climbs.
// The riser that lifts the bridge sits at x >= 4, clear of the lip (x <= 3).
// Print flat on the back face; the bridge wings need a dab of support.

module rack() {
  pitch_x = rack_spine_w + rack_tooth_h_ded;
  bridge_z0 = (lm_wall_h - lm_channel_floor_top) + rack_bridge_clear;
  linear_extrude(rack_thickness) {
    square([rack_spine_w, rack_len]);                // spine
    translate([pitch_x, 0]) rack_teeth_2d(rack_len); // teeth
  }
  // head: spine continues down, riser lifts the bridge over the wall
  translate([0, -rack_plate_h, 0])
    cube([rack_spine_w, rack_plate_h, rack_thickness]);
  translate([4, -rack_plate_h, rack_thickness])
    cube([rack_spine_w - 4, rack_plate_h, bridge_z0 - rack_thickness]);
  // duck mounting bridge (duck screws to its front face)
  translate([rack_spine_w / 2 - rack_plate_w / 2, -rack_plate_h, bridge_z0])
    difference() {
      cube([rack_plate_w, rack_plate_h, rack_plate_t]);
      for (sx = [-1, 1])
        translate([rack_plate_w / 2 + sx * duck_mount_spacing / 2,
                   rack_plate_h / 2, -0.5])
          cylinder(d = m3_hole, h = rack_plate_t + 1);
    }
}

// ---- Preview -------------------------------------------------------------

if (part == "pinion") pinion();
if (part == "rack") rack();
if (part == "mesh") {
  // rack pitch line on x = 0, pinion axis one pitch radius away
  translate([-(rack_spine_w + rack_tooth_h_ded), -rack_len / 2, 0]) rack();
  translate([pinion_pitch_r, 0, 0]) rotate(360 / pinion_teeth / 2) pinion();
}
