# Uno pen plotter — stock GRBL baseline

Step 3: GRBL 1.1h (20190825) with dual-X auto-squaring for Arduino Uno R3,
ATmega328P, 16 MHz. X1 uses X socket, X2 uses independent A socket, and Y uses
Z socket. Servo support is not implemented. See [Step 3 details](docs/STEP3.md)
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

```powershell
pio run -e uno
```

On this computer the explicit executable is:

```powershell
& 'C:\Users\willi\.platformio\penv\Scripts\pio.exe' run -e uno
```

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

## Next stages

1. Complete: CNC Shield V3.00 connections and pin/resource table.
2. Complete: configure and compile existing GRBL dual-X homing support.
3. Explain timer allocation, implement pen-servo control, and compile again.
4. Provide a staged bench-test procedure for the configured machine.

Source provenance is in [docs/SOURCE.md](docs/SOURCE.md).
