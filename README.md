# Uno Pen Plotter - custom GRBL firmware

## Status: commissioned and fully functional

On 2026-09-17, the machine owner confirmed all required pinout/wiring work and
all bench testing are complete. X1/X2/Y motion, independent dual-X squaring,
limits, homing, pen servo and positive XY coordinates are operational.
See the [commissioning sign-off](docs/BENCH-TEST.md). The future auxiliary
slate-cleaning stepper is not part of the implemented machine.

Current firmware uses [positive XY machine coordinates](docs/POSITIVE-XY.md),
independent dual-X homing, and Timer2 pen-servo control. Latest Uno build:
29,572 bytes flash, 1,553 bytes static RAM. Pen up/down: 1152/1024 us.
Hard-limit instantaneous state checking is enabled (not timed debounce).
See the [machine settings reference](docs/MACHINE-SETTINGS.md).
For the machine overview, operating sequence, and intended Jetson supervisor
behavior, read [Machine Context and Control Procedure](docs/MACHINE-CONTEXT-AND-CONTROL-PROCEDURE.md).

Earlier verification: [code review, fixes and clean-build results](docs/CODE-REVIEW.md).

Step 4: GRBL 1.1h (20190825) with dual-X auto-squaring for Arduino Uno R3,
ATmega328P, 16 MHz. X1 uses X socket, X2 uses independent A socket, and Y uses
Z socket. Hardware servo output is D3: M3 down, M5 up. See [servo details](docs/STEP4.md)
and the [bench-test procedure](docs/BENCH-TEST.md). See [Step 3 details](docs/STEP3.md)
for wiring, EEPROM settings, changes and actual build results.

Step 2 documentation is complete: see [the shield pin map and resource review](docs/PINMAP.md)
for verified schematic connections, the proposed machine pin table, NC wiring,
existing dual-axis support, timer allocation, and remaining resources.

## Project layout

```text
Uno Pen Plotter/
  platformio.ini         PlatformIO Uno build configuration
  grbl/                  Official source with documented machine changes
    main.c               GRBL entry point
    config.h             Machine compile-time configuration
    cpu_map.h            AVR pin assignments
    examples/            Upstream Arduino example, excluded from build
  COPYING                Upstream GPL license
  docs/
    SOURCE.md            Release tag, commit and import details
    BUILD.md             Actual baseline build results
    STEP3.md             Dual-X configuration and build results
    UPSTREAM-README.md   Original project README
  .gitignore
  .gitattributes
  .pio/                  Generated build output (Git-ignored)
```

## Build in VS Code

Open this directory in VS Code with the PlatformIO IDE extension. Use
PlatformIO: Build, or run in a PlatformIO terminal:

```text
pio run -e uno
```

For installation, cloning, serial-port selection and host-platform limitations,
see [portable setup instructions](docs/SETUP.md). All commands run from the
project root containing platformio.ini; no particular username or folder is required.

The first build downloads the pinned Atmel AVR platform and its toolchain.
Outputs are `.pio/build/uno/firmware.hex` and `firmware.elf`.
Building does not upload to a controller.

## Build configuration

The source is vendored directly in `grbl/`; no library download is needed.
GRBL provides its own C main() and hardware initialization, so `framework`
is intentionally omitted. There is no Arduino setup()/loop() wrapper.
PlatformIO supplies the MCU target and F_CPU from the board settings.

- `-Os`: optimize for size.
- `-flto`: link-time optimization, also used in upstream's AVR Makefile.
- `-ffunction-sections`, `-fdata-sections`, `-Wl,--gc-sections`: discard unused sections.
- `-lm`: AVR math library required by GRBL.

Machine feature macros are in config.h; no extra command-line defines are needed.
USB serial remains on D0/D1 at 115200 baud.

## Completed stages

1. Complete: CNC Shield V3.00 connections and pin/resource table.
2. Complete: configure and compile existing GRBL dual-X homing support.
3. Complete: Timer2 hardware pen servo implemented and commissioned.
4. Complete: owner-confirmed hardware bench testing; procedure retained for retesting.

Source provenance is in [docs/SOURCE.md](docs/SOURCE.md).
