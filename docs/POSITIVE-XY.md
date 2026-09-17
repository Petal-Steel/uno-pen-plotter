# Positive XY machine coordinates

## Scope and source trace

This change uses the existing PEN_PLOTTER_XY configuration. It changes only
limits_go_home()'s final X/Y coordinate assignment and system_check_travel_limits()'s
X/Y bounds. Physical homing motion, direction masks, dual locks, search distances,
locate cycles, release checks, hard limits, step timing and servo code are unchanged.
The exact source/test diff is in POSITIVE-XY.patch. Earlier config.h changes (servo
positions and hard-limit state checking) are preserved and excluded from that patch.

The actual position variable is sys_position[], measured in integer steps.
settings.c stores $130-$132 as negative max_travel values. Previously, with $23=3,
limits_go_home() assigned round((max_travel + homing_pulloff) * steps_per_mm).
With travel 800/755 and pull-off 4 this produced approximately -796/-751 mm.
Integer-step rounding explains small differences in reported decimals.

HOMING_FORCE_SET_ORIGIN is disabled. Enabling that stock option would assign zero
at the pulled-off position, not the requested switch-corner zero. The XY branch
now takes precedence over that stock option. N_HOMING_LOCATE_CYCLE remains 1:
search, pull-off, locate, final pull-off. Final coordinate assignment occurs only
after those phases. X1/X2 still use separate locks and the original mismatch guard.

For X/Y, negative-end homing ($23 bit set) assigns:

    round(homing_pulloff * steps_per_mm)

Positive-end homing ($23 bit clear) assigns:

    round((-max_travel - homing_pulloff) * steps_per_mm)

Thus $23=3 yields approximately +4/+4 without changing physical direction.
Z keeps its existing handling. max_travel remains negative internally, preserving
homing search distances, dual mismatch distances, settings storage and reporting.

The shared travel check now permits 0 <= X <= -max_travel[X] and
0 <= Y <= -max_travel[Y]. Both ordinary soft-limit checks and jog validation use it.
The range is inclusive; actual physical clearance still requires bench validation.

Project source inspection found no further active negative-range assumptions
requiring changes. Planner/stepper positions and system_convert_array_steps_to_mpos()
operate on signed coordinates without assuming negative travel. G53 bypasses work
offsets naturally in gcode.c. G54/G92 arithmetic is unchanged. Parking has a negative
target convention but PARKING_ENABLE is disabled in this build; do not enable it
without a separate review. G28/G30 stored positions and saved work offsets refer to
the old machine origin and must be reviewed/re-established. No EEPROM settings or
stored positions are changed automatically. Review startup blocks with $N before
first homing, since GRBL executes them after successful homing.

## Validation

PlatformIO environment uno / ATmega328P builds successfully.

| Resource | Before | After |
|---|---:|---:|
| Flash | 29,352 | 29,572 bytes |
| Static SRAM | 1,553 | 1,553 bytes |

tests/run_review.py passes on the AVR instruction simulator. Added checks cover
both inclusive XY bounds, negative and upper overtravel on each axis, G53 offset
bypass, and ordinary work-coordinate translation with nonzero offsets. Existing
servo register, parser, limit translation and homing-entry checks also pass.
Hardware boundaries are mocked: these tests do not execute physical homing or
measure pulse timing. Homing final assignment was source-reviewed; physical
auto-squaring and the resulting position must be confirmed on the machine.

## UGS bench procedure after upload

1. Keep the pen raised and provide clearance. Save $$, $# and $N output. Confirm
   $23=3, $27=4, $130=800, $131=755, and $20/$21/$22=1. Do not change motor
   direction, polarity, speed or calibration settings. Review any startup moves.
2. Send M5, then $H. Verify the same physical X squaring and Y homing as before.
3. Send ?. Expect MPos approximately 4,4,0, within integer-step rounding.
4. Send G21 G90, then G53 G1 X9 Y4 F100. Confirm X moves 5 mm away from home.
5. Send G53 G1 X9 Y9 F100. Confirm Y moves 5 mm away from home.
6. First return slowly with G53 G1 X4 Y4 F100. Verify the post-homing location.
7. Test G53 G0 X9 Y9, then G53 G0 X4 Y4 only after the slow tests pass.
   G0 uses configured rapid rates, not the preceding F100.
8. Check soft-limit rejection without approaching the ends using jog commands:
   $J=G21 G90 G53 X-1 F100, then X801, Y-1 and Y756 variants. Each should be
   rejected with travel-exceeded error and no movement. Test commands individually.
9. Re-establish the intended G54 work origin; verify normal work-coordinate moves.
   Review saved G28/G30 positions before using them or older G-code files.
10. With the machine stationary, verify each hard-limit loop still alarms when
    opened. Reset and rehome after these tests before resuming normal operation.

No upload was performed as part of this change.
