# Machine Context and Control Procedure

## Document status and control notes

Owner-supplied machine context, recorded 2026-09-17. Hardware commissioning is
complete; see [bench-test sign-off](BENCH-TEST.md). The Jetson procedures below
specify intended supervisory behavior, not software implemented by this document.
Settings are a reference snapshot; verify the controller with `$$` rather than
writing settings automatically. See [machine settings](MACHINE-SETTINGS.md),
[pin map](PINMAP.md), and [positive XY coordinates](POSITIVE-XY.md).
The current [Fusion post processor](../post-processors/grbl_pen_plotter_final.cps)
and [verification NC sample](../post-processors/samples/pen-plotter-verification.nc)
are included in this repository. See the [post-processing guide](../post-processors/README.md)
for current behavior and validation status. Pen-up commands include the configured
lift dwell (default 200 ms), including before initial XY travel and final parking;
short command examples below illustrate sequencing rather than exact full output.

- Serial connection: 115200 baud, 8 data bits, no parity, 1 stop bit. Opening USB
  serial may reset the Uno. A connection to an already-running controller may not
  produce a fresh startup banner; use a bounded connection/state-check procedure.
- Before the motion examples, establish `G21 G90` (millimeters, absolute mode),
  confirm a valid homed position and send `M5`. Coordinates and feed rates in this
  document assume millimeters and mm/min.
- An ordinary motion command's `ok` acknowledges acceptance, not physical
  completion. Drain outstanding acknowledgments and confirm `Idle` before marking
  the job complete. Homing must finish successfully before normal job streaming.
- Pen commands set a servo target, not measured pen position. Allow a validated
  mechanical settling interval when needed; no automatic settling wait is added
  by this document. `M5` is synchronized with queued motion, not an emergency stop.
- If position is uncertain, inhibit jobs and establish that homing is safe before
  issuing `$H`; uncertainty alone must not trigger automatic movement. Clear the
  cause of an alarm before recovery. Treat an unexplained disconnect/reconnect as
  loss of supervisory position confidence.
- GRBL reset/abort is the real-time Ctrl-X byte (`0x18`), including during homing.
  Normal feed hold is not a homing-abort mechanism. In this custom build, critical
  limit alarms can require reset before normal status/command handling resumes.
- The NC loops are normally LOW and become HIGH when opened. X2 may report `Pn:XZ`
  because of its retained logical-Z alias; this does not indicate a physical Z axis.
- Software bounds include X=0/Y=0, but these are switch locations. Use the normal
  X4/Y4 parked position and validated physical clearance for routine jobs. Bounds
  validation must account for units, G54/G92 offsets, incremental moves and arc
  extrema, not just line endpoints or drawing width/height.
- Servo power is external regulated 5 V with common ground; up/down pulse widths
  are 1152/1024 us. Hard-limit force-state checking is enabled, but is not timed
  debounce. `$26` is a homing phase delay, not continuous-active qualification.

The numbered sections below preserve the supplied machine context and procedure.


PURPOSE

This document describes the physical machine, customized GRBL controller,
coordinate system, homing behavior, pen-control commands, safety behavior,
and intended responsibilities of the Jetson Nano supervisory computer.

The Jetson Nano should use this information when communicating with the
Arduino Uno running the customized GRBL firmware.

The Arduino/GRBL controller is responsible for real-time motion control.

The Jetson Nano is responsible for higher-level machine supervision,
job generation, G-code transmission, startup sequencing, state checking,
and deciding when homing is appropriate.

## 1. MACHINE DESCRIPTION

Machine type:
2-axis CNC pen plotter

Usable travel:

X approximately 800 mm
Y approximately 755 mm

Primary motion axes:

X = dual-motor gantry
Y = single motor

The X gantry uses TWO independently driven motors:

X1
X2

X1 and X2 independently home against their own switches so the gantry can
auto-square during the homing cycle.

The machine must preserve independent X1/X2 homing.

## 2. CONTROLLER

Motion controller:

Arduino Uno R3
ATmega328P
CNC Shield V3 style hardware
Customized GRBL firmware

Host/supervisory controller:

Jetson Nano

The Jetson communicates with GRBL over serial/USB.

The Jetson must NOT attempt to directly generate step pulses.

All actual motor motion must be commanded through GRBL.

## 3. CUSTOM MOTOR/PIN ARRANGEMENT

This machine does NOT use the CNC shield in its normal axis arrangement.

Logical X1:
STEP = Arduino D2
DIR  = Arduino D5
Physical CNC shield X driver

Logical X2:
STEP = Arduino D12
DIR  = Arduino D13
Physical CNC shield A driver

X2 is independently controlled.
It is NOT simply hardware-cloned from X1.

Logical Y:
STEP = Arduino D4
DIR  = Arduino D7

The physical Y motor is connected to the CNC shield's Z DRIVER SOCKET.

This is intentional.

The logical Y axis remains Y inside GRBL even though its motor is physically
connected to the shield Z driver.

Do NOT assume physical shield socket names correspond to logical GRBL axes.

## 4. LIMIT SWITCHES

There are six physical normally-closed limit switches:

X1 negative
X1 positive

X2 negative
X2 positive

Y negative
Y positive

Each pair of normally-closed switches is wired in SERIES for its respective
input.

Limit inputs:

X1 limit = Arduino D9
Y limit  = Arduino D10
X2 limit = Arduino D11

Current GRBL limit polarity:

$5 = 1

Normal switch condition:
NC circuit closed
input inactive

Triggered switch condition:
NC circuit opens
input becomes active

Do NOT invert $5 unless the electrical system is deliberately redesigned.

## 5. LIMIT INPUT EMI FILTERING

The plotter previously suffered false hard-limit alarms and false Y homing
detections caused by motor-induced electrical noise.

Hardware filtering was added:

D9  -> 10 nF capacitor -> GND
D10 -> 10 nF capacitor -> GND
D11 -> 10 nF capacitor -> GND

These capacitors solved the observed false-trigger problem.

The oscilloscope probe itself previously affected the circuit enough to
improve homing, which helped identify high-frequency noise as the cause.

The Jetson should NOT assume that every historical hard-limit problem was
caused by firmware.

If unexpected hard-limit events return, inspect the physical electrical
system as well as software.

## 6. PEN SERVO

The pen is controlled by an RC servo.

Servo signal:

Arduino D3 / PD3 / OC2B

Servo timing uses Timer2.

The physical CNC shield Y driver socket must remain unused because D3 is
used for the servo.

Do NOT modify Timer2 without understanding the pen-servo implementation.

## 7. PEN G-CODE COMMANDS

The customized firmware interprets:

M3 = PEN DOWN
M5 = PEN UP

Typical drawing sequence:

M5
G0 X... Y...
M3
G1 ...
M5

Meaning:

M5
Raise pen before travel.

G0
Rapid reposition with pen raised.

M3
Lower pen.

G1/G2/G3
Draw path.

M5
Raise pen before leaving the path.

The post processor intentionally does not use Z motion for pen control.

## 8. MACHINE COORDINATE SYSTEM

This GRBL installation has been customized so that the physical homing
switch corner is machine coordinate:

X0 Y0

The table extends in the POSITIVE X and POSITIVE Y directions.

Coordinate layout:

HOME
X0 Y0
 |
 |-------------------------> +X
 |
 |
 v
+Y

Usable machine coordinate area is approximately:

X = 0 to +800 mm
Y = 0 to +755 mm

The machine should NOT be treated as having a conventional GRBL
negative-coordinate work envelope.

## 9. HOMING

GRBL homing is enabled.

Relevant settings:

$20 = 1    Soft limits enabled
$21 = 1    Hard limits enabled
$22 = 1    Homing enabled
$23 = 3    Homing direction mask
$24 = 400  Homing locate/feed speed
$25 = 2000 Homing search speed
$26 = 244  Homing debounce
$27 = 4    Homing pull-off distance

Maximum travel:

$130 = 800 mm X
$131 = 755 mm Y

IMPORTANT:

$23 = 3 is intentional.

Do NOT change $23 merely to alter displayed coordinate direction.

The physical homing motion is currently correct.

## 10. POST-HOMING POSITION

The physical switches define approximately:

X0 Y0

After successful homing, GRBL pulls away from the switches by:

$27 = 4 mm

Therefore the normal carriage position immediately after homing is
approximately:

MPos:4,4

Small deviations may exist due to machine calibration.

A typical verification command is:

?

The expected result after homing should show an Idle state and machine
position near:

X = 4
Y = 4

## 11. STARTUP STATE

GRBL may start in an ALARM/locked state because machine position is not yet
known.

This behavior is intentional.

The Jetson should NOT blindly issue:

$X

simply to unlock the machine and then assume coordinates are valid.

Unlocking without homing does not establish a trustworthy machine position.

If the machine position is unknown, the preferred recovery is:

$H

## 12. RECOMMENDED JETSON STARTUP PROCEDURE

The Jetson should supervise startup.

Recommended sequence:

1. Open the serial connection to GRBL.

2. Wait for the GRBL startup response.

3. Query machine status.

Command:

?

4. Determine GRBL state.

Possible relevant states include:

Alarm
Idle
Run
Hold

5. If GRBL is in its normal startup alarm because machine position has not
   been established, determine whether it is safe to home.

6. If safe, issue:

$H

7. Wait for homing to finish.

Do NOT send normal motion commands while homing is active.

8. Confirm that GRBL reaches:

Idle

9. Query status:

?

10. Confirm machine position is approximately:

X4 Y4

11. Only after successful homing should the Jetson consider machine
coordinates valid and enable normal job execution.

## 13. AUTOMATIC HOMING POLICY

The Jetson may eventually perform automatic homing after application startup,
but homing should be initiated by the Jetson rather than automatically from
inside GRBL firmware.

Reason:

A GRBL reset should not automatically cause unexpected machine movement.

The Jetson can make a higher-level safety decision before issuing $H.

For example, the Jetson may eventually check:

- controller communication healthy
- no job currently active
- E-stop healthy
- enclosure/interlock state
- operator permission
- machine startup state

before commanding homing.

## 14. DO NOT AUTOMATICALLY HOME AFTER EVERY RESET

A USB reset, watchdog reset, controller reset, electrical disturbance, or
software restart should not automatically cause physical motion merely because
GRBL restarted.

The Jetson should determine WHY the controller restarted and decide whether
another $H command is appropriate.

## 15. G53 MACHINE COORDINATE COMMANDS

G53 means use MACHINE coordinates for that move.

Example:

G53 G1 X4 Y4 F2000

This returns the carriage to approximately the normal post-homing parking
position.

G53 applies to the commanded move and does not redefine the G54 work offset.

Do NOT replace this with a normal:

G1 X4 Y4

unless the intention is specifically to move to X4/Y4 in the CURRENT WORK
COORDINATE SYSTEM.

## 16. WORK COORDINATES

Drawing programs normally use G54 work coordinates.

G54 allows a drawing origin to be located anywhere appropriate on the table.

Machine coordinates and work coordinates must remain conceptually separate.

Example:

Machine coordinate:
G53 X4 Y4

always refers to the same physical position.

Work coordinate:
G54 X4 Y4

depends on where the current G54 origin has been established.

## 17. END-OF-JOB BEHAVIOR

The custom Fusion 360 post processor is configured to return the carriage to
approximately the post-homing position at the end of every completed program.

Desired program ending:

M5
G53 G1 X4 Y4 F2000
M30

Meaning:

M5
Raise pen.

G53
Use machine coordinates.

G1 X4 Y4 F2000
Return to machine X4/Y4 at 2000 mm/min.

M30
End program.

The return speed is intentionally 2000 mm/min rather than using unrestricted
G0 rapid speed.

## 18. CURRENT MOTION SETTINGS

Current known GRBL settings:

$0  = 10
$1  = 255
$2  = 0
$3  = 8
$4  = 0
$5  = 1
$6  = 0

$10 = 1
$11 = 0.010
$12 = 0.002
$13 = 0

$20 = 1
$21 = 1
$22 = 1
$23 = 3
$24 = 400
$25 = 2000
$26 = 244
$27 = 4

$30 = 1000
$31 = 0
$32 = 0

$100 = 40.100
$101 = 40.100
$102 = 40.100

$110 = 8000
$111 = 8000
$112 = 8000

$120 = 400
$121 = 400
$122 = 300

$130 = 800
$131 = 755
$132 = 20

## 19. STATUS REPORTING

Use:

?

to request current GRBL status.

The Jetson should parse the GRBL state rather than depending only on timing.

Examples:

<Idle|...>
<Run|...>
<Hold|...>
<Alarm|...>

Do not assume that a command has completed merely because enough time has
passed.

Wait for the appropriate GRBL state.

## 20. JOB TRANSMISSION

The Jetson will eventually create and/or load G-code and stream it to GRBL.

The sender should respect GRBL's serial buffering protocol.

Do NOT transmit an entire large file blindly without flow control.

Track:

- commands transmitted
- acknowledgments ("ok")
- errors
- alarm messages
- controller state
- serial buffer usage if implemented by the sender

If GRBL returns an error, the Jetson should identify the failed command and
stop or handle the condition deliberately rather than continuing blindly.

## 21. MACHINE STATE MODEL

The Jetson should maintain its own supervisory state.

Suggested high-level states:

DISCONNECTED
CONNECTING
GRBL_STARTUP
NEEDS_HOME
HOMING
READY
JOB_RUNNING
JOB_PAUSED
JOB_COMPLETE
ALARM
ERROR

The Jetson's READY state should mean more than merely:

GRBL says Idle.

READY should mean:

- communication healthy
- GRBL Idle
- machine has been successfully homed
- machine coordinate system considered valid
- no known alarm condition

## 22. HOMING VALIDITY FLAG

The Jetson should maintain a logical flag such as:

machine_homed = true/false

Set:

machine_homed = true

only after a successful $H cycle and appropriate position/state verification.

Set:

machine_homed = false

after events that make machine position uncertain, such as:

- controller reset
- loss of stepper position
- hard-limit interruption when position integrity is uncertain
- emergency stop
- power loss to motors
- loss of Arduino power
- other conditions capable of causing position loss

## 23. JOB START REQUIREMENTS

Before starting an automatic drawing, the Jetson should verify at least:

GRBL state = Idle

machine_homed = true

no known alarm

G-code is valid for the machine

expected drawing extents fit within the usable machine area

pen begins in UP state

A useful pre-job command is:

M5

to explicitly ensure the pen is raised.

## 24. DRAWING BOUNDS

Approximate physical machine envelope:

X = 0 to 800 mm
Y = 0 to 755 mm

The Jetson should preferably calculate the bounding box of generated G-code
before running it.

When using work coordinates, the Jetson should consider:

work origin
+
drawing minimum/maximum coordinates

and ensure the resulting MACHINE position remains inside the allowed table
area.

Do not merely check that drawing dimensions are smaller than 800 x 755.

The drawing can still exceed the table if its work origin is badly located.

## 25. SAFETY AND ERROR PHILOSOPHY

When uncertain about machine position:

HOME.

When uncertain whether the pen is raised:

M5.

When GRBL reports an alarm:

Do not continue streaming normal motion commands.

Determine the alarm condition first.

Do not automatically issue $X and continue a job unless the software
specifically knows that preserving the previous position is valid.

Do not automatically modify GRBL settings during routine startup.

## 26. FIRMWARE RESPONSIBILITIES

The Arduino/GRBL firmware should remain responsible for:

- deterministic step generation
- acceleration
- motion planning
- hard limits
- soft limits
- homing motion
- dual-X auto-squaring
- limit-switch interpretation
- pen-servo actuation
- machine coordinate tracking

## 27. JETSON RESPONSIBILITIES

The Jetson should be responsible for:

- user interface
- job creation
- G-code generation
- G-code validation
- drawing-boundary validation
- serial communications
- startup state determination
- deciding when to initiate $H
- verifying homing success
- tracking whether machine position is trustworthy
- streaming G-code with proper flow control
- detecting errors and alarms
- job pause/resume logic
- job completion logic
- logging
- eventual higher-level safety/interlock logic

## 28. IMPORTANT DESIGN RULE

Do not move real-time motion-control functionality from GRBL to the Jetson.

The Jetson determines WHAT the machine should do.

GRBL determines HOW the motors execute that motion.

## 29. KNOWN GOOD BASIC TEST

Pen test:

M5
M3
M5

Expected physical result:

pen UP
pen DOWN
pen UP


Homing test:

$H

then:

?

Expected:

GRBL Idle
machine position approximately X4 Y4


Parking test:

G53 G1 X4 Y4 F2000

Expected:

Carriage moves to approximately the same physical location occupied
immediately after successful homing.

## 30. CURRENT INTENDED NORMAL CYCLE

POWER ON

-> Jetson connects to Arduino/GRBL

-> Jetson reads controller state

-> machine position initially considered UNKNOWN

-> Jetson determines homing is safe

-> send $H

-> wait for homing completion

-> confirm Idle

-> verify MPos approximately X4 Y4

-> mark machine_homed = true

-> ensure pen up with M5

-> establish/select required work origin

-> validate generated G-code bounds

-> stream drawing

-> GRBL executes drawing

-> final M5 raises pen

-> final G53 G1 X4 Y4 F2000 returns carriage to home parking position

-> M30 completes program

-> Jetson confirms Idle

-> job marked COMPLETE
