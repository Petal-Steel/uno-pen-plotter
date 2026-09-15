# Stock build results

Result: SUCCESS, `pio run -e uno`.

## Build environment

- PlatformIO Core: 6.2.0
- Platform: platformio/atmelavr 5.1.0 (pinned in platformio.ini)
- Toolchain: toolchain-atmelavr 1.70300.191015, AVR GCC 7.3.0
- Target: Arduino Uno, ATmega328P, 16 MHz
- Configuration: release, no Arduino framework, unchanged GRBL 1.1h.20190825

## Actual memory usage

| Resource | Used | Available | Usage | Remaining |
| --- | ---: | ---: | ---: | ---: |
| Application flash | 29,762 B | 32,256 B | 92.3% | 2,494 B |
| Static RAM | 1,633 B | 2,048 B | 79.7% | 415 B |

The flash limit reserves 512 bytes of the MCU's 32 KiB for the Uno bootloader.
The 415 bytes of RAM remaining must accommodate the runtime stack and any other
dynamic use; it is not a measured runtime free-memory guarantee.

The build produced `.pio/build/uno/firmware.elf` and `firmware.hex`.
No firmware was uploaded and no hardware operation was tested.

## Warnings

AVR GCC emitted two `-Wint-in-bool-context` warnings in upstream eeprom.c,
lines 133 and 144, for `(checksum << 1) || (checksum >> 7)`.
These do not prevent compilation. The upstream expressions were deliberately
preserved to keep this baseline unmodified.

## Source verification

SHA-256 file comparisons against the checkout of the official release verified
all 43 imported files in grbl/ are byte-for-byte identical before Git import.
