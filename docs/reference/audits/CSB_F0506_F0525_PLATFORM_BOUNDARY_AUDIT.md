# CSB F0506-F0525 Platform Boundary Source Audit

Authority: ReDMCSB `AMIGA.H`. The available source inventory names these
symbols but does not contain their Amiga implementation bodies. No authenticated
Amiga executable or trackdisk corpus is available to establish equivalent
runtime ownership.

| Range | Declared owner | Required missing evidence | Firestaff disposition |
| --- | --- | --- | --- |
| F0506-F0512 | Amiga allocation, shutdown, copper palette/screen/vblank, alt-key | Amiga binary plus chipset/copper and input evidence | Always reject; no allocation, palette, screen, timing, or input substitute. |
| F0513 | `DIALOG` ready-to-play draw | Amiga dialog/font/bitmap evidence | Always reject; no dialog or text output. |
| F0514 | `MOVE` sound selection | Amiga sound mapping evidence | Always reject; no sound routing. |
| F0515-F0517 | portrait-planar conversion and spell-area allocation | Amiga planar portrait and spell-buffer evidence | Always reject; no conversion/allocation. |
| F0518-F0525 | floppy/trackdisk/AmigaDOS | authenticated Amiga trackdisk/device/filesystem corpus | Always reject; no format, checksum, packet, device, or protection emulation. |

A real PC34 `GRAPHICS.DAT` cache is intentionally insufficient evidence for
these Amiga-specific contracts. The boundary exposes only that fact and never
creates a synthetic platform route.
