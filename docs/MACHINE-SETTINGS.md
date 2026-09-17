# Machine settings reference

Snapshot supplied by the user during commissioning, not a live EEPROM backup.
Firmware uploads do not transfer these settings to a replacement controller.
This is reference documentation, not a file to stream automatically.

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
$120=300.000
$121=300.000
$122=300.000
$130=800.000
$131=755.000
$132=20.000
```

Reported G54 offset: 3.990,3.990,0.000. G92 and tool-length offset were zero.
Re-establish work offsets for the actual job. Current firmware uses positive
XY machine coordinates; see POSITIVE-XY.md. Servo up/down are 1152/1024 us.
