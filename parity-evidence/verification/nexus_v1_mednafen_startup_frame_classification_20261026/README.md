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
