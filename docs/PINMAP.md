# CNC Shield V3.00 pin and resource review (Step 2)

Status: documentation only. Firmware and platformio.ini still match the Step 1
baseline. The proposed assignments below are for subsequent implementation;
do not use the stock firmware to operate this proposed wiring.

## Evidence and revision boundary

The original Protoneer pages were unavailable when checked. I visually inspected
the [Protoneer V3.XX schematic archived by CMU](https://courses.ideate.cmu.edu/16-375/f2018/text/_images/Arduino-CNC-Shield-Schematics-V3.XX_.jpg).
Its Arduino symbol, paired end-stop nets, and A routing matrix are the electrical
basis of this table. This is a V3.XX family drawing, not proof of the exact PCB
revision of an individual clone. It includes an ES_Select ground/5V selector;
do not assume that selector exists on a board marked V3.00.

The [archived original Protoneer article](https://www.optimusdigital.ro/index.php?controller=attachment&id_attachment=220)
identifies V3.00's independent D12/D13 option and doubled limit connectors.
The [archived jumper photo](https://courses.ideate.cmu.edu/16-375/f2018/text/_images/A-axis-drivers.jpg)
shows the D12/D13 selection. Before wiring, verify the specific board by the
unpowered continuity checks below. No physical board measurements have been made.

MCU pin names follow the [Arduino Uno R3 pinout](https://docs.arduino.cc/resources/pinouts/A000066-full-pinout.pdf).
Timer behavior follows the [ATmega328P datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf),
Timer2 register tables 17-4 through 17-8. Firmware facts below come from the
vendored release, not current upstream master.

## Complete Uno pin map

Stock means GRBL 1.1h with VARIABLE_SPINDLE enabled, as currently compiled.
OC = hardware output compare. Pin-change interrupts are available on all listed
GPIO; the interrupt column names the resources relevant to this design.
An available alternate function does not mean its timer is free.

| Arduino | AVR port/bit | Shield V3.00 connection/net | Stock GRBL | Proposed final use | Hardware timer capability | Conflict / required action |
| --- | --- | --- | --- | --- | --- | --- |
| D0 | PD0 | RX comms header | Serial RX | Preserve USB serial RX | None | USART RX ISR retained |
| D1 | PD1 | TX comms header | Serial TX | Preserve USB serial TX | None | USART UDRE ISR retained |
| D2 | PD2 | X STEP, X socket, X.STEP breakout | X STEP | X1 STEP | None; INT0 input | Keep GRBL software step output |
| D3 | PD3 | Y STEP, Y socket, Y.STEP breakout | Y STEP | Reserve servo signal | OC2B; INT1 input | Move Y to Z socket; remove physical Z output ownership before servo use |
| D4 | PD4 | Z STEP, Z socket, Z.STEP breakout | Z STEP | Y STEP | T0 external clock input, no PWM | Remap logical Y here; Timer0 remains internally clocked |
| D5 | PD5 | X DIR, X socket, X.DIR breakout | X DIR | X1 DIR | OC0B; T1 external clock input | GPIO only; Timer0 occupied by pulse reset |
| D6 | PD6 | Y DIR, Y socket, Y.DIR breakout | Y DIR | Unassigned after XY-only output changes | OC0A | Not a free timer; stock masks still own it today |
| D7 | PD7 | Z DIR, Z socket, Z.DIR breakout | Z DIR | Y DIR | None | Remap logical Y here |
| D8 | PB0 | EN for all four sockets, EN breakout | Shared driver disable | Shared X1/X2/Y enable | ICP1 input | Active-low driver enable; no independent socket enables |
| D9 | PB1 | X+ and X- signal net | X limit | X1 NC switch chain | OC1A | PCINT1 in PCINT0 vector; Timer1 output disconnected |
| D10 | PB2 | Y+ and Y- signal net | Y limit | Y NC switch chain | OC1B | PCINT2 in PCINT0 vector; Timer1 output disconnected |
| D11 | PB3 | Z+ and Z- signal net | Spindle PWM | X2 NC switch chain | OC2A | Disable VARIABLE_SPINDLE; keep OC2A disconnected even when OCR2A is timer TOP |
| D12 | PB4 | SpnEn; selectable A STEP source | Z limit | X2 STEP via A socket | None | Remove from limit mask; select D12-to-A.STEP jumper |
| D13 | PB5 | SpnDir; selectable A DIR source | Spindle DIR | X2 DIR via A socket | None | Select D13-to-A.DIR jumper; also Uno LED/SPI SCK |
| A0 / D14 | PC0 | Abort | Software reset input | Retain Abort | None | PCINT8 in PCINT1 vector |
| A1 / D15 | PC1 | Hold | Feed hold / optional safety door | Retain Hold | None | PCINT9 in PCINT1 vector |
| A2 / D16 | PC2 | Resume | Cycle start | Retain Resume | None | PCINT10 in PCINT1 vector |
| A3 / D17 | PC3 | CoolEn | Flood output | Available after spindle-to-servo integration | None | Stock clone dual-axis mode assigns spindle enable here until Step 4 |
| A4 / D18 | PC4 | SDA comms breakout | Optional mist (disabled) | Reserve / optional coolant | None | Clone dual-axis mode moves flood here; must disable/reassign before peripheral use |
| A5 / D19 | PC5 | SCL comms breakout | Probe input | Retain probe or optional future GPIO | None | Probe code owns it; cannot reuse without disabling/reassigning probe |
| RESET | PC6 | RST button, E-STOP, RST breakout | Hardware reset | Preserve hardware reset | None | Not spare GPIO; preserve bootloader/reset behavior |

Other connections: GND is common logic/servo/supply return; shield 5V is logic
power; 3V3, VIN, IOREF and AREF are not extra digital outputs. PB6/PB7 serve the
Uno's 16 MHz oscillator and are not available. R3 SDA/SCL duplicate A4/A5;
ICSP duplicates D11/D12/D13, so neither header adds GPIO. The shield motor-supply
terminal feeds driver VMOT; it is not a servo 5V source.

## A socket: independent control is available

Use X socket for X1 and A socket for X2. In the A routing matrix, install only
the two links connecting D12 to A.STEP and D13 to A.DIR. Remove all X/Y/Z
clone-selection links. The matrix is separate from the microstep jumpers under
the driver modules. Do not bridge several source rows together.

SpnEn and SpnDir are electrically the same D12/D13 nets used by this selection.
They cannot simultaneously control a spindle or a servo. The A breakout follows
the selected A signals. Shared D8 enable does not prevent independent stopping:
homing suppresses STEP pulses for the reached motor while keeping both enabled.

## Existing GRBL dual-X implementation

Relevant source: [config.h](../grbl/config.h) dual-axis section,
[cpu_map.h](../grbl/cpu_map.h) DUAL_AXIS_CONFIG_CNC_SHIELD_CLONE,
[limits.c](../grbl/limits.c) limits_get_state()/limits_go_home(), and
[stepper.c](../grbl/stepper.c) TIMER1_COMPA_vect.

For Step 3, enable ENABLE_DUAL_AXIS, select X_AXIS and
DUAL_AXIS_CONFIG_CNC_SHIELD_CLONE, deselect PROTONEER_V3_51, and disable
VARIABLE_SPINDLE. This selects D12/D13 for X2 and D11 for its limit with minimal
changes. V3.51's A4/A3 routing does not describe this V3.00 board.

The source mirrors each normal X step into the dual output and applies separate
homing locks to X1 and X2. limits_get_state() carries the second motor on bit
N_AXIS in addition to its shared Z limit bit. Either X motor can stop first.
The existing seek/locate/pull-off sequence and dual-approach failure-distance
check should be retained. Electrical DIR polarity can differ for mirrored motors;
both motors must move the same gantry direction, with identical step counts.

Plan X-only homing followed by Y-only homing, with no Z homing cycle. Keep the
three-coordinate planner structure; do not blindly set N_AXIS to 2. The final
XY-only output layer must stop emitting physical Z STEP/DIR, remove those pins
from initialization/inversion masks, and prevent Z commands from affecting the
servo or remapped Y outputs. Merely changing the homing cycle is insufficient.
These are later code changes, not changes made in Step 2.

Two source details require explicit Step 3 validation:

- The pull-off failure check uses `limits_get_state() & cycle_mask`; an X-only
  cycle does not include the separate dual bit. Check X2 releases as well, and
  add a narrow dual-limit pull-off check if needed. Do not assume upstream
  independently validates both switches on pull-off.
- Stock dual-X status prints X when either X side is triggered and also Z for
  the shared D11 input. It does not provide distinct X1/X2 labels. Bench tests
  must exercise each switch separately and verify actual motor stopping.

## NC limit wiring

The schematic connects X+ and X- to one X-EndStop net, Y+ and Y- to one
Y-EndStop net, and Z+ and Z- to one Z-EndStop net. Six connectors provide only
three inputs. The + and - labels are travel ends, not electrical polarity.

Wire each motor's two switches in SERIES, using COM and NC on each switch:

```text
D9  / X signal ---- X1 negative NC ---- X1 positive NC ---- GND
D10 / Y signal ---- Y  negative NC ---- Y  positive NC ---- GND
D11 / Z signal ---- X2 negative NC ---- X2 positive NC ---- GND
```

Connect each complete chain to one signal/ground connector for its input.
Leave its duplicated connector unused. Plugging two individual NC switches into
the + and - connectors would put them in parallel: the closed switch would mask
the other opening. Keep X1 and X2 chains electrically separate.

For this wiring, retain internal pull-ups (DISABLE_LIMIT_PIN_PULL_UP undefined),
leave INVERT_LIMIT_PIN_MASK undefined, and set `$5=1`. Both switches closed pulls
the input LOW (normal); either opens and it rises HIGH (triggered). This follows
the actual inversion logic in limits_get_state(). The setting affects all three
inputs. Configure homing direction separately with `$23`; `$5` does not select
the homing end. These settings are documented here, not sent to hardware.

A broken series wire normally triggers the input; a short to ground can mask a
switch. Firmware cannot distinguish which end of one chain opened. Positive and
negative switches on one motor are hard-limit inputs, not separate home sensors.
Homing must start clear of the opposite-end switch.

## Timer allocation and proposed servo resource

| Resource | Actual stock use | Proposed allocation |
| --- | --- | --- |
| Timer1, OCR1A / COMPA ISR | Variable-period step-event scheduler | Preserve unchanged; OCR1B shares this counter and is not an independent servo timer |
| Timer0 overflow ISR | Ends each step pulse; timer starts/stops per pulse | Preserve unchanged |
| Timer0 OCR0A / COMPA ISR | Optional STEP_PULSE_DELAY | Preserve capability; neither Timer0 PWM output is an independent servo resource |
| Timer2, OCR2A / OC2A | Stock variable spindle PWM on D11 | Release by disabling VARIABLE_SPINDLE; later reserve Timer2 for servo |
| Timer2 OCR2B / OC2B | No stock timer output; D3 is GPIO Y STEP | Candidate hardware servo output on D3 |
| Watchdog | Optional limit debounce ISR, disabled in stock config | Retain existing role; not a precision motion/servo timer |
| USART / pin-change vectors | Serial / limits / control inputs | Preserve existing allocation |

The proposed servo uses Timer2 phase-correct PWM mode 5, synchronous 16 MHz
clock, prescaler 1024, OCR2A=156 (TOP), OCR2B for pulse width, and non-inverting
OC2B on D3. Calculation: period = 2*156*1024/16000000 = 19.968 ms, or 50.080 Hz.
High time = OCR2B*128 us; example settings 8, 12, 15 give 1024, 1536, 1920 us.
This is an engineering proposal derived from the datasheet, not a measured
waveform. Width resolution is coarse (128 us); calibrate the two pen positions
on the actual servo. Arbitrary microsecond/angle precision is not available.

OCR2A determines the period, so D11/OC2A cannot independently generate the desired
pulse width in this mode. Keeping COM2A disconnected and D11 configured as input
allows D11 to remain the X2 limit. Hardware OC2B requires D3; it cannot simply be
routed in software to SpnEn or CoolEn. No Timer2 ISR is needed, so pulse generation
adds no periodic interrupt load. Step 4 must leave Timer0/1 registers and their
shared prescaler untouched and validate output during motion and resets.

Consequently, the proposed Y motor/driver moves to the Z socket (D4 STEP, D7 DIR).
The Y socket stays empty; its Y.STEP breakout provides D3 for the servo signal.
This avoids PCB trace cuts or a separate timing ISR. Do not connect the servo
signal to the four-pin motor winding connector. Keep servo power on its external
regulated 5V supply and join grounds; do not use the Uno regulator for servo power.

If 128 us increments cannot give satisfactory pen positions, revisit pulse
generation before implementing Step 4. Finer timing on a generic GPIO would
require an interrupt-latency assessment; it must not be declared interference-free.
Arduino Servo/analogWrite are not part of this project and must not be added to
take over GRBL's timers. M3=down/M5=up remains the planned command convention,
using the existing synchronized spindle command path in a later stage.

## Remaining resources for a future peripheral

With the proposed final map, D6 and A3 can supply two GPIO once their old firmware
owners are removed. A4 can be reclaimed by disabling flood; A5 by disabling probe.
A0/A1/A2 remain useful controls. D0/D1 remain serial. No complete general-purpose
timer remains free after the hardware servo is implemented.

A low-rate STEP/DIR peripheral is realistic, particularly if operated only while
the plotter is idle. Smooth independent concurrent motion is not yet established:
it needs a rate/latency budget and measured step timing, within the remaining
flash/RAM. The spare Y driver socket's STEP net is now the servo signal, so it
cannot be used as-is for that motor. An external driver wired to spare GPIO, or
physical isolation/rerouting of that socket's STEP net, would be required.
No peripheral firmware is implemented or reserved in the planner.

## Unpowered continuity checks for the user's actual board

Disconnect USB, motor power and servo power before checking continuity.

1. Verify X+ signal to X- signal to D9; Y pair to D10; Z pair to D11.
2. Identify the ground side of each connector by continuity to GND. If the board
   has an ES_Select jumper, select GND for the NC series wiring above.
3. Verify SpnEn to D12 and SpnDir to D13.
4. With only independent-A links installed, verify A STEP to D12 and A DIR to D13;
   verify no direct clone links to X/Y/Z STEP/DIR remain.
5. Verify X socket STEP/DIR to D2/D5, Y socket to D3/D6, Z socket to D4/D7,
   and common enable to D8.
6. Verify each assembled NC chain is continuous at rest and opens when either
   switch is pressed. These checks establish PCB/wiring compatibility; powered
   motor, homing and servo tests belong to later stages.

## Step 2 validation

Documentation cross-checked against the schematic, pinout, datasheet and release
source. No firmware/build configuration changed; no rebuild or upload required.
The successful Step 1 build and memory figures remain the baseline. Hardware
continuity, signal timing and mechanical behavior remain untested.
