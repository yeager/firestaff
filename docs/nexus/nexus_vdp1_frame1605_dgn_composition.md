# Nexus VDP1 frame 1605 retail composition replay

## Evidence

The capture-only regression replays the complete 320×224 VDP1 viewport from
authentic Japanese retail Nexus frame 1605. It validates the Ymir bundle and
pins SHA-256 hashes for the VDP1 framebuffer (FBS), VDP1 VRAM, VDP1/VDP2
register snapshots, full command trace, and bundle receipt. The full trace has
228,449 rows, zero dropped rows, and 151 command rows for frame 1605. The 150
nonzero-index command records match the final captured VDP1 VRAM snapshot;
row 0 is replayed from the trace because its command link is updated as part
of execution.

All 138 textured commands resolve to image bytes in hash-verified Japanese
retail `LEV01.DGN`: 131 indexed draws also resolve their 32-byte CLUT in that
file, and seven mode-5 draws resolve direct-colour image bytes. One flat
polygon, four local-origin updates, two user-clip updates, five skipped
commands, and the end command are replayed from the captured trace. The
capture-only Ymir stepper then reproduces all 71,680 pixels at native 320×224:
RGB555 values match the authentic VDP1 FBS exactly, and every opacity bit
matches exactly.

Run `nexus_v1_vdp1_frame1605_dgn_stepper_real` with
`FIRESTAFF_NEXUS_YMIR_FRAME1605_CAPTURE_DIR` pointing at the retained Ymir
capture root and `FIRESTAFF_NEXUS_DATA_DIR` pointing at the directory
containing the canonical Japanese `LEV01.DGN`. The CTest skips when external
capture/data inputs are absent and fails closed on hash or join mismatches.

## Limits

This verifies one emulator-captured VDP1 buffer and its retail source joins.
It is not a complete VDP1+VDP2 output comparison, independent Saturn hardware
parity, arbitrary-frame coverage, semantic ownership for every DGN face, or
production-renderer admission. The replay remains capture-only.
