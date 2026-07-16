// lift_module.scad — one servo lift carriage per duck (print 6, identical).
// part = "module" (printable bracket) | "assembly" (bracket+rack+pinion)
//
// Render:  openscad -o lift_module.stl -D part="\"module\"" lift_module.scad
// Prints flat on its back plate, no supports (the rack-retaining lip is
// chamfered at 45 degrees).
//
// How it works: the plate screws to a laser-cut housing wall (4 corner
// holes). The servo drops in from the front — its body passes through the
// plate window (and a matching window in the wall behind), tabs screwed to
// the plate face. The pinion, screwed to the servo horn, meshes with the
// vertical rack sliding in the C-channel. Gear separating force presses
// the rack INTO the channel wall, so the guide only needs light retention
// on the open side (the two corner stubs).

include <params.scad>
use <rack_pinion.scad>

part = "assembly";

yc = lm_plate_h / 2;                       // pinion axis y (and mesh point)
spine_top_z = lm_channel_floor_top + rack_thickness;
servo_center_y = yc + (servo_body_l / 2 - servo_shaft_offset);

module lift_module() {
  difference() {
    union() {
      // back plate
      translate([lm_x0, 0, 0]) cube([lm_plate_w, lm_plate_h, lm_plate_t]);
      // channel floor (raises the rack to the pinion's height band)
      translate([-rack_clearance, 0, lm_plate_t])
        cube([9.5 + rack_clearance, lm_plate_h,
              lm_channel_floor_top - lm_plate_t]);
      // channel wall + 45-degree chamfered lip, full height
      translate([0, lm_plate_h, 0]) rotate([90, 0, 0])
        linear_extrude(lm_plate_h) polygon([
          [-rack_clearance - channel_wall_t, lm_plate_t],
          [-rack_clearance - channel_wall_t, lm_wall_h],
          [3, lm_wall_h],
          [3, lm_wall_h - 0.7],
          [-rack_clearance, spine_top_z + rack_clearance],
          [-rack_clearance, lm_plate_t],
        ]);
      // open-side retainer stubs, clear of the gear's swept circle
      for (y0 = [0, lm_plate_h - 10])
        translate([rack_spine_w + rack_tooth_h_add
                   + rack_tooth_h_ded + 1, y0, lm_plate_t])
          cube([8, 10, lm_wall_h - lm_plate_t]);
    }
    // corner holes to the housing wall
    for (x = [lm_x0 + lm_hole_inset, lm_x1 - lm_hole_inset])
      for (y = [lm_hole_inset, lm_plate_h - lm_hole_inset])
        translate([x, y, -0.5]) cylinder(d = lm_hole_d, h = lm_plate_t + 1);
    // servo body window (shaft lands exactly on the pinion axis)
    translate([lm_axis_x, servo_center_y, lm_plate_t / 2])
      cube([servo_body_w + servo_window_clear,
            servo_body_l + servo_window_clear, lm_plate_t + 1], center = true);
    // servo tab screw holes
    for (sy = [-1, 1])
      translate([lm_axis_x, servo_center_y + sy * servo_hole_spacing / 2, -0.5])
        cylinder(d = servo_tab_hole_d, h = lm_plate_t + 1);
  }
}

if (part == "module") lift_module();

if (part == "assembly") {
  lift_module();
  // rack mid-travel in the channel
  color("orange") translate([0, (lm_plate_h - rack_len) / 2,
                             lm_channel_floor_top]) rack();
  // pinion on the servo axis
  color("steelblue") translate([lm_axis_x, yc, lm_channel_floor_top])
    rotate(360 / pinion_teeth / 2) pinion();
}
