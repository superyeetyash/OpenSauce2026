// housing.scad — laser-cut panels for the arcade cabinet. ALL 2D.
// part = "layout" (preview sheet, don't cut) | "front" | "back" | "side"
//      | "mount" | "base" | "deck"
// Quantities: side x2, mount x2 (one per duck row), everything else x1.
//
// Export one panel:  openscad -o front.dxf -D part="\"front\"" housing.scad
//
// Joinery: finger tabs on the front/back walls slot into notches on the
// side panels; the base floats base_lift above the wall bottoms on tabs
// through slot-holes; the two internal mount walls (which carry 3 lift
// modules each) tab into the base. Tabs are drawn laser_kerf oversize and
// slots laser_kerf undersize for a snug fit — set laser_kerf to your
// machine (or 0 if your laser software compensates).
//
// The mount walls also get a window behind each servo: the SG90 body is
// deeper than the printed plate, so it pokes through the wall.

include <params.scad>

part = "layout";

k = laser_kerf;
n_front = 4;   // finger pairs, front wall <-> sides
n_back = 5;    // back wall <-> sides
n_base_w = 4;  // base <-> front/back walls AND mount walls <-> base
n_base_d = 3;  // base <-> side panels

// Tab/slot strip along +Y from 0, mat_t thick in +X.
// grow=+k for tabs (drawn fat), -k for slots (drawn skinny).
module finger_strip(len, n, grow) {
  seg = len / (2 * n + 1);
  for (i = [0:n - 1])
    translate([-0.01, seg * (2 * i + 1) - grow])
      square([mat_t + 0.02, seg + 2 * grow]);
}

// Same pattern as a row of slot-holes along +X at height y0.
module slot_row(len, n, y0) {
  seg = len / (2 * n + 1);
  for (i = [0:n - 1])
    translate([seg * (2 * i + 1) + k, y0])
      square([seg - 2 * k, mat_t]);
}

// One lift module's wall cutouts, centered on the module's plate center.
module module_wall_cutouts() {
  for (sx = [-1, 1]) for (sy = [-1, 1])
    translate([sx * (lm_plate_w / 2 - lm_hole_inset),
               sy * (lm_plate_h / 2 - lm_hole_inset)])
      circle(d = wall_pilot_d);
  // servo body window (generous: it must line up through two layers)
  translate([lm_axis_x - (lm_x0 + lm_plate_w / 2),
             servo_body_l / 2 - servo_shaft_offset])
    square([servo_body_w + 2, servo_body_l + 2], center = true);
}

// ---- Panels ---------------------------------------------------------------

module panel_front() {
  difference() {
    union() {
      square([housing_w, front_wall_h]);
      translate([-mat_t, 0]) finger_strip(front_wall_h, n_front, k);
      translate([housing_w, 0]) finger_strip(front_wall_h, n_front, k);
    }
    slot_row(housing_w, n_base_w, base_lift);
  }
}

module panel_back() {
  difference() {
    union() {
      square([housing_w, housing_h]);
      translate([-mat_t, 0]) finger_strip(housing_h, n_back, k);
      translate([housing_w, 0]) finger_strip(housing_h, n_back, k);
    }
    slot_row(housing_w, n_base_w, base_lift);
  }
}

module panel_side() {
  difference() {
    square([housing_d, housing_h]);
    // notches where the front wall's tabs land (front edge, x = 0)
    translate([mat_t, 0]) mirror([1, 0]) finger_strip(front_wall_h, n_front, -k);
    // notches for the back wall (back edge)
    translate([housing_d - mat_t, 0]) finger_strip(housing_h, n_back, -k);
    slot_row(housing_d, n_base_d, base_lift);
  }
}

module panel_base() {
  difference() {
    union() {
      square([housing_w, housing_d]);
      // tabs into front and back walls (protruding outward)
      rotate(-90) finger_strip(housing_w, n_base_w, k);
      translate([0, housing_d]) mirror([0, 1])
        rotate(-90) finger_strip(housing_w, n_base_w, k);
      // tabs into the side panels
      translate([-mat_t, 0]) finger_strip(housing_d, n_base_d, k);
      translate([housing_w, 0]) finger_strip(housing_d, n_base_d, k);
    }
    // slot-holes for the two mount walls' bottom tabs
    for (d = [mount_wall_d1, mount_wall_d2]) slot_row(housing_w, n_base_w, d);
  }
}

module panel_mount() {
  difference() {
    union() {
      square([housing_w, mount_wall_h]);
      // bottom tabs into the base (base floats at base_lift, tabs pass through)
      rotate(-90) finger_strip(housing_w, n_base_w, k);
    }
    for (col = [-1, 0, 1])
      translate([housing_w / 2 + col * module_col_pitch,
                 module_mount_y + lm_plate_h / 2])
        module_wall_cutouts();
  }
}

module panel_deck() {
  difference() {
    square([housing_w, deck_d]);
    for (col = [-1, 0, 1]) for (row = [-1, 1]) {
      cx = housing_w / 2 + col * module_col_pitch;
      cy = deck_d / 2 + row * deck_row_pitch / 2;
      translate([cx, cy]) circle(d = button_hole_d);
      translate([cx, cy + deck_led_offset]) circle(d = led_hole_d);
    }
  }
}

// ---- Selection / preview sheet --------------------------------------------

module label(t) { text(t, size = 16, halign = "left"); }

if (part == "front") panel_front();
if (part == "back") panel_back();
if (part == "side") panel_side();
if (part == "base") panel_base();
if (part == "mount") panel_mount();
if (part == "deck") panel_deck();

if (part == "layout") {
  gap = 40;
  panel_front();
  translate([10, front_wall_h + 8]) label("front wall (x1)");
  translate([0, front_wall_h + gap]) {
    panel_back();
    translate([10, housing_h + 8]) label("back wall (x1)");
  }
  translate([0, front_wall_h + housing_h + 2 * gap]) {
    panel_mount();
    translate([10, mount_wall_h + 8]) label("mount wall (x2)");
  }
  translate([housing_w + 2 * mat_t + gap, 0]) {
    panel_side();
    translate([10, housing_h + 8]) label("side (x2)");
  }
  translate([housing_w + 2 * mat_t + gap,
             front_wall_h + housing_h + 2 * gap]) {
    panel_base();
    translate([10, housing_d + 8]) label("base (x1)");
  }
  translate([housing_w + 2 * mat_t + gap, housing_h + gap]) {
    panel_deck();
    translate([10, deck_d + 8]) label("button deck (x1)");
  }
}
