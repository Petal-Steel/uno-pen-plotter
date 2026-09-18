# Machine settings reference

Current working settings supplied by the owner on 2026-09-17 from UGS `$$`
output, not an independently read binary EEPROM backup. The complete labeled
snapshot is [UGS-EEPROM-SETTINGS.txt](UGS-EEPROM-SETTINGS.txt).
This update changes X/Y acceleration from 300 to 400 mm/s²; all other supplied
values match the previous snapshot.
Firmware uploads do not transfer these settings to a replacement controller.
This is reference documentation, not a file to stream automatically.
The text snapshot is not a UGS-native settings export/import file. It records
`$$` settings only; work offsets (`$#`), startup blocks (`$N`) and build information
are not included. No settings were sent to the controller when saving this file.

```text
$0=10
$1=255
$2=0
$3=8
$4=0
$5=1
$6=0
$10=1
$11=0.010
$12=0.002
$13=0
$20=1
$21=1
$22=1
$23=3
$24=400.000
$25=2000.000
$26=244
$27=4.000
$30=1000
$31=0
$32=0
$100=40.100
$101=40.100
$102=40.100
$110=8000.000
$111=8000.000
$112=8000.000
$120=400.000
$121=400.000
$122=300.000
$130=800.000
$131=755.000
$132=20.000
```

Previously reported G54 offset: 3.990,3.990,0.000. G92 and tool-length offset
were zero in that earlier report; they were not supplied with this latest snapshot.
Re-establish work offsets for the actual job. Current firmware uses positive
XY machine coordinates; see POSITIVE-XY.md. Servo up/down are 1152/1024 us.
