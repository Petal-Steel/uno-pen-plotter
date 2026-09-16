"""Build actual GRBL C with selected I/O boundaries wrapped; run AVR simulator.
No upload. This verifies code paths/register setup, not timers or real switches.
"""
import pathlib
import subprocess

root = pathlib.Path(__file__).resolve().parents[1]
tools = pathlib.Path.home() / '.platformio/packages/toolchain-atmelavr/bin'
out = root / '.pio/review'
out.mkdir(parents=True, exist_ok=True)
sources = [p for p in (root / 'grbl').glob('*.c') if p.name != 'main.c']
wrapped = ['protocol_buffer_synchronize', 'protocol_execute_realtime', 'delay_ms',
           'limits_go_home', 'plan_sync_position', 'gc_sync_position',
           'settings_read_coord_data', 'mc_line']
elf = out / 'review.elf'
subprocess.run([str(tools/'avr-gcc.exe'), '-mmcu=atmega328p', '-DF_CPU=16000000UL',
                '-Os', '-g', '-ffunction-sections', '-fdata-sections',
                '-I'+str(root/'grbl'), str(root/'tests/review.c'),
                *map(str, sources), '-Wl,--gc-sections',
                *['-Wl,--wrap='+s for s in wrapped], '-lm', '-o', str(elf)], check=True)
result = subprocess.run([str(tools/'avr-gdb.exe'), '-batch', str(elf),
                         '-ex', 'target sim', '-ex', 'load',
                         '-ex', 'break tests_finished', '-ex', 'run',
                         '-ex', 'printf "FAILURE_LINE=%u\\n", failure_line'],
                        text=True, capture_output=True, timeout=30)
print(result.stdout)
if result.returncode or 'FAILURE_LINE=0' not in result.stdout:
    raise SystemExit('AVR regression failed: '+result.stderr)
print('PASS: actual AVR code regression checks (hardware boundaries mocked).')
