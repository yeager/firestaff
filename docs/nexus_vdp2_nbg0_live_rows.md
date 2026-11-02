# Nexus NBG0 live-row bitmap replay

The capture-only NBG0 sampler uses Ymir's live per-output-row scroll and 8.8
coordinate increments, with captured final register values only for rows
without a live sample. It validates the frame's register profile before
replaying enabled layers. This path remains outside production-renderer
admission.

## Full-height authentic capture

The current real-data regression uses an authentic Japanese retail cold boot
captured with the Yeager Ymir fork. Its external, hash-manifested evidence is
at:

`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus-vdp2-full-frame7486-sameframe-20261102`

The capture binds VDP1 frame 7486 to Ymir VDP2 serial 8199, with frame-matched
registers (`BGON=0x0003`, `CHCTLA=0x0013`, `SPCTL=0x1536`, `PRINA=0x0404`,
`CCCTL=0x0043`, `CCRNA=0x1010`) and 224 unique row samples. All rows use the
observed scroll `0x1f79/0x0bcd`, increments `0xcd/0xcd`, priority 4, and
ratio-15 color calculation. The VDP2 memory receipt is complete, with no
VRAM/CRAM writes or dropped rows in the selected frame range.

The regression pins the VDP1 framebuffer pair, VDP2 VRAM/CRAM/registers, row
trace, and same-pass Ymir RGBX output by SHA-256. It also verifies the 4,096
NBG1 PND entries in the authentic 64×64 map and the transparent character data
at `0x20000` before permitting the NBG0-only capture replay. The CTest was
built and run on TRV2 with the external evidence configured: Firestaff matched
all 71,680 same-pass Ymir RGB pixels. Its capture-only receipt reports 21,455
NBG0 pixels and 50,225 VDP1 underlay pixels.

The independent Ymir layer analyzer also matches source colors and priorities
at all 71,680 positions and reconstructs the complete same-pass RGB image. See
[`docs/nexus/nexus_vdp2_layer_output_frame7486.md`](nexus/nexus_vdp2_layer_output_frame7486.md)
and the external capture's `README.md` and `SHA256SUMS.txt`.

Run the authentic C regression:

```sh
capture=/Volumes/Extern-disk/firestaff-evidence/nexus/nexus-vdp2-full-frame7486-sameframe-20261102
FIRESTAFF_NEXUS_NBG0_LIVE_CAPTURE_DIR="$capture" \
FIRESTAFF_NEXUS_NBG0_LIVE_ROW_TRACE="$capture/dumps/nexus-nbg0-vdp2-render-state.csv" \
  ctest --test-dir /path/to/firestaff-build --output-on-failure \
  -R '^nexus_v1_vdp2_nbg0_live_rows_real$'
```

This test returns skip code 77 when either evidence variable is missing. A CI
pass without the external capture therefore does not prove that the real-data
replay executed.

## Earlier screenshot capture

The earlier 2026-10-29 intro capture remains separate evidence at
`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_nbg0_cram_capture_20261029`.
Its GDB row trace at
`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_nbg0_live_row_state_20261029`
covers rows 4–116 and uses the same observed row tuple. Applying that tuple
previously matched all 286,720 samples across the 2× screenshot blocks; the
snapshot-only output matched 70,454 upper-left samples. That historical
screenshot comparison is not an input to the current full-height CTest.

A separate 2026-11-02 write-trace capture at
`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus-nbg0-vdp2-write-capture-20261102`
also samples rows 4–116 and reports zero VDP2 VRAM/CRAM writes during frame
7486. Its final snapshot is from frame 7663; it is not used as the full-height
test's register input.

## Limits

This verifies one authentic Ymir startup frame in a capture-only Firestaff
compositor. It does not establish retail source ownership/update timing across
other frames, NBG1 output, interactive menu transitions, dungeon/HUD states,
other frames, Saturn hardware parity, or production-renderer permission. The
retained SH-2 read/write traces are incomplete for exclusive final-writer
attribution; continue joining authentic DMA/CPU writes to the on-screen frame
before claiming source timing.
