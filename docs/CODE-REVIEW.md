# Code review and clean build

Historical review of the earlier firmware revision. For current build size and
commissioning status, see the [README](../README.md) and [bench tests](BENCH-TEST.md).
For host setup, see [SETUP.md](SETUP.md).

Reviewed current custom changes against the stock GRBL baseline, including port
ownership, Timer2 setup, reset paths, NC limit handling and coordinate handling.

## Confirmed issues fixed

1. Shared negative/positive switch inputs: enabled upstream
   LIMITS_TWO_SWITCHES_ON_AXES. Homing now refuses an already-active limit input
   (ALARM:1), because firmware cannot distinguish the home end from the other end.
   This applies equally to the NC series wiring used here. Release all switches
   before homing, including after a hard-limit alarm.
2. Reset during the pen-up settling delay: process the pending realtime reset
   before returning from mc_homing_cycle. This sets sys.abort so system_execute_line
   does not treat the return as success and execute post-homing startup blocks.
3. G28/G30 with old EEPROM coordinates: hold the unused Z coordinate at its current
   value rather than allowing a stored Z position to enter the motion plan. This
   prevents phantom Z distance from changing XY feed planning/parser position.

## Checks performed

- Clean PlatformIO build: SUCCESS. Flash 29,344 / 32,256 bytes (91.0%);
  static SRAM 1,553 / 2,048 bytes (75.8%). Remaining 495 bytes SRAM must cover
  runtime stack and other dynamic use; stack margin has not been measured.
- AVR instruction-simulator regression suite passes. It compiles actual project
  C sources, wrapping EEPROM, motion and realtime boundaries. Checks cover M3/M5,
  rejection of M4/Z/G18/G19, G28/G30 with nonzero stored Z, servo register setup,
  no pen-down over a pending reset, check-mode servo suppression, NC input decoding,
  no-Z masks, Y output mapping, pending reset during homing delay, active-limit
  precheck, and two homing-cycle calls when inputs are clear.
- Linked Timer2 vectors 7/8/9 remain weak defaults: no added timer ISR.
  Timer1 COMPA and Timer0 overflow handlers remain present.
- git diff --check passes.

Run regression checks from the project directory on Windows with Python 3 and
the PlatformIO AVR toolchain installed:

```text
python tests/run_review.py
```

The current runner locates tools under the current user's default `.platformio`
directory and calls `.exe` executables. It is not yet portable to Linux/macOS or
custom PlatformIO package locations. See SETUP.md for these limitations; removing
personal paths from this guide does not change the runner's supported platforms.

The suite uses bundled avr-gcc/avr-gdb and builds into .pio/review. It does not
upload. Firmware builds exclude tests because src_dir is grbl. The instruction
simulator does NOT model physical switch contacts, servo PWM timing, step-pulse
timing, or gantry behavior. Homing motion itself is mocked, not dynamically tested.
Follow BENCH-TEST.md for those checks.

## Remaining warnings

Two upstream eeprom.c warnings remain at lines 133 and 144: logical OR is used
between shifted checksum values. Both read and write use the same legacy checksum.
Changing || to | would change the EEPROM checksum format and potentially invalidate
stored settings. Left unchanged deliberately; this review does not migrate EEPROM
or claim stronger error detection than stock GRBL.

No firmware was uploaded and no physical tests were performed during this review.
Subsequent owner-confirmed hardware commissioning is recorded in BENCH-TEST.md.
