# Bench test: X1, X2, Y and pen

## Commissioning complete — owner confirmation, 2026-09-17

The machine owner confirms that all required pinout/wiring work and all bench
testing have been completed, and the current pen plotter is fully functional.
This records owner-reported hardware acceptance, not an automated test result.

- X1/X2/Y motion and calibration: complete.
- Independent dual-X homing/auto-squaring and pull-off: complete.
- All six NC switches, hard limits and soft limits: complete.
- D3/Timer2 pen servo (M3 down, M5 up): complete.
- Positive XY machine coordinates and UGS visualizer alignment: complete.

Installed motor routing: X1 on X, X2 on independent A (D12/D13), Y on Z;
the shield Y driver socket remains empty. Limit inputs are D9/X1, D10/Y,
D11/X2, each with a permanent 10 nF capacitor to GND. Servo up/down pulse
widths are 1152/1024 us. See [settings reference](MACHINE-SETTINGS.md) and
[positive-coordinate details](POSITIVE-XY.md).

The future auxiliary slate-cleaning stepper is outside this completed scope.

## Retained commissioning/retest procedure

For routine operation and intended Jetson supervision, see
[Machine Context and Control Procedure](MACHINE-CONTEXT-AND-CONTROL-PROCEDURE.md).

The instructions below are retained for rebuilding or retesting the machine;
they are not outstanding tasks. Initial-test settings below are not instructions
to overwrite the commissioned settings ($20=1 and $21=1).

Perform initial motor tests mechanically disconnected from the gantry. Do not
plug/unplug motors or change drivers/jumpers while powered. Set driver current
limits for the actual motor and driver before motion. Confirm the PINMAP.md
continuity checks and independent A routing first.

## 1. Prepare and upload

Use X socket for X1, A for X2 (D12/D13 routing), Z for Y; leave Y socket empty.
Wire each pair of NC switches in series to its input and GND. Initially leave
servo linkage off and servo power disconnected. Connect USB, identify the Uno
port, and upload only after checking the configuration:

```text
pio device list
pio run -e uno -t upload --upload-port PORT
```

Replace PORT with the actual Uno port reported by `pio device list`; it is a
placeholder, not a literal port name. Examples include COM4 on Windows,
/dev/ttyACM0 on Linux, or /dev/cu.usbmodem... on macOS. Identify your own device.
See [SETUP.md](SETUP.md) for host setup and USB access.
Connect a GRBL sender at 115200 baud, one serial application at a time.
Inspect `$$`, `$#`, `$N` (startup blocks) and `$I`. Remove unexpected automatic
startup motion before commissioning. Preserve needed settings before edits.

Apply/verify `$1=255`, `$5=1`, `$22=1`, `$23=3`, `$20=0`, `$21=0` for initial
disconnected tests. These are commissioning settings, not full calibration.
Set actual steps/mm ($100/$101), sensible speed/acceleration, travel ($130/$131),
homing feed/seek ($24/$25) and pull-off ($27). Begin with slow homing rates.
Steps/mm = motor full steps/rev * microsteps / travel per motor revolution.
Both X drivers must use matching microstepping and mechanical ratios.

## 2. Limits with motion disabled

With hard limits off, send `?` after each operation. At rest, no limit letters
should appear. Test each of the SIX switches separately:

| Open switch | Expected Pn letters |
| --- | --- |
| X1 negative or X1 positive | X |
| X2 negative or X2 positive | XZ |
| Y negative or Y positive | Y |

Release each switch and verify the letters clear. Disconnect one wire in each
chain and verify the corresponding indication. XZ is expected because X2 shares
the unused logical Z input. Test one chain at a time: combined status does not
distinguish X2-only from both X switches active. Do not proceed if a switch is
masked by another closed switch; that indicates parallel NC wiring.

## 3. Direction and ordinary motion

Keep motors mechanically disconnected. If in startup homing alarm, use `$X`
only for these controlled tests. With motor power on, send one short jog at a time:

```gcode
$J=G91 G21 X1 F60
$J=G91 G21 X-1 F60
$J=G91 G21 Y1 F60
$J=G91 G21 Y-1 F60
```

X1 and X2 must both turn on each X jog and reverse together. Determine whether
their rotation would move the installed gantry in the SAME direction (mirrored
mounting can require opposite shaft rotations). Change $3 bits as needed:
X1=1, Y=2, X2=8; XOR the relevant bit while preserving the others. Repeat tests.
Y jogs must move only the motor in the Z socket. Confirm no unexpected motor
movement on M3/M5 once the servo is connected.

With a logic analyzer if available, count D2 and D12 pulses during ordinary X
jogs: counts must match. A small sequential-port-write skew is normal. Verify
step widths and DIR setup time against the installed driver's requirements.

## 4. Independent X homing stops and pull-off

Keep motors uncoupled. All switches start released. An already-active switch
now blocks homing with ALARM:1; release it before retrying. Use `$H`; X should seek in
its configured negative direction. Operate the negative switches by hand:

1. Trigger X1 first. X1 must stop stepping; X2 must continue.
2. Trigger X2 promptly, before the configured skew-distance guard trips.
3. During pull-off, both motors move away. Release both switches during this
   movement, before pull-off finishes.
4. Repeat the switch action on the slow locate approach, then release both on
   final pull-off. GRBL performs multiple approach/pull-off phases.
5. Y then homes: trigger Y negative on each approach and release during pull-off.
6. Repeat the full test with X2 triggered before X1. This time X2 must stop while
   X1 continues. Finish both X and Y homing normally.

Successful homing finishes without alarm and leaves all three input chains clear.
Observe pulse outputs if hand timing makes motor stopping hard to distinguish.
The guard is distance-based: calibrate steps/mm and allowable skew; do not enlarge
it just to make a mechanically coupled test easier.

Fault tests, still uncoupled: hold only X1 active through pull-off, then repeat
holding only X2 active. Each must produce ALARM:8. Trigger one X switch and never
the other: expect ALARM:10 when mismatch distance is exceeded (or approach failure
if the search ends first). Reset after each fault, release switches and rehome.
Test a missing Y switch produces approach failure. Never use these tests to rack
an attached gantry. Recheck real switch travel and $27 before coupling motors.

## 5. Pen waveform and calibration

Verify D3/Y.STEP relative to GND with an oscilloscope/logic analyzer before
connecting the servo: approximately 19.968 ms period, 1.024 ms up, 1.152 ms down.
Connect external regulated 5V servo power and common grounds; leave linkage off.
After unlocking if needed, send:

```gcode
M5
G4 P0.3
M3
G4 P0.3
M5
G4 P0.3
```

Verify direction and end positions. Edit PEN_UP_US/PEN_DOWN_US in config.h in
128 us increments, rebuild/upload, and retest before attaching linkage. Ensure
neither position forces a mechanical stop. Increase dwell if needed. M3 works
without S; S does not specify an angle. M4 must return error:20.

Repeat waveform measurements while jogging X and Y. Period/width should remain
stable; X1/X2 step counts must still match outside homing. Test pen-up on software
reset and before homing, and check startup behavior after physical reset. A lift
needs a valid servo frame and mechanical travel time; it is not instantaneous.
Feed hold alone is expected to retain the pen position.

## 6. Sequencing, parser and final limits

In `$C` check mode, verify Z0, G18, G19 and M4 return error:20; M3/M5 must not
physically move the servo. Exit check mode (GRBL resets), then unlock/rehome as
needed. On the disconnected rig, verify ordering with:

```gcode
G21 G91
M5
G4 P0.3
G1 X2 F60
M3
G4 P0.3
G1 Y2 F60
M5
G4 P0.3
G90
```

The X move finishes before lowering; Y starts after the dwell; lifting follows
completion of Y. Once inputs and motion are verified, set `$21=1` and individually
open each of the six switches: expect ALARM:1 and pen-up request. Reset and rehome
after alarms; release switches before restarting. Enable soft limits ($20=1)
only after homing, directions, workspace coordinates and actual travel are verified.

Record each pass/fail, settings, pulse measurements and firmware commit. Couple
the motors only after both independent-stop orders and all pull-off tests pass.
