// params.scad — every shared dimension for the Duck Hunt mechanism.
// Units: millimeters. Edit here, and every part stays consistent.
//
// The one number that rules them all: rack travel per 180 degree servo
// sweep = PI * pinion pitch radius. With module 2 / 38 teeth the pitch
// radius is 38 mm -> ~119.4 mm of pop-up travel.

$fn = 64;

/* ---- Duck ---- */
duck_height = 150;      // beak-to-base silhouette height
duck_thickness = 12;    // extrusion thickness (prints flat)
duck_eye_d = 4;         // through-hole eye (0 = no eye)

/* ---- Gear train ---- */
gear_module = 2;        // tooth size; laser/print-friendly and chunky
pinion_teeth = 38;      // pitch dia = module * teeth = 76 mm
pressure_angle = 20;    // standard; rack flanks are straight at this angle
gear_thickness = 8;     // pinion face width = rack thickness
gear_backlash = 0.25;   // shaved off tooth thickness for 3D print fit

pinion_pitch_r = gear_module * pinion_teeth / 2;   // 38
pinion_tip_r = pinion_pitch_r + gear_module;       // 40
pinion_root_r = pinion_pitch_r - 1.25 * gear_module;
duck_travel = round(PI * pinion_pitch_r);          // ~119 mm per 180 deg

/* ---- Rack ---- */
rack_spine_w = 10;      // solid bar the teeth grow out of
rack_thickness = gear_thickness;
rack_tooth_h_add = gear_module;        // addendum above pitch line
rack_tooth_h_ded = 1.25 * gear_module; // dedendum below pitch line
// The rack rises UP through the lift module; the duck hangs off the
// BOTTOM of the rack on a bridge plate that passes in front of the
// channel wall. Toothed length = travel + 8 mm engagement margin each end.
rack_len = duck_travel + 16;
rack_plate_w = 30;      // duck mounting bridge at the bottom of the rack
rack_plate_h = 22;
rack_plate_t = 6;
rack_bridge_clear = 1.5; // bridge underside clearance over the channel wall
duck_mount_spacing = 16; // M3 hole pair, duck <-> rack bridge

m3_hole = 3.4;
m2_selftap_hole = 1.8;

/* ---- SG90 / MG90S micro servo (datasheet standard) ---- */
servo_body_l = 23.0;      // body length
servo_body_w = 12.4;      // body width
servo_window_clear = 0.5; // extra on the plate cutout
servo_hole_spacing = 27.8;   // mounting tab hole spacing
servo_tab_hole_d = 2.2;
servo_shaft_offset = 5.9;    // shaft axis from the near body end
// Flange contact plane -> underside of a horn pressed onto the shaft.
// MEASURE YOUR SERVO+HORN and tune; this sets how high the rack rides.
servo_face_to_horn = 9.5;

/* ---- Servo horn pocket (fits the 4-arm cross or 2-arm horn) ---- */
horn_hub_d = 8.4;         // horn hub outer dia + fit
horn_arm_w = 5.4;         // arm slot width
horn_arm_r = 17;          // arm slot length from center
horn_pocket_depth = 2.6;  // horn plate thickness + fit
horn_screw_clear_d = 5.5; // screwdriver access to the horn screw
horn_screw_r = 8;         // M2 self-tap circle into the horn arms

/* ---- Lift module (one per duck, print 6) ---- */
lm_plate_t = 4;
rack_clearance = 0.4;     // sliding fit around the rack
channel_wall_t = 4;
// Servo drops in from the front; tabs screw to the plate's front face and
// the body pokes through the plate (and the housing wall behind it).
// Rack must ride at the same height band as the pinion teeth:
lm_channel_floor_top = lm_plate_t + servo_face_to_horn;  // above plate back
// Horizontal distance, rack spine back face -> pinion axis:
lm_axis_x = rack_spine_w + rack_tooth_h_ded + pinion_pitch_r;   // 50.5
lm_x0 = -(channel_wall_t + 6);                       // plate left edge
lm_x1 = lm_axis_x + pinion_tip_r + 6;                // plate right edge
lm_plate_w = lm_x1 - lm_x0;
lm_plate_h = 110;                                    // gear dia + margin
lm_hole_inset = 5.5;      // corner mounting holes (to housing wall)
lm_hole_d = 4.4;          // M4 or #8 wood screws
lm_wall_h = lm_channel_floor_top + rack_thickness + 4.5;  // wall + lip top

/* ---- Housing (laser cut) ---- */
mat_t = 6;                // laser stock thickness (plywood/acrylic)
housing_w = 540;          // interior width (3 ducks + gaps)
housing_d = 260;          // interior depth
housing_h = 300;          // side / back wall height (rack tips rise high)
front_wall_h = 185;       // the wall ducks hide behind
mount_wall_h = 230;       // internal walls carrying the lift modules
mount_wall_d1 = 95;       // row 1 wall, distance from front (interior)
mount_wall_d2 = 195;      // row 2 wall
base_lift = 10;           // base floats this far above the wall bottoms
module_col_pitch = 170;   // duck-to-duck horizontal spacing
// Vertical position of the lift modules on the mount walls: puts the gear
// mesh high enough that the duck (hanging at the rack's bottom) rests just
// above the base and hides behind the front wall.
module_mount_y = 86;      // lift module plate bottom above mount wall bottom
wall_pilot_d = 3.2;       // pilot holes in walls for the module screws
laser_kerf = 0.15;        // per-side kerf compensation, applied to joints

/* ---- Button deck ---- */
deck_d = 160;             // deck panel depth
button_hole_d = 12;       // ADJUST to your button caps / arcade buttons
led_hole_d = 5.2;         // 5 mm RGB LED press fit
deck_row_pitch = 70;
deck_led_offset = 18;     // LED hole above its button
