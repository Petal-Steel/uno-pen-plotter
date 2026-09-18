# Proven Post Processing Sample

## Configuration responsibilities and evidence

This is an owner-reported working Fusion post-processing example, added 2026-09-18.
It was inspected as source here; Fusion was not run and no new machine execution
was performed as part of this import. The `.cps` is unchanged.

The machine does not need to have all of its physical configuration encoded in
Fusion's post or a Fusion machine definition. For this machine, runtime controller
settings are configured afterward through **UGS** and stored in the Uno's EEPROM.
UGS is the interface used to set them; GRBL stores and applies them.

| Layer | Responsibility |
|---|---|
| Custom GRBL firmware | Pin mapping, dual-X independent homing, Timer2 servo, M3/M5 interpretation, positive XY coordinate convention |
| Controller settings entered through UGS | Steps/mm, maximum rates, acceleration, travel, homing parameters, limit enable/polarity, work offsets |
| Fusion `.cps` post | Translate supported CAM operations into G-code, pen-up delay, end-of-job parking commands |
| UGS or Jetson sender | Establish a safe homed state, select work coordinates, stream commands and monitor status |

See [current EEPROM settings](../docs/MACHINE-SETTINGS.md) and
[Machine Context and Control Procedure](../docs/MACHINE-CONTEXT-AND-CONTROL-PROCEDURE.md).
Changing the post does not configure the controller's travel or calibration.

## Using this reference

1. Select the included `.cps` as the post configuration in Fusion's post-processing workflow.
2. Use the intended **2D cutting/jet toolpaths** with **millimeter output** and the correct work origin. This is not a general milling or multi-axis post for this firmware.
3. Check the `Pen lift delay (ms)` property; its default is 200.
4. Inspect generated output for M3/M5, lift dwells, XY-only motion, the intended work offset and final parking move.
5. Configure/verify the controller in UGS, home safely, and verify drawing extents in machine coordinates before running.

### Source-inspection limitations

- `writePenUp()` emits M5 and a dwell when `penLiftDelayMs > 0`.
- `onCommand()` maps jet power ON to M3 and power OFF to the pen-up sequence.
- `writeProgramEnd()` requests machine X4/Y4 at feed 2000 and then M30.
  These numeric parking values are not converted from millimeters for inch output;
  use millimeters for this unchanged example.
- Parking uses modal `xOutput`/`yOutput` formatting without explicitly forcing both
  words. If a previous work-coordinate value equals 4, an axis word can be omitted
  even though the G53 machine target differs. Inspect the generated final line:
  both X4 and Y4 must be present when both axes need repositioning. This import
  does not fix or expand the validated scope of the working source.
- Legacy milling/multi-axis handlers remain in the file. Their presence does not
  establish compatibility with this XY-only firmware.
- The drawing arc below was corrected in this documentation to use equal start/end
  radii. All snippets are examples and assume appropriate units, modes and clearance.

Post SHA-256: `89f9a8689beabdd82384cd86e0b5480c67fea8d8faf9a12220d1d41af2b85737`.

---

## Purpose of this section

This section contains a **known-good post-processing reference** for the pen plotter.

The included Fusion 360 `.cps` post processor represents a configuration that has been physically tested on the machine and has produced correct plotter behavior. It is kept here so future software, Jetson Nano code, alternate CAM/post-processing tools, or other developers can see exactly what a successful G-code output sequence should look like.

This section is intended as a **reference implementation**, not the only permitted way to generate G-code.

The machine-level behavior described here should remain consistent even if Fusion 360 is no longer used.

---

## What has been proven on the machine

The working post-processing behavior uses the following conventions:

### Pen control

```gcode
M3
```

means:

**Pen DOWN**

```gcode
M5
```

means:

**Pen UP**

The plotter does not use Z-axis motion to raise or lower the pen.

---

## Pen-lift delay

The physical servo requires a short amount of time to lift the pen clear of the drawing surface.

After `M5`, the known-good post inserts a short dwell before XY travel:

```gcode
M5
G4 P0.2
```

The current nominal delay is:

**200 ms**

This delay may be adjusted later if the servo, linkage, pen holder, or lift distance changes.

The purpose of the delay is to prevent the pen from dragging across the paper while the servo is still moving upward.

---

## Travel between disconnected paths

A correct travel sequence is:

```gcode
M5
G4 P0.2
G0 X... Y...
M3
```

Meaning:

1. Raise the pen.
2. Wait for the servo to physically clear the paper.
3. Rapid to the next drawing location.
4. Lower the pen.
5. Begin drawing.

XY travel should not begin immediately after `M5` unless the pen-lift mechanism has been proven fast enough to avoid dragging.

---

## Drawing motion

Typical drawing motion uses:

```gcode
G1
G2
G3
```

for linear and arc motion while the pen is down.

A typical segment looks like:

```gcode
M3
G1 X100 Y100 F2413
G2 X120 Y100 I10 J0
M5
G4 P0.2
```

---

## Machine coordinate convention

The customized GRBL installation uses the physical home-switch corner as:

```text
Machine X0 Y0
```

The usable table extends in the positive X and Y directions.

Approximate usable machine travel:

```text
X: 0 to +800 mm
Y: 0 to +755 mm
```

After homing, GRBL performs the configured 4 mm pull-off, so the normal parked position immediately after homing is approximately:

```text
X4 Y4
```

---

## End-of-job parking behavior

The proven post processor raises the pen and returns the carriage to the normal post-homing park position.

The expected end of a completed file is:

```gcode
M5
G4 P0.2
G53 G1 X4 Y4 F2000
M30
```

Meaning:

- `M5` — pen up
- `G4 P0.2` — wait approximately 200 ms for the pen to lift
- `G53` — use machine coordinates for this move
- `G1 X4 Y4 F2000` — return to machine X4/Y4 at 2000 mm/min
- `M30` — end program

`G53` is important because the return position must not depend on the current G54/work offset.

---

## Why this sample is stored in the repository

This section exists so that future software can compare its generated G-code against behavior already proven on the physical machine.

It can be used to:

- validate Jetson Nano G-code generation
- develop alternate post processors
- verify changes to the Fusion 360 post
- compare output from SVG/DXF conversion tools
- diagnose pen-drag or travel-order problems
- preserve known-good machine behavior after software changes
- help another developer understand the machine without reverse-engineering the GRBL firmware

---

## Important distinction

The `.cps` file is a **Fusion 360 implementation** of the machine's post-processing rules.

The machine itself is not dependent on Fusion 360.

Future software should reproduce the same functional behavior even if it generates G-code directly.

The important machine rules are:

```text
M3 = pen down

M5 = pen up

After pen up:
wait approximately 200 ms before XY travel

Machine home:
X0 Y0

Normal post-home park:
X4 Y4

End-of-job park:
G53 G1 X4 Y4 F2000

No Z-axis motion is required for pen control.
```

---

## Repository contents

- [Working Fusion post](grbl_pen_plotter_return_home.cps): byte-for-byte copy of the owner-supplied `.cps`; only the filename was simplified.
- This document: adapted from the supplied `PROVEN_POST_PROCESSING_SAMPLE.md`.
- No generated `.nc` file was supplied with this addition. The snippets here are illustrative, not a captured execution log.

The post retains its Autodesk copyright/legal notices. Its inclusion does not
relicense it under the GRBL firmware's GPL license.

---

## Maintenance note

If the known-good post processor is changed, update this document only after the new behavior has been physically tested.

Changes that should be documented include:

- pen-up delay
- pen-down behavior
- return-to-park position
- return speed
- machine coordinate convention
- use of G53
- changes to M3/M5 behavior
- any reintroduction of Z-axis commands

The purpose of this section is to preserve a reliable reference for behavior that has actually worked on the machine.
