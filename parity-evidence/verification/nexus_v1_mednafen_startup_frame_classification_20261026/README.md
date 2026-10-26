# Nexus Mednafen startup-frame classification (2026-10-26)

This receipt prevents a raw Saturn capture from being described as a title-menu
capture solely because an input pulse was scheduled. It uses the authentic
Japanese retail CUE and Japanese BIOS on TRV2; raw VRAM, RAM and video files
remain operator-local and are not committed.

The bounded capture window was frames `15980..16119`. It scheduled the
previously observed `0x0010` pulses at frames `15150..15209` and
`16000..16029`, and retained rendered receipts for frames 15980, 16000 and
16029. The frame-16029 PPM hash was
`e0cf839283659b6edaf7efb7f73c7d15a049d5a2decacd7736f8992da91c143b`.
Visual inspection identifies that frame as the retail TrueMotion logo rather
than the interactive Nexus title menu.

The raw capture transport validator accepted all 140 frames and observed active
VDP1 state. That is not a menu-consumer proof. In particular, this run must not
be used to reject or admit the `MENU.BPK` PALT route, identify a title-menu
asset owner, or enable Firestaff startup rendering.

`scripts/verify_nexus_saturn_capture_candidate.py` verifies that a local PPM
matches its manifest receipt and rejects an operator-supplied authenticated
non-menu hash. A non-rejected result is deliberately reported as
`unclassified`, never as a menu.

## Follow-up: retained Ymir state classification

On the same authenticated Japanese retail disc, an isolated Ymir 0.4.1
profile loaded the retained state
`ymir-title-prompt-slot-2.savestate` (SHA-256
`11d88ef517c408b3bca5c078c0ccb8081dbf3342f308239dd73403c73f0c3774`).
Its visible Nexus surface is the first-run Japanese date/time configuration,
not a title prompt. The local rendered receipt after dismissing the Ymir-only
update dialog had SHA-256
`3338a1f2f8dbc87336a56251f29d5b47683eadc40574a1441ace79c983b9d60b`.

The observed surface has editable year, month, day, hour and minute fields,
plus Cancel and Confirm controls. The tested Control Pad mapping was
`W/A/S/D` for the directional pad, `J/K/L` for A/B/C and `F` for Start.
Keyboard directional input visibly moves or changes the active date/time
field. An X11 mouse click over the visible Confirm control did not reach the
Saturn Control Pad in this Ymir profile; it must not be recorded as evidence
of mouse input support in Firestaff.

These screenshots and the temporary profile were deleted after inspection.
They are observational evidence only: they neither establish the original
Saturn asset owner for the surface nor authorize a host-drawn replacement.
