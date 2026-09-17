# Setup on another computer

## Requirements and scope

- Git and access to this private GitHub repository.
- VS Code with the PlatformIO IDE extension, or PlatformIO Core for command-line use.
- A host supported by the pinned PlatformIO Atmel AVR toolchain.
- USB access to an Arduino Uno only when uploading or controlling the machine.

Install PlatformIO using its [official installation instructions](https://docs.platformio.org/en/latest/core/installation/methods/installer-script.html).
In VS Code, use a PlatformIO terminal. In another terminal, configure PlatformIO's
[shell commands](https://docs.platformio.org/en/latest/core/installation/shell-commands.html)
so `pio` is on PATH. Do not copy another user's absolute executable path.

## Get the project

After configuring GitHub SSH authentication (a read-only deploy key is sufficient
for downloading), run:

```text
git clone git@github.com:Petal-Steel/uno-pen-plotter.git
cd uno-pen-plotter
```

HTTPS with your own GitHub authentication is an alternative:

```text
git clone https://github.com/Petal-Steel/uno-pen-plotter.git
```

The project can live anywhere. Run subsequent commands in the directory containing
`platformio.ini`. Open that directory as the VS Code project, particularly when
using a workspace containing several PlatformIO projects.

To retrieve subsequent updates with a clean working tree:

```text
git pull --ff-only
```

## Build

```text
pio run -e uno
```

The first build downloads the pinned platform/toolchain. GRBL itself is already
vendored in `grbl/`. Output is `.pio/build/uno/firmware.hex` and `firmware.elf`.
No Arduino framework or external source checkout is required. Building does not
upload or move the machine. `.pio/` and `.vscode/` are excluded from Git.

The firmware target is always the Uno/ATmega328P, not the host computer.
Windows builds are verified for this project. Linux/macOS and Jetson ARM builds
have not been validated here; availability of compatible binaries for the pinned
toolchain must be checked on the target host. A Jetson can clone/read the project
and supervise the Uno without compiling firmware locally. If the pinned build
tools are unavailable there, use a supported build computer rather than silently
changing platform versions or firmware configuration.

## Select the device and upload

Close UGS or another program holding the Uno serial port, then use
[PlatformIO device discovery](https://docs.platformio.org/en/latest/core/userguide/device/cmd_list.html):

```text
pio device list
pio run -e uno -t upload --upload-port PORT
```

Replace `PORT` with the connected Uno's actual port. Typical forms:

| Host | Example only |
|---|---|
| Windows | `COM4` |
| Linux / Jetson | `/dev/ttyACM0` or `/dev/ttyUSB0` |
| macOS | `/dev/cu.usbmodem...` |

Do not assume an example is your device. On Linux, arrange serial permissions
and udev rules according to PlatformIO's host setup instructions. Uploading
requires a connection to the Uno; a GitHub clone does not provide remote USB access.
An upload resets the controller. Follow BENCH-TEST.md before moving new hardware.
Connect the sender at 115200 baud afterward and establish a valid homed position.

## Regression tests

The existing `tests/run_review.py` is a Windows-specific AVR simulator harness:

```text
python tests/run_review.py
```

Use Python 3 after installing the PlatformIO toolchain. The script expects tools
under the current user's default `.platformio/packages/toolchain-atmelavr/bin`
and uses `.exe` names. Custom package directories and Linux/macOS require adapting
the runner; it also requires an AVR GDB build supporting `target sim`. This does
not affect ordinary firmware builds. Tests mock hardware boundaries and do not
replace bench testing. Historical results are in CODE-REVIEW.md.

## Machine-specific information

Portable host setup does not make this firmware suitable for arbitrary CNC wiring.
Retain the custom dual-X, logical-Y-on-Z-socket, limits and servo assignments in
PINMAP.md. MACHINE-SETTINGS.md is a reference snapshot from the commissioned
machine, not a settings file to apply blindly to a different machine. Calibration
is stored in the Uno's EEPROM and is not transferred by cloning this repository.

For runtime control and the intended Jetson role, see
[Machine Context and Control Procedure](MACHINE-CONTEXT-AND-CONTROL-PROCEDURE.md).
