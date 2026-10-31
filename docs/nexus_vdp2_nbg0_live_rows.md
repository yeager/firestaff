# Nexus NBG0 live-row bitmap replay

The capture-only NBG0 sampler accepts per-output-row Ymir scroll and 8.8
coordinate increments. It uses the captured final register values on rows
without a live render-state sample. This covers authentic Nexus intro frames
where the end-of-frame NBG0 registers differ slightly from the values used to
render visible rows.

## Authentic evidence

Raw input and the original Ymir screenshot are retained outside Git at
`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_nbg0_cram_capture_20261029`.
The GDB row trace and prior replay are retained at
`/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_nbg0_live_row_state_20261029`
and `../nexus_nbg0_ymir_replay_20261029`. These captures use the hash-verified
Japanese retail disc and matching Saturn IPL; no synthetic game data is used.
The row trace is `gdb-row-state.log` (SHA-256
`1f1ddff055c5189e8b42db9b15936135601f43bc7320873c14e0269a323c40ff`). The
real-data test verifies this hash, the single frame-counter marker `7486`, the
complete row sequence 4–116, and each row's NBG0 transparency, scroll,
increment, priority, and color-calculation fields before supplying row states
to the C compositor. It also hash-pins the VDP2 register and VRAM dumps,
checks the captured NBG1 register profile, and verifies all 4,096 PNDs in its
64×64 map are `(attr=0, charno=0x1000)`. The corresponding 32-byte 4bpp
character pattern at `0x20000` is all zero, proving transparent index-zero
pixels for every visible NBG1 cell regardless of the live map scroll.

The trace observes one consistent live row tuple over output rows 4–116:
NBG0 H/V scroll `0x1f79/0x0bcd` and H/V increments `0xcd/0xcd` in 11.8/8.8
fixed-point units. The captured final registers encode H/V scroll
`0x1f65/0x0bc6`. The older snapshot-only Firestaff replay matches 70,454 of
71,680 upper-left screenshot sample pixels; applying the live row state yields
286,720 of 286,720 exact matches across all four samples in each 2×2 screenshot
block. The test pins the VDP1 FBS, VDP2 VRAM, CRAM, register
snapshot, baseline replay, and screenshot by SHA-256.

This compositor admits the observed profile only: `BGON=0x0003`,
`CHCTLA=0x0013`, `SPCTL=0x1536`, `PRINA=0x0404`, `CCCTL=0x0043`, and
`CCRNA=0x1010`, with the captured zero bitmap/CRAM offsets. NBG1 is enabled in
this profile; the real-data harness verifies its register profile, all 4,096
map entries, and the selected transparent character before passing the
attestation to the compositor. Other enabled backgrounds or register profiles
are rejected instead of being silently omitted.

Run the authentic regression:

```sh
capture=/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_nbg0_cram_capture_20261029
baseline=/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_nbg0_ymir_replay_20261029/firestaff-replay.rgb
row_trace=/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_nbg0_live_row_state_20261029/gdb-row-state.log
FIRESTAFF_NEXUS_NBG0_LIVE_CAPTURE_DIR="$capture" \
FIRESTAFF_NEXUS_NBG0_LIVE_BASELINE_REPLAY="$baseline" \
FIRESTAFF_NEXUS_NBG0_LIVE_ROW_TRACE="$row_trace" \
  ctest --test-dir /path/to/firestaff-build --output-on-failure \
  -R '^nexus_v1_vdp2_nbg0_live_rows_real$'
```

## Limits

The regression harness authenticates the retained row trace before passing
row-state values to the C API; the C API itself does not authenticate that
trace. This capture's SH-2 read trace
overflowed and has no frame IDs, and its VDP2 write trace was not enabled.
Therefore source-to-VRAM transfer timing and complete asset ownership remain
unproven. This scanline compositor is capture-only and does not grant
production renderer permission, prove Saturn hardware parity, or cover other
register profiles.
