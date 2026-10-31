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
this profile, so the caller must attest its transparent visible plane; the
retained capture resolves 1,120 visible map cells to zero pixels. The real
data regression also verifies that the compositor rejects the input when
this NBG1 transparency attestation is missing. Other enabled backgrounds or
register profiles are rejected instead of being silently omitted.

Run the authentic regression:

```sh
capture=/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_nbg0_cram_capture_20261029
FIRESTAFF_NEXUS_NBG0_LIVE_CAPTURE_DIR="$capture" \
  ctest --test-dir /path/to/firestaff-build --output-on-failure \
  -R '^nexus_v1_vdp2_nbg0_live_rows_real$'
```

## Limits

The row-state values are caller-verified against a retained emulator trace;
the C API does not authenticate that trace. This capture's SH-2 read trace
overflowed and has no frame IDs, and its VDP2 write trace was not enabled.
Therefore source-to-VRAM transfer timing and complete asset ownership remain
unproven. This scanline compositor is capture-only and does not grant
production renderer permission, prove Saturn hardware parity, or cover other
register profiles.
