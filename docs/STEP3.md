# Step 3: dual-X auto-squaring

Implemented on the official 1.1h.20190825 baseline. Servo is not implemented.
The planner and Timer0/Timer1 configuration are unchanged.

## Connections

| Function | Shield connection | Uno pins |
| --- | --- | --- |
| X1 | X driver socket | STEP D2, DIR D5 |
| X2 | Independent A driver socket | STEP D12, DIR D13 |
| Y | Z driver socket | STEP D4, DIR D7 |
| Enable | All driver sockets | D8 |
| X1 NC series chain | One X limit connector | D9 to GND through both switches |
| Y NC series chain | One Y limit connector | D10 to GND through both switches |
| X2 NC series chain | One Z limit connector | D11 to GND through both switches |

Install only D12-to-A.STEP and D13-to-A.DIR routing links; no X/Y/Z clone links.
Y socket is empty. D3/D6 are not initialized or written as motion outputs; D3
is reserved for the later servo. SpnEn/SpnDir carry X2 STEP/DIR. See PINMAP.md
for the full schematic review and unpowered continuity checks.

## Changes

- Enable upstream dual-X with CNC_SHIELD_CLONE routing; disable VARIABLE_SPINDLE.
- Home both X sides independently, then Y, without a Z homing cycle.
- Map logical Y to Z socket. PEN_PLOTTER_XY removes physical Z stepping and
  returns zero Z step/direction masks, including for EEPROM inversion settings.
- Preserve upstream planner, step generation, independent homing locks,
  seek/locate/pull-off sequence and dual-approach failure-distance protection.
- Include the second switch's bit N_AXIS in the dual-axis pull-off release mask.
  Either X switch remaining active now produces ALARM:8 on pull-off.
- Reject Z words (including Z0/jogs), G18 and G19 with error:20. N_AXIS remains
  three to preserve GRBL structures and reporting. No physical Z output exists.
- Compile-time checks enforce motion/limit pin masks and X-then-Y homing.

Status uses upstream labels: X1 alone gives Pn:X, X2 alone gives Pn:XZ (shared
logical Z limit), Y gives Pn:Y. Test chains individually; Pn:X does not prove
both X switches activated. During normal motion, hard limits stop the machine;
independent per-motor stopping applies during homing.

## Defaults and EEPROM

Changed defaults: `$1=255` keeps the squared gantry energized; `$5=1` selects NC
series-to-ground polarity with internal pull-ups; `$22=1` enables homing;
`$23=3` homes X/Y toward negative ends. Hard/soft limits remain disabled by
default pending wiring/travel checks. Homing-enabled startup alarm is expected.

Flashing does NOT replace valid EEPROM settings. Inspect `$$` and explicitly
apply the required settings when commissioning. No settings were sent to a board.
Steps/mm, speed, acceleration, travel, pull-off and homing rates are still generic,
NOT calibrated. Verify direction before homing or mechanically coupling motors.
X1/X2 require matching microstep settings and mechanical steps/mm.

Upstream uses bit 0 (value 1) for X1, bit 1 (value 2) for Y, and bit 3 (value 8)
for X2 in `$3` direction inversion and `$2` step inversion. For example `$3=8`
reverses X2 alone when the other bits are zero. Preserve other desired bits.
Direction polarity may differ electrically while gantry travel must agree.

Mismatch protection retains the upstream allowance: 5% of configured Y travel,
bounded to 2.5–25 mm, after the first X switch triggers. Review these values
against permissible gantry skew and set actual travel before coupled tests.

Use XY G-code. Three-dimensional coordinate storage remains; review old EEPROM
offsets and G28/G30 positions before reuse. Stored Z coordinates cannot drive a
physical output. M3/M5 still control a digital output on A3 in this stage; they
do NOT move a pen servo. Flood owns A4. Neither pin is currently spare.

## Build results

Command: `pio run -e uno`. Platform/toolchain unchanged from Step 1.

| Resource | Step 1 | Step 3 | Available |
| --- | ---: | ---: | ---: |
| Flash | 29,762 B | 29,166 B (90.4%) | 32,256 B |
| Static RAM | 1,633 B | 1,553 B (75.8%) | 2,048 B |

Remaining: 3,090 bytes application flash, 495 bytes SRAM for runtime stack and
other dynamic use. This is not measured stack headroom.

Compilation and pin/homing checks pass. Source review covers both switch orders,
pull-off mask, output initialization and inversion. No upload or hardware tests
were performed. Verify motor directions, each limit, both orders of independent
X stopping, normal pull-off, each X switch stuck on pull-off (ALARM:8), missing
second-switch approach (ALARM:10), and absence of motion signals on D3/D6 with
mechanically disconnected motors. Also verify parser rejection of Z0/G18/G19 in
check mode. The full staged bench procedure follows the servo stage.
