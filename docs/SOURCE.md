# Source provenance

- Repository: https://github.com/gnea/grbl
- Release: https://github.com/gnea/grbl/releases/tag/v1.1h.20190825
- Tag: `v1.1h.20190825`
- Commit: `40eb439bf2afc4c6f3003836c396e183ba72c45b`
- Firmware identifiers: `1.1h`, build `20190825`.
- Imported the complete upstream `grbl/` directory without edits, plus `COPYING`
  and the upstream README (stored as `docs/UPSTREAM-README.md`).
- License: GPL-3.0-or-later; see COPYING and individual file headers.

Source is vendored so subsequent machine changes can be reviewed in this project's
Git history. It is not a floating dependency or a fork downloaded from a third party.
The original import used a separate upstream reference checkout. That checkout
is not required: this repository contains the source needed to build the firmware.

PlatformIO compiles the root `grbl/*.c` files directly, including upstream main.c.
The Arduino import example is retained for provenance but excluded from the build.
No Arduino core, sketch wrapper, or external library is required. Step 1 used
unchanged stock settings. Step 3 adds the machine configuration and narrow fixes
described in STEP3.md. The original source is preserved in Git milestone
9c1761d; the working source now contains those documented changes.
