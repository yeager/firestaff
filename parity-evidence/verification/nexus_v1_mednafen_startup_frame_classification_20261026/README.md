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

## Follow-up: Ymir debug-memory input receipt

Ymir's Debug > Dump all memory action was used on the same isolated profile
immediately before and after one right-direction Control Pad input. The raw
dumps remained local and were deleted after comparison. Their SHA-256 receipts
were:

| Region | Before | After |
| --- | --- | --- |
| WRAM-H | `e0e2122264526eb7e7695b6c5b92f900938e94b02a74d9a8a4c7d95e187d41fb` | `f49c447c1189170cdb064ea5ca24d46ea5b306c818335b2491c8040ffcb1eb4d` |
| VDP2 VRAM | `07854d2fef297a06ba81685e660c332de36d5d18d546927d30daad6d7fda1541` | `5f3c6446774358ac37920e4064cdae54246dbdc6b3c01fc9c50d72948d2e89b5` |

The WRAM-H comparison changed 476,550 bytes across 28,892 ranges, so it is
not yet a stable field-level consumer receipt. VDP2 VRAM changed 2,816 bytes;
the first changed range is `0x40000..0x403ff`, followed by tile-like changes
from `0x60000`. This is strong evidence that the observed date/time surface is
live Saturn display state, but it remains Ymir-only diagnostic evidence and
does not satisfy the Mednafen-specific menu admission gate.

## Follow-up: bounded Mednafen startup capture

An instrumented Mednafen session captured 900 consecutive frames from the
authentic Japanese retail CUE with explicit Start-button pulses at frames
`300..329` and `600..629`. The raw capture SHA-256 was
`e19bcb3fafc2b8e8f5a473b696aee0bbac36c5c7f1be846358b15aa0fd8ba9b6`.
The transport validator accepted the 900-frame raw layout and observed VDP1
activity in 733 frames. Semantic admission remained blocked, as expected:
this bounded run does not yet bind a displayed title-menu surface to its
canonical asset owner and consumer. The 1.4 GiB operator-local raw capture
was deleted after validation.
