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
The sibling `.grbl-1.1h-upstream/` checkout is only an import/reference copy;
building this project does not depend on it.

PlatformIO compiles the root `grbl/*.c` files directly, including upstream main.c.
The Arduino import example is retained for provenance but excluded from the build.
No Arduino core, sketch wrapper, or external library is required. Stock config.h
already selects DEFAULTS_GENERIC and CPU_MAP_ATMEGA328P. No feature defines are
added; dual-axis remains disabled and stock variable spindle remains enabled.
