# Nexus VDP1 row 80 SCU DMA source join

This verifier joins three authentic sources: a cold-boot Ymir SCU-DMA trace,
the same run's final VDP1 VRAM snapshot, and the Japanese retail `MENU.BPK`
archive. It also checks the frame-300 VDP1 command and VRAM capture used by the
gameplay composition audit.

The DMA descriptor is transfer 3303: SCU channel 0 copies 1,312 bytes from
SH-2 address `0x060c5e08` to VDP1 address `0x05c643c0`, reading four bytes and
writing two bytes per step. The bounded payload trace contains all 656
16-bit writes, with no dropped rows. The payload equals the bytes at that
destination in the same run's final VDP1 VRAM snapshot. Its first 656 bytes
exactly equal the decoded 16-by-41 indexed surface `MENU.BPK[144]`; the second
656 bytes are retained as a separate part of the transfer and are not
attributed to that archive entry.

In the separate authentic gameplay capture, frame 300 command row 80 is a
mode-4 normal sprite with PMOD `0x00a0`, CMDSRCA `0xc878`, and CMDSIZE
`0x0229`. Its encoded source resolves to byte address `0x643c0`, and the first
656 bytes in that run's VDP1 VRAM snapshot also match `MENU.BPK[144]` exactly.
The same-frame palette join and visible 431-pixel ownership are verified by
the existing frame-300 composition regression.

These are cross-run joins by exact bytes and VDP1 address. The cold-boot DMA
capture contains no VDP1 command reads from CMDSRCA `0xc878`; it therefore
does not establish same-run DMA timing or command consumption. The archive
entry's UI meaning, broader HUD ownership, live champion-state binding, and
production-renderer admission remain unresolved.

## Reproduce

Keep the emulator evidence and authentic game media outside the repository.
Set the evidence root to the cold-boot package (containing `dumps/`), the
frame-300 package root (containing `capture-301/dumps/`), and the authentic
Japanese Nexus data directory containing `MENU.BPK`:

```sh
export FIRESTAFF_NEXUS_ROW80_DMA_EVIDENCE_DIR=/path/to/nexus-vdp1-row80-dma
export FIRESTAFF_NEXUS_FRAME300_EVIDENCE_DIR=/path/to/nexus-frame300
export FIRESTAFF_NEXUS_DATA_DIR=/path/to/authentic/Japanese/Nexus/data
ctest --test-dir build -R '^nexus_v1_vdp1_row80_scu_dma_bpk_join_real$' --output-on-failure
```

The analyzer pins SHA-256 hashes for the trace bundles, both captures, and the
retail archive and decoded surface. Missing external files skip the test;
changed evidence fails it. A skip is not a parity result.
