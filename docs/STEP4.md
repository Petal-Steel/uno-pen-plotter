# Step 4: hardware pen servo

PEN_SERVO in config.h uses the spindle interface for M3=down and M5=up.
M4 is rejected with error:20. S values do not change position (including M3 S0).
The parser synchronizes earlier motion before changing the pen. Add G4 P0.3
after a pen command for mechanical settling; there is no automatic G-code dwell.
Repeated modal commands need not reapply the output. Check mode does not move it.

## Timer allocation

Timer0 still ends step pulses; Timer1 still schedules steps. Their initialization
and stepper ISR code are unchanged. Timer2 runs synchronous phase-correct PWM
mode 5 with prescaler 1024, OCR2A=156 and non-inverting OC2B on D3. Period is
19.968 ms (50.080 Hz); high time is OCR2B*128 us. No Timer2 ISR is installed.
OC2A is disconnected, leaving D11 as the X2 input despite OCR2A defining TOP.
The shared Timer0/1 prescaler is never reset by servo initialization.

PEN_UP_US=1024 and PEN_DOWN_US=1152 are configurable in config.h. Compile checks
require distinct multiples of 128 us from 1024 through 2048. Widths are not
angles; calibrate without linkage first. Swap the values if the mechanism moves
the wrong way. This resolution may be insufficient for some mechanisms; verify
before fitting the linkage. PEN_HOMING_SETTLE_MS defaults to 300.

The timer hardware produces pulse edges independently of interrupts. Command
updates use a short interrupt-protected byte write to avoid racing a reset's
pen-up request. Buffered OCR2B updates take effect at the timer update point;
pen-up is a request, not an instantaneous mechanical lift. Startup may contain
a partial initial frame; inspect startup behavior as part of the bench test.

## Wiring and operating behavior

Servo signal: D3 at Y.STEP breakout, NOT a motor winding terminal. Y driver socket
must be empty; Y motor is in Z socket. External regulated 5V powers the servo;
join servo supply ground, Uno ground and motor-supply ground. Do not feed servo
current from the Uno regulator. D12/D13 are X2 signals, not servo connections.

Startup initializes up. M5, program end and GRBL's spindle-stop/reset paths
request up while keeping the servo waveform active. Hard-limit reset requests
up. Homing explicitly lifts, waits the configured delay and resets the parser's
spindle mode so the next M3 can lower the pen. Review stored startup blocks:
upstream runs them after successful homing, and they could contain M3 or motion.
During physical reset/bootloader execution the firmware cannot generate pulses;
there is no guarantee of pen position when firmware or servo power is absent.

Ordinary feed hold retains the current pen state, matching stock GRBL spindle
behavior. Do not expect ! to lift. A reset aborts the job and requests up;
normal M5 waits for earlier motion. Sender spindle-stop override behavior is
inherited, and should not be confused with a position-confirmed lift.

A3 is no longer initialized or driven by the spindle interface and can be used
for a future peripheral with new firmware. D6 is also available. A4 still belongs
to flood coolant and A5 to probe. All three timers are allocated; a low-rate
idle-only peripheral is plausible, but independent concurrent stepping requires
additional timing analysis. The spare Y socket cannot be used unchanged because
its STEP net is the servo waveform. No peripheral support has been added.

## Actual build and checks

`pio run -e uno`: SUCCESS, AVR GCC 7.3.0, same platform as prior stages.

| Resource | Step 3 | Step 4 | Available |
| --- | ---: | ---: | ---: |
| Flash | 29,166 B | 29,280 B (90.8%) | 32,256 B |
| Static RAM | 1,553 B | 1,553 B (75.8%) | 2,048 B |

Remaining flash: 2,976 bytes. Remaining SRAM: 495 bytes for stack/other dynamic
use, not a measured runtime margin. Linked-symbol inspection confirms Timer2
vectors 7/8/9 remain weak defaults, with Timer1 COMPA and Timer0 overflow handlers
present. Stepper/planner sources match Step 3. No hardware upload, waveform
measurement or mechanical testing was performed. Follow BENCH-TEST.md.
